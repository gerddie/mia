/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <fstream>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/ica.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/transformfactory.hh>

using namespace std;
using namespace mia;

namespace bfs = boost::filesystem;

const SProgramDescription g_description = {
       {pdi_group, "Registration of series of 2D images"},
       {pdi_short, "Run a registration of a series of 2D images."},
       {
              pdi_description, "This program runs the non-rigid registration of an perfusion image series."
              "In each pass, first an ICA analysis is run to estimate and eliminate "
              "the periodic movement and create reference images with intensities similar "
              "to the corresponding original image. Then non-rigid registration is run "
              "using the an \"ssd + divcurl\" cost model. The B-spline c-rate and the "
              "divcurl cost weight are changed in each pass according to given parameters."
              "In the first pass a bounding box around the LV myocardium may be extracted"
              "to speed up computation\n"
              "Special note to this implemnentation: the registration is always run from the "
              "original images to avoid the accumulation of interpolation errors."
       },
       {
              pdi_example_descr, "Register the perfusion series given in 'segment.set' by "
              "using automatic ICA estimation. Skip two images at the beginning and otherwiese "
              "use the default parameters. Store the result in 'registered.set'."
       },
       {pdi_example_code, "  -i segment.set -o registered.set -k 2"}
};

C2DFullCostList create_costs(double imageweight)
{
       C2DFullCostList result;
       stringstream image_descr;
       image_descr << "image:weight=" << imageweight;
       result.push(C2DFullCostPluginHandler::instance().produce(image_descr.str()));
       return result;
}

P2DTransformationFactory create_transform_creator(size_t c_rate, double divcurlweight)
{
       stringstream transf;
       transf << "spline:rate=" << c_rate << ",penalty=[divcurl:weight=" << divcurlweight << "]";
       return C2DTransformCreatorHandler::instance().produce(transf.str());
}


void segment_and_crop_input(CSegSetWithImages&  input_set,
                            const C2DPerfusionAnalysis& ica,
                            float box_scale,
                            C2DPerfusionAnalysis::EBoxSegmentation segmethod,
                            C2DImageSeries& references,
                            const string& save_crop_feature)
{
       C2DBounds crop_start;
       auto cropper = ica.get_crop_filter(box_scale, crop_start,
                                          segmethod, save_crop_feature);

       if (!cropper)
              throw create_exception<runtime_error>( "Cropping was requested, but segmentation failed");

       C2DImageSeries input_images = input_set.get_images();

       for (auto i = input_images.begin(); i != input_images.end(); ++i)
              *i = cropper->filter(**i);

       for (auto i = references.begin(); i != references.end(); ++i)
              *i = cropper->filter(**i);

       auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
       P2DTransformation shift = tr_creator->create(C2DBounds(1, 1));
       auto p = shift->get_parameters();
       p[0] = crop_start.x;
       p[1] = crop_start.y;
       shift->set_parameters(p);
       input_set.transform(*shift);
       input_set.set_images(input_images);
}

vector<P2DTransformation>
run_registration_pass(CSegSetWithImages&  input_set,
                      C2DImageSeries& registered,
                      const C2DImageSeries& references,
                      int skip_images, PMinimizer minimizer, size_t mg_levels,
                      double c_rate, double divcurlweight, double imageweight)
{
       vector<P2DTransformation> result;
       C2DImageSeries input_images = input_set.get_images();
       registered.resize(input_images.size());
       auto costs  = create_costs(imageweight);
       auto transform_creator = create_transform_creator(c_rate, divcurlweight);
       C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);

       // this loop could be parallized
       for (size_t i = skip_images; i < input_images.size(); ++i) {
              cvmsg() << "Register frame " << i << "\n";
              P2DTransformation transform = nrr.run(input_images[i],
                                                    references[i - skip_images]);
              registered[i] = (*transform)(*input_images[i]);
              result.push_back(transform);
       }

       return result;
}

