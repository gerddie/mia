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

#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <mia/core.hh>

#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/ground_truth_evaluator.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/similarity_profile.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
       {pdi_group, "Registration of series of 2D images"},
       {pdi_short, "Registration of a series of 2D images"},
       {
              pdi_description, "This program implements the non-linear registration based on Pseudo "
              "Ground Thruth for motion compensation of series of myocardial perfusion images as "
              "described in [Chao Li and Ying Sun, 'Nonrigid Registration of Myocardial Perfusion "
              "MRI Using Pseudo Ground Truth' , In Proc. Medical Image Computing and Computer-Assisted "
              "Intervention MICCAI 2009, 165-172, 2009.](https://doi.org/10.1007/978-3-642-04268-3_21) "
              "Note that for this nonlinear motion correction "
              "a preceding linear registration step is usually required."
       },
       {
              pdi_example_descr, "Register the perfusion series given by images imageXXXX.exr by using "
              "Pseudo Ground Truth estimation. Skip two images at the beginning and otherwiese use the "
              "default parameters. Store the result images to  'regXXXX.exr'."
       },
       {pdi_example_code, "-i imageXXXX.exr -o regXXXX.exr -k 2"}
};


struct SPGTParams {
       double alpha;
       double beta;
       double rho_thresh;

       SPGTParams();
};

struct SRegistrationParams {
       PMinimizer minimizer;
       size_t c_rate;
       double divcurlweight;
       double imageweight;
       size_t mg_levels;
       SRegistrationParams();
};

SPGTParams::SPGTParams():
       alpha(1.0),
       beta(1.0),
       rho_thresh(0.9)
{
}

SRegistrationParams::SRegistrationParams():
       c_rate(32),
       divcurlweight(20.0),
       imageweight(1.0),
       mg_levels(3)
{
}

C2DFullCostList create_costs(double imageweight)
{
       C2DFullCostList result;
       stringstream image_descr;
       image_descr << "image:cost=ssd,weight=" << imageweight;
       result.push(C2DFullCostPluginHandler::instance().produce(image_descr.str()));
       return result;
}

P2DTransformationFactory create_transform_creator(size_t c_rate, double divcurlweight)
{
       stringstream transf;
       transf << "spline:rate=" << c_rate << ",penalty=[divcurl:weight=" <<  divcurlweight << "]";
       return C2DTransformCreatorHandler::instance().produce(transf.str());
}


vector<P2DTransformation>
run_registration_pass(const CSegSetWithImages&  input_set, C2DImageSeries& series,
                      const C2DImageSeries& references,
                      int skip_images, const SRegistrationParams& params)
{
       vector<P2DTransformation> result;
       C2DImageSeries input_images = input_set.get_images();
       auto costs  = create_costs(params.imageweight);
       auto transform_creator = create_transform_creator(params.c_rate, params.divcurlweight);
       C2DNonrigidRegister nrr(costs, params.minimizer,  transform_creator, params.mg_levels);

       // this loop could be parallized
       for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
              cvmsg() << "Register frame " << i << "\n";
              P2DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
              series[i] = (*transform)(*input_images[i + skip_images]);
              result.push_back(transform);
       }

       return result;
}


int do_main( int argc, char *argv[] )
{
       SPGTParams pgt_params;
       SRegistrationParams reg_params;
       // this parameter is currently not exported - reading the image data is
       // therefore done from the path given in the segmentation set
       bool override_src_imagepath = true;
       string in_filename;
       string out_filename;
       string registered_filebase("reg");
       size_t skip = 2;
       size_t passes = 4;
       double c_rate_divider = 4;
       double divcurlweight_divider = 4.0;
       PSplineKernel interpolator_kernel;
       CCmdOptionList options(g_description);
       options.set_group("\nFile-IO");
       options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOptionFlags::required_input));
       options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOptionFlags::required_output));
       options.add(make_opt( registered_filebase, "registered", 'r', "file name base for registered files"));
       options.set_group("\nPreconditions");
       options.add(make_opt( skip, "skip", 's', "skip images at beginning of series"));
       options.add(make_opt( passes, "passes", 'P', "number of registration passes"));
       options.set_group("\nPseudo-Ground-Thruth");
       options.add(make_opt( pgt_params.alpha, "alpha", 'A', "spacial neighborhood penalty weight",
                             CCmdOptionFlags::required));
       options.add(make_opt( pgt_params.beta, "beta", 'B', "temporal second derivative penalty weight",
                             CCmdOptionFlags::required));
       options.add(make_opt( pgt_params.rho_thresh, "rho_thresh", 'R',
                             "correlation threshold for neighborhood analysis", CCmdOptionFlags::required));
       options.set_group("\nRegistration");
       options.add(make_opt( reg_params.minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
       options.add(make_opt( interpolator_kernel, "bspline:d=3", "interpolator", 'p', "image interpolator kernel"));
       options.add(make_opt( reg_params.mg_levels, "mr-levels", 'l', "multi-resolution levels"));
       options.add(make_opt( reg_params.divcurlweight, "divcurl", 'd',
                             "divcurl regularization weight"));
       options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
                             "divcurl weight scaling with each new pass"));
       options.add(make_opt( reg_params.c_rate, "start-c-rate", 'a',
                             "start coefficinet rate in spines, gets divided "
                             "by --c-rate-divider with every pass"));
       options.add(make_opt( c_rate_divider, "c-rate-divider", 0,
                             "cofficient rate divider for each pass"));
       options.add(make_opt( reg_params.imageweight, "imageweight", 'w',
                             "image cost weight"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       // load input data set
       CSegSetWithImages  input_set(in_filename, override_src_imagepath);

       if (skip >= input_set.get_frames().size()) {
              throw create_exception<invalid_argument>("Try to skip ", skip,
                            " images, but input set has only ", input_set.get_frames().size(), " images.");
       }

       // create ground thruth evaluator
       C2DGroundTruthEvaluator gte(pgt_params.alpha, pgt_params.beta, pgt_params.rho_thresh);
       vector<P2DImage> pgt;
       vector<P2DTransformation> transforms;
       C2DImageSeries images = input_set.get_images();
       C2DImageSeries series(images.begin() + skip, images.end());
       // main registration loop
       size_t pass = 0;

       while (pass < passes) {
              cvmsg() << "Registration pass " << pass << "\n";
              // skip images at start
              // create pseudeo ground thruth
              gte(series, pgt);
              // run registration pass
              // we always register from the original image to not make it too smooth
              // due to interpolation
              transforms = run_registration_pass(input_set, series, pgt, skip, reg_params);
              // prepare next pass
              reg_params.c_rate /= c_rate_divider;
              reg_params.divcurlweight /= divcurlweight_divider;
              ++pass;
       }

       copy(series.begin(), series.end(), images.begin() + skip);
       CSegSet::Frames& frames = input_set.get_frames();
       auto t = transforms.begin();

       for (auto f = frames.begin() + skip; f != frames.end(); ++t, ++f)
              f->inv_transform(**t);

       // copy back registered images
       input_set.rename_base(registered_filebase);
       input_set.save_images(out_filename);
       ofstream outfile(out_filename.c_str(), ios_base::out );

       if (outfile.good())
              outfile << input_set.write().write_to_string();

       return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);
