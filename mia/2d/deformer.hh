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

#ifndef mia_2d_deformer_hh
#define mia_2d_deformer_hh

#include <mia/2d/image.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN


/**
   @ingroup filtering

   Deform an image by using a dense vector field as transformation.
   \remark obsolete-should be done by using C2DTransform
 */

struct FDeformer2D: public TFilter<P2DImage> {

       /**
          Construor taking the
          \param vf vector field defining the transformation like T(x) := x - vf(x)
          \param ipfac the interpolation factory used for image interpolation
        */
       FDeformer2D(const C2DFVectorfield& vf, const C2DInterpolatorFactory& ipfac):
              m_vf(vf),
              m_ipfac(ipfac)
       {
       }

       /**
          Operator to run the transformation like a filter to be called by mia::filter
          \tparam T pixel type if the input image
          \param image the input image
          \returns the transformed image
        */
       template <typename T>
       P2DImage operator () (const T2DImage<T>& image) const
       {
              T2DImage<T> *timage = new T2DImage<T>(image.get_size());
              std::shared_ptr<T2DInterpolator<T>>  interp(m_ipfac.create(image.data()));
              typename T2DImage<T>::iterator r = timage->begin();
              C2DFVectorfield::const_iterator v = m_vf.begin();

              for (size_t y = 0; y < image.get_size().y; ++y)
                     for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
                            *r = (*interp)(C2DFVector(x - v->x, y - v->y));
                     }

              return P2DImage(timage);
       }

       /**
          Operator to transform the image and store the result in a pre-allocated
          image
          \param[in] image the input image
          \param[out] result the output image
       */
       template <typename T>
       void operator () (const T2DImage<T>& image, T2DImage<T>& result) const
       {
              assert(image.get_size() == result.get_size());
              std::shared_ptr<T2DInterpolator<T>>  interp(m_ipfac.create(image.data()));
              typename T2DImage<T>::iterator r = result.begin();
              C2DFVectorfield::const_iterator v = m_vf.begin();

              for (size_t y = 0; y < image.get_size().y; ++y)
                     for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
                            *r = (*interp)(C2DFVector(x - v->x, y - v->y));
                     }
       }

private:
       C2DFVectorfield m_vf;
       C2DInterpolatorFactory m_ipfac;
};

NS_MIA_END

#endif
