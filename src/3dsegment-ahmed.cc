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

/*
   This program implements and enhances the paper:
   Mohamed N. Ahmed et. al, "A Modified Fuzzy C-Means Algorithm for Bias Field
   estimation and Segmentation of MRI Data", IEEE Trans. on Medical Imaging,
   Vol. 21, No. 3, March 2002

   changes are:
   - p = 2
   - exp
   -
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std;

typedef vector<C3DFImage> C3DFImageVec;

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"},
       {pdi_short, "Run a fuzzy c-means segmentation of a 3D image."},
       {
              pdi_description, "This program implements a variation of the paper:"
              "[Mohamed N. Ahmed et. al, \"A Modified Fuzzy C-Means Algorithm for Bias Field "
              "estimation and Segmentation of MRI Data\", IEEE Trans. on Medical Imaging, "
              "Vol. 21, No. 3, March 2002,](https://doi.org/10.1109/42.996338) changes are: p=2, and exp"
       },
       {
              pdi_example_descr, "Run a 5-class segmentation over input image input.v and store the class "
              "probability images in cls.v."
       },
       {pdi_example_code, "-i input.v -a 5 -o cls.v"}
};


class CSegment3d
{
public:
       typedef pair<P3DImage, C3DImageVector> result_type;

       CSegment3d(bool bg_correct, int ncc,
                  const vector<float>& icc, float k);

       template <class T>
       CSegment3d::result_type operator()(const T3DImage<T>& image);

private:
       C3DFImage process(const C3DFImage& image,
                         vector<float>& class_centers,
                         C3DFImageVec& prob)const;

       float update_class_centers(C3DFImage& image, C3DFImageVec& prob, vector<float>& class_centers)const;

       void evaluate_probabilities(C3DFImageVec& prob) const;

       unsigned int _M_nClasses;
       bool         _M_bg_correct;
       vector<float> _M_class_centers;
       const float _M_k;
       mutable float _M_minh;
       mutable float _M_maxh;


};



CSegment3d::CSegment3d(bool bg_correct, int ncc,
                       const vector<float>& icc, float k):
       _M_nClasses(ncc),
       _M_bg_correct(bg_correct),
       _M_class_centers(icc),
       _M_k(k),
       _M_minh(0),
       _M_maxh(0)
{
}

struct CLogTransform {
       CLogTransform(float minh, float maxh):
              _M_minh(minh),
              _M_div(maxh - minh + 1.0)
       {
       }

       float operator() (float x) const
       {
              return (x - _M_minh) / _M_div;
       }
private:
       float _M_minh;
       float _M_div;

};

struct CExpTransform {
       CExpTransform(float minh, float maxh):
              _M_minh(minh),
              _M_div(maxh - minh + 1.0)
       {
       }
       float operator() (float x) const
       {
              return x *  _M_div + _M_minh;
       }
private:
       float _M_minh;
       float _M_div;

};

template <class T>
struct CThresholder {
       CThresholder(T thresh): _M_thresh(thresh)
       {
       }

       T operator ()(T x)
       {
              return x > _M_thresh ? x : 0;
       }
private:
       T _M_thresh;
};

template <typename CI>
void get_min_max(CI begin, CI end, typename CI::value_type& minh, typename CI::value_type& maxh)
{
       auto minmax = minmax_element(begin, end);
       minh = *minmax.first;
       maxh = *minmax.second;
}


template <class T>
CSegment3d::result_type CSegment3d::operator()(const T3DImage<T>& image)
{
       // first evaluate the histogram borders and get the initial class centers
       typename T3DImage<T>::value_type minh = 0;
       typename T3DImage<T>::value_type maxh = numeric_limits<T>::max();
       get_min_max(image.begin(), image.end(), minh, maxh);
       _M_minh = minh;
       _M_maxh = maxh;
       cvdebug() << "Entering CSegment3d::operator()\n";

       if (_M_class_centers.empty()) {
              cvdebug() << "Estimate initial class centers from histogram\n";
              // estimate some classes
              _M_class_centers.resize(_M_nClasses);
              _M_class_centers[0] = _M_minh;
              float step = (_M_maxh - _M_minh) / 256.0;
              THistogram<THistogramFeeder<T>> histogram(THistogramFeeder<T>(_M_minh, _M_maxh, 256));
              histogram.push_range(image.begin(), image.end());
              float thresh = static_cast<float>(image.size() - histogram[0]) / _M_nClasses;
              float hit = 0.0;
              size_t i = 1;
              float val = _M_minh + step;
              auto hi = histogram.begin();
              auto he = histogram.end();
              ++hi;

              while (i < _M_nClasses && hi != he) {
                     hit += *hi;

                     if (hit > thresh) {
                            _M_class_centers[i++] = val;
                            hit -= thresh;
                     }

                     ++hi;
                     val += step;
              }

              // print out the new class centers
       }

       cvmsg() << " initial classes [" << _M_class_centers << "\n";
       C3DFImage log_image(image.get_size()); 	                                 // 4 Bpp
       vector<float> log_class_centers(_M_nClasses);
       CLogTransform logt(_M_minh, _M_maxh);
       // fill the logarithmic image with values
       transform(image.begin(), image.end(), log_image.begin(), logt);
       transform(_M_class_centers.begin(), _M_class_centers.end(), log_class_centers.begin(), logt);
       // initialise teporaries
       C3DFImageVec prob;

       for (size_t i = 0; i < _M_nClasses; ++i) {
              prob.push_back(C3DFImage(image.get_size()));                    // 4 * _M_nClasses Bpp
       }

       auto b0_image = process(log_image, log_class_centers, prob);
       CExpTransform exptrans(minh, maxh);
       transform(log_class_centers.begin(), log_class_centers.end(),
                 _M_class_centers.begin(), exptrans);
       P3DImage pb0_corrected;

       if ( _M_bg_correct ) {
              T3DImage<T> *b0_corrected = new T3DImage<T>(image);
              transform(b0_image.begin(), b0_image.end(), b0_corrected->begin(),
              [&exptrans](float x) {
                     return mia_round_clamped<T>(exptrans(x));
              });
              pb0_corrected.reset(b0_corrected);
       }

       transform(log_class_centers.begin(), log_class_centers.end(),
                 _M_class_centers.begin(), exptrans);
       C3DImageVector class_images;

       for (size_t j = 0; j < _M_nClasses; ++j) {
              C3DFImage *r = new C3DFImage(image.get_size(), image);
              transform(prob[j].begin(), prob[j].end(), r->begin(),
              [](float x) {
                     return sqrt(x);
              });
              class_images.push_back(P3DImage(r));
       }

       return make_pair(pb0_corrected, class_images);
}


void CSegment3d::evaluate_probabilities(C3DFImageVec& prob)const
{
       vector<C3DFImage::iterator> pi(_M_nClasses);
       C3DFImage::iterator stopper =  prob[0].end();

       for (size_t i = 0; i < _M_nClasses; ++i)
              pi[i] = prob[i].begin();

       while (pi[0] != stopper) {
              float sum = 0.0;

              for (size_t i = 0; i < _M_nClasses; ++i)
                     sum += *pi[i];

              if (sum != 0.0)
                     for (size_t i = 0; i < _M_nClasses; ++i) {
                            *pi[i] /= sum;
                            *pi[i] *= *pi[i];
                            ++pi[i];
                     } else { // must be a bright value, otherwise, class[0] would catch it
                     for (size_t i = 0; i < _M_nClasses - 1; ++i)
                            *pi[i]++ = 0.0;

                     *pi[_M_nClasses - 1]++ = 1.0;
              }
       }
}

float CSegment3d::update_class_centers(C3DFImage& image, C3DFImageVec& prob, vector<float>& class_centers) const
{
       float residuum = 0.0;

       for (size_t i = 0; i < _M_nClasses; ++i) {
              float cc = class_centers[i];
              float loc_res = 100;

              // try a fix-point iteration for the new class center

              while (loc_res > 0.0001) {
                     float cc_old = cc;
                     C3DFImage::const_iterator ihelp = image.begin();
                     C3DFImage::const_iterator iprob = prob[i].begin();
                     C3DFImage::const_iterator eprob = prob[i].end();
                     double sum_prob = 0.0;
                     double sum_weight = 0.0;

                     while (iprob != eprob) {
                            if (*iprob > 0.0f) {
                                   const double val = *ihelp - cc;
                                   const double val2 = val * val;
                                   const double val2mexpval2 = val2 * exp(val2)  * *iprob;
                                   sum_prob += val2mexpval2;
                                   sum_weight += val2mexpval2 * *ihelp;
                            }

                            ++ihelp;
                            ++iprob;
                     }

                     // how do i deal with the (very unprobable case sum_prob==0.0?)
                     if (sum_prob  != 0.0) {
                            cc += 0.1 * (sum_weight / sum_prob - cc);
                     } else {
                            cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << sum_prob << ":" << sum_weight << endl;
                     }

                     loc_res = cc > cc_old ? cc - cc_old :  cc_old - cc;
                     cvdebug() << "res = " << loc_res << " : cc = " << cc << "\n";
              }// end while (loc_res > 0.01)

              cverb << "\n";
              float delta = cc - class_centers[i];
              residuum += delta * delta;
              class_centers[i] =  cc;
       }// end update class centers

       return residuum;
}



C3DFImage CSegment3d::process(const C3DFImage& image,
                              vector<float>& class_centers,
                              C3DFImageVec& prob)const
{
       int n_iterations = 0;
       CExpTransform exptrans(_M_minh, _M_maxh);
       C3DFImage tmp(image.get_size());                                        // 4Bpp
       unique_ptr<C3DFImage> bg_image;

       if (_M_bg_correct) {
              bg_image.reset(new C3DFImage(image.get_size()));
              fill(bg_image->begin(), bg_image->end(), 1.0);
       }

       float residuum = 100;

       // these conditions have to be exported
       while (residuum > 0.000001 && n_iterations < 100) {
              ++n_iterations;
              // distance evaluation
              vector<float>::const_iterator icls = class_centers.begin();

              for (size_t i = 0; i < _M_nClasses; ++i, ++icls) {
                     // evaluate D for class i
                     C3DFImage::iterator di = prob[i].begin();
                     C3DFImage::iterator de = prob[i].end();
                     C3DFImage::const_iterator ii = image.begin();

                     if (_M_bg_correct) {
                            C3DFImage::const_iterator ib = bg_image->begin();

                            while (di != de) {
                                   float val = *ii++ * *ib++ - *icls;
                                   *di++ = expf(-val * val * _M_k);
                            }
                     } else
                            while (di != de) {
                                   float val = *ii++ - *icls;
                                   *di++ = expf(-val * val * _M_k);
                            }
              }// end D evaluation

              // evaluate probabilities
              evaluate_probabilities(prob);

              if (_M_bg_correct)
                     transform(image.begin(), image.end(), bg_image->begin(), tmp.begin(),
                     [](float x, float y) {
                     return x * y;
              });
              else
                     copy(image.begin(), image.end(), tmp.begin());

              residuum = update_class_centers(tmp, prob, class_centers);
              // print out the new class centers
              cvmsg() << n_iterations << ": classes [";

              for (size_t j = 0; j < _M_nClasses; ++j)
                     cverb << exptrans(class_centers[j]) << " ";

              cverb << "]" << endl;

              // update field correction
              // this certainly needs a methodological review
              if (_M_bg_correct) {
                     C3DFImage::const_iterator ii = image.begin();
                     C3DFImage::iterator ib = bg_image->begin();
                     C3DFImage::iterator eb = bg_image->begin();
                     vector<C3DFImage::const_iterator> pi(_M_nClasses);

                     for (size_t j = 0; j < _M_nClasses; ++j)
                            pi[j] = prob[j].begin();

                     while (ib != eb) {
                            float sum_prob = 0.0;
                            float sum_weight = 0.0;

                            for (size_t j = 0; j < _M_nClasses; ++j) {
                                   sum_weight += *pi[j] * class_centers[j];
                                   sum_prob   += *pi[j];
                                   ++pi[j];
                            }

                            assert(sum_prob != 0.0);
                            *ib = *ii * (sum_prob / sum_weight);
                            ++ib;
                            ++ii;
                     }
              }// end update bg field
       } // end main while loop

       return tmp;
}

int do_main(int argc, char *argv[])
{
       string in_filename;
       string cls_filename;
       string bias_filename;
       int n_classes = 3;
       bool bg_correct = false;
       string neighborhood_filter;
       vector<float> initial_class_centres;
       float spread = 64;
       const auto& image3dio = C3DImageIOPluginHandler::instance();
       CCmdOptionList opts(g_description);
       opts.set_group("File-IO");
       opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input, &image3dio ));
       opts.add(make_opt( cls_filename, "class-file", 'c', "class probability images, the image type "
                          "must support multiple images and floating point values",
                          CCmdOptionFlags::required_output, &image3dio ));
       opts.add(make_opt( bias_filename, "out-file", 'b', "Bias corrected image will be of the same type like "
                          "the input image. If this parameter is not given, then the bias correction will not be applied.",
                          CCmdOptionFlags::output, &image3dio ));
       opts.set_group("Parameters");
       opts.add(make_opt( n_classes, "no-of-classes", 'n', "number of classes"));
       opts.add(make_opt( initial_class_centres, "class-centres", 'C', "initial class centers, this parameter overrides "
                          "'no-of-clases'."));
       opts.add(make_opt( spread, "spread", 's', "spread parameter describing the strength of mattar distinction"));

       if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       if (initial_class_centres.empty()) {
              cvdebug() << "no class centers given at command line\n";

              if (n_classes < 2)
                     throw invalid_argument("at least two classes need to be requested");
       } else {
              cvdebug() << "Initially classes: " << initial_class_centres.size() << "\n";
              n_classes = initial_class_centres.size();
       }

       bg_correct = !bias_filename.empty();
       // initialize the functor
       CSegment3d Segment(bg_correct, n_classes,
                          initial_class_centres, spread);
       auto in_image = load_image3d(in_filename);
       auto result = mia::accumulate (Segment, *in_image);
//	C3DImageIOPluginHandler::Instance::Data out_image_list(result.second);

       //CHistory::instance().append(argv[0], revision, opts);

       if ( !image3dio.save(cls_filename, result.second) ) {
              throw create_exception<runtime_error>("unable to save class images to '", cls_filename, "'");
       }

       if (!bias_filename.empty()) {
              if (!save_image(bias_filename, result.first))
                     throw create_exception<runtime_error>("unable to save bias-corrected image '", bias_filename, "'");
       }

       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);