int do_main( int argc, char *argv[] )
{
       // IO parameters
       string in_filename;
       string out_filename;
       string registered_filebase("reg");
       // debug options: save some intermediate steps
       string cropped_filename;
       string save_crop_feature;
       // this parameter is currently not exported - reading the image data is
       // therefore done from the path given in the segmentation set
       bool override_src_imagepath = true;
       // registration parameters
       PMinimizer minimizer;
       double c_rate = 32;
       double c_rate_divider = 4;
       double divcurlweight = 20.0;
       double divcurlweight_divider = 4.0;
       double imageweight = 1.0;
       PSplineKernel interpolator_kernel;
       size_t mg_levels = 3;
       // ICA parameters
       size_t components = 0;
       bool normalize = false;
       bool no_meanstrip = false;
       float box_scale = 0.0;
       size_t skip_images = 0;
       size_t max_ica_iterations = 400;
       C2DPerfusionAnalysis::EBoxSegmentation
       segmethod = C2DPerfusionAnalysis::bs_features;
       PIndepCompAnalysisFactory icatool;
       size_t current_pass = 0;
       size_t pass = 3;
       CCmdOptionList options(g_description);
       options.set_group("\nFile-IO");
       options.add(make_opt( in_filename, "in-file", 'i',
                             "input perfusion data set", CCmdOptionFlags::required_input));
       options.add(make_opt( out_filename, "out-file", 'o',
                             "output perfusion data set", CCmdOptionFlags::required_output));
       options.add(make_opt( registered_filebase, "registered", 'r',
                             "file name base for registered fiels"));
       options.add(make_opt( cropped_filename, "save-cropped", 0,
                             "save cropped set to this file"));
       options.add(make_opt( save_crop_feature, "save-feature", 0,
                             "save segmentation feature images"
                             " and initial ICA mixing matrix"));
       options.set_group("\nRegistration");
       options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
       options.add(make_opt( c_rate, "start-c-rate", 'a',
                             "start coefficinet rate in spines,"
                             " gets divided by --c-rate-divider with every pass"));
       options.add(make_opt( c_rate_divider, "c-rate-divider", 0,
                             "cofficient rate divider for each pass"));
       options.add(make_opt( divcurlweight, "start-divcurl", 'd',
                             "start divcurl weight, gets divided by"
                             " --divcurl-divider with every pass"));
       options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
                             "divcurl weight scaling with each new pass"));
       options.add(make_opt( imageweight, "imageweight", 'w',
                             "image cost weight"));
       options.add(make_opt( interpolator_kernel, "bspline:d=3", "interpolator", 'p', "image interpolator kernel"));
       options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
       options.add(make_opt( pass, "passes", 'P', "registration passes"));
       options.set_group("\nICA");
       options.add(make_opt( icatool, "internal", "fastica", 0, "FastICA implementationto be used"));
       options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
       options.add(make_opt( normalize, "normalize", 0, "don't normalized ICs"));
       options.add(make_opt( no_meanstrip, "no-meanstrip", 0,
                             "don't strip the mean from the mixing curves"));
       options.add(make_opt( box_scale, "segscale", 's',
                             "segment and scale the crop box around the LV (0=no segmentation)"));
       options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
                             "e.g. because as they are of other modalities"));
       options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA"));
       options.add(make_opt(segmethod, C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E',
                            "Segmentation method"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       // this cost will always be used
       // load input data set
       CSegSetWithImages  input_set(in_filename, override_src_imagepath);
       C2DImageSeries input_images = input_set.get_images();
       cvmsg() << "skipping " << skip_images << " images\n";
       vector<C2DFImage> series(input_images.size() - skip_images);
       transform(input_images.begin() + skip_images, input_images.end(),
                 series.begin(), FCopy2DImageToFloatRepn());
       // run ICA
       unique_ptr<C2DPerfusionAnalysis> ica(new C2DPerfusionAnalysis(components, normalize, !no_meanstrip));

       if (max_ica_iterations)
              ica->set_max_ica_iterations(max_ica_iterations);

       ica->set_approach(CIndepCompAnalysis::appr_defl);

       if (!ica->run(series, *icatool)) {
              ica.reset(new C2DPerfusionAnalysis(components, normalize, !no_meanstrip));
              ica->set_approach(CIndepCompAnalysis::appr_symm);

              if (!ica->run(series, *icatool))
                     box_scale = false;
       }

       vector<C2DFImage> references_float = ica->get_references();
       C2DImageSeries references(references_float.size());
       transform(references_float.begin(), references_float.end(), references.begin(),
                 FWrapStaticDataInSharedPointer<C2DImage>());

       // crop if requested
       if (box_scale) {
              segment_and_crop_input(input_set, *ica, box_scale, segmethod, references, save_crop_feature);
              input_images = input_set.get_images();
       }

       // save cropped images if requested
       if (!cropped_filename.empty()) {
              bfs::path cf(cropped_filename);
              cf.replace_extension();
              input_set.rename_base(cf.filename().string());
              input_set.save_images(cropped_filename);
              ofstream outfile(cropped_filename, ios_base::out );

              if (outfile.good())
                     outfile << input_set.write().write_to_string();
              else
                     throw create_exception<runtime_error>( "unable to save to '", cropped_filename, "'");
       }

       vector<P2DTransformation> transformations;
       C2DImageSeries registered;
       bool do_continue = true;
       bool lastpass = false;

       while (do_continue || lastpass) {
              ++current_pass;
              cvmsg() << "Registration pass " << current_pass << "\n";
              transformations =
                     run_registration_pass(input_set, registered, references,  skip_images,  minimizer,
                                           mg_levels, c_rate, divcurlweight, imageweight);

              if (lastpass)
                     break;

              C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip);

              if (max_ica_iterations)
                     ica2.set_max_ica_iterations(max_ica_iterations);

              transform(registered.begin() + skip_images,
                        registered.end(), series.begin(), FCopy2DImageToFloatRepn());

              if (!ica2.run(series, *icatool))
                     ica2.set_approach(CIndepCompAnalysis::appr_symm);

              ica2.run(series, *icatool);
              divcurlweight /= divcurlweight_divider;

              if (c_rate > 1)
                     c_rate /= c_rate_divider;

              references_float = ica2.get_references();
              transform(references_float.begin(), references_float.end(),
                        references.begin(), FWrapStaticDataInSharedPointer<C2DImage>());
              const bool can_one_more_pass = !pass || current_pass < pass;
              do_continue = can_one_more_pass && ica2.has_movement();

              if (!do_continue && !save_crop_feature.empty()) {
                     stringstream cfile;
                     cfile << save_crop_feature << "-final.txt";
                     ica2.save_coefs(cfile.str());
                     stringstream new_base;
                     new_base << save_crop_feature << "-p" << pass << "-final";
                     ica2.save_feature_images(new_base.str());
              }

              // run one more pass if the limit is not reached and no movement identified
              lastpass = (!do_continue && can_one_more_pass);
       }

       CSegSetWithImages::Frames& frames = input_set.get_frames();

       for (size_t i = skip_images; i < input_images.size(); ++i)
              frames[i].inv_transform(*transformations[i - skip_images]);

       C2DImageSeries iimages = input_set.get_images();
       copy(iimages.begin(), iimages.begin() + skip_images, registered.begin());
       input_set.set_images(registered);
       input_set.rename_base(registered_filebase);
       input_set.save_images(out_filename);
       ofstream outfile(out_filename.c_str(), ios_base::out );

       if (outfile.good())
              outfile << input_set.write().write_to_string();

       return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main);

