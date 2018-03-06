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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <cmath>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/histogram.hh>
#include <mia/core/kmeans.hh>
#include <mia/core/cmeans.hh>
#include <mia/2d/fuzzyclustersolver_cg.hh>
#include <mia/2d/fuzzyclustersolver_sor.hh>
#include <mia/2d/fuzzyseg.hh>

NS_MIA_BEGIN
using namespace std;


/* maximum number of iterations for EM-Algorithm */
#define _MAXIT 400


typedef vector<C2DFImage> C2DFImageVec;


class CSegment2d : public TFilter<C2DFImageVec>
{
public:
       CSegment2d(const unsigned int& nClasses, const SFuzzySegParams& params);
       ~CSegment2d ();
       P2DImage get_out_image () const;
       P2DImage get_gain_image () const;

       template <typename T>
       CSegment2d::result_type operator () (const T2DImage<T>& data);
private:
       unsigned int m_nClasses;
       SFuzzySegParams m_params;
       P2DImage     m_out;
       P2DImage     m_gain_image;

};


/// maximum number of iterations for the solver
#define _MAX_ITER_PDE 	1000


using namespace std;


// solves the PDE  (W + lambda1 * H1 + lambda2 * H2) = f
void solvePDE (C2DFImage& weight_image, C2DFImage& force_image, C2DFImage& gain_image, const SFuzzySegParams& params)
{
       C2DFuzzyClusterSolver solver(weight_image, params.lambda1, params.lambda2, 1000);
       solver.solve (force_image, gain_image);
}

template <class Data2D>
int estimateGain (C2DFImage& gain_image, const Data2D& src_image, vector<C2DFImage>& cls_image,
                  vector<double>& clCenter, unsigned int classes, const SFuzzySegParams& params)
{
       const unsigned int nx = src_image.get_size().x;
       const unsigned int ny = src_image.get_size().y;
       int t = 0;
       // precompute force (f) and weight matrix (w)
       C2DFImage force_image (src_image.get_size());
       C2DFImage weight_image (src_image.get_size());

       for (unsigned int y = 2; y < ny - 2; y++)  {
              for (unsigned int x = 2; x < nx - 2; x++)  {
                     double forcePixel = 0;
                     double weightPixel = 0;

                     for (unsigned int k = 0; k < classes; k++)  {
                            double uk = cls_image[k](x, y);
                            double vk = clCenter[k];
                            double v = uk * uk * vk;
                            forcePixel += v;
                            weightPixel += v * vk;
                     };

                     forcePixel *= src_image(x, y);

                     force_image(x, y) = (float)(forcePixel);

                     weight_image(x, y) = (float)(weightPixel);
              };
       };

       // C.Wolters:
       // now solve system using scaled CG
       solvePDE(weight_image, force_image, gain_image, params);

       return t;
};



CSegment2d::CSegment2d(const unsigned int& nClasses, const SFuzzySegParams& params):
       m_nClasses(nClasses),
       m_params(params)
{
}

CSegment2d::~CSegment2d ()
{
}

P2DImage CSegment2d::get_out_image () const
{
       return m_out;
}

P2DImage CSegment2d::get_gain_image () const
{
       return m_gain_image;
}


/*! Functor operator
  \param data any data struture that hold an (STL- style) iterator
  \returns the corrected image, as well as the class-probability images
*/
template <typename T>
CSegment2d::result_type CSegment2d::operator () (const T2DImage<T>& data)
{
       const size_t nx = data.get_size().x;
       const size_t ny = data.get_size().y;
       const size_t noOfPixels = nx * ny;
       // number of pixels in image
       // TODO: was ist u ??
       vector<double> u(m_nClasses);
       // Field to store the border
       vector<char> border(noOfPixels);
       // get type of iterator
       typedef T itype;
       // maximum pixel value in image
       auto maximum =	max_element(data.begin(), data.end());
       assert(maximum == data.end() || *maximum >= 1);
       itype iMax = *maximum;
       cvmsg()  << " The Maximum pixel value is " << (double) iMax << endl;
       unsigned long k = 0;
       auto data_itr = data.begin();

       for ( ; data_itr < data.end(); k++, data_itr++ ) {
              // initialize border information
              if ( fborder(k, nx, ny) )
                     border[k] = 0;
              else
                     border[k] = 1;
       }

       // Create and initialize gain-field
       C2DFImage gain_image( data.get_size(), data);
       fill (gain_image.begin(), gain_image.end(), 1.0);
       // class probability image, compute initial class centers
       vector<double> clCenter(m_nClasses);
       vector<unsigned short> buffer(data.size());
       kmeans(data.begin(), data.end(), buffer.begin(), clCenter);
       cvmsg()  << "intl. class centers:" << clCenter << "\n";
       // Algorithm step 1
       // create class membership volumes
       C2DFImageVec cls_image;

       for (size_t i = 0; i < m_nClasses; ++i)  {
              cls_image.push_back(C2DFImage ( data.get_size(), data));
       }

       for (unsigned int t = 0; t < _MAXIT; t++)  {
              // Algorithm step 2:
              // estimate gain field
              estimateGain (gain_image, data, cls_image, clCenter, m_nClasses, m_params);
              // Algorithm step 3:
              // recompute class memberships
              cmeans_evaluate_probabilities(data, gain_image, clCenter, cls_image);
              // Algorithm step 4:
              // recompute class clCenters
              double residuum = cmeans_update_class_centers(data, gain_image, cls_image, clCenter);
              cvmsg() << "I[" << t << "]: cc=" << clCenter << ", r=" << residuum << "\n";

              if (residuum < 0.0001)
                     break;
       };

       // compute corrected image
       T2DImage<T> *corrected_image = new T2DImage<T> (data.get_size(), data);

       C2DFImage::iterator gain_itr = gain_image.begin();

       typename T2DImage<T>::iterator corrected_itr = corrected_image->begin();

       for ( data_itr = data.begin(); data_itr < data.end();
             gain_itr++, data_itr++, corrected_itr++) {
              double corrected_val = (double) * data_itr;

              if (*gain_itr != 0)
                     corrected_val /= *gain_itr;

              *corrected_itr = (T) corrected_val;
       }

       corrected_image->set_attribute("class_centers",
                                      PAttribute(new CVDoubleAttribute( clCenter)));
       m_out.reset(corrected_image);

       for (size_t i = 0; i < cls_image.size(); ++i) {
              cls_image[i].set_attribute("class_number",
                                         PAttribute(new CIntAttribute( i )));
              cls_image[i].set_attribute("class_centers",
                                         PAttribute(new CVDoubleAttribute( clCenter)));
       }

       m_gain_image.reset(new C2DFImage(gain_image));
       return cls_image;
}

template <>
CSegment2d::result_type CSegment2d::operator () (const C2DBitImage& /*data*/)
{
       throw invalid_argument("Segemtation of binary images makes no sense");
}



EXPORT_2D P2DImage fuzzy_segment_2d(const C2DImage& src, size_t noOfClasses, const SFuzzySegParams& params,
                                    C2DImageVector& classes, P2DImage& gain)
{
       CSegment2d segment2D (noOfClasses, params);
       C2DFImageVec imagesVector = mia::accumulate (segment2D, src);

       for (size_t i = 0; i < noOfClasses; i++) {
              classes.push_back( P2DImage(new C2DFImage(imagesVector[i])));
       }

       gain = segment2D.get_gain_image();
       return segment2D.get_out_image();
}

NS_MIA_END

