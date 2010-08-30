/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_deformer_hh
#define mia_2d_deformer_hh

#include <mia/2d/2DImage.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN 

struct FDeformer2D: public TFilter<P2DImage> {
	FDeformer2D(const C2DFVectorfield& vf, const C2DInterpolatorFactory& ipfac): 
		_M_vf(vf), 
		_M_ipfac(ipfac)
		{
		}
	template <typename T> 
	P2DImage operator () (const T2DImage<T>& image) const {
		T2DImage<T> *timage = new T2DImage<T>(image.get_size()); 

		std::shared_ptr<T2DInterpolator<T> >  interp(_M_ipfac.create(image.data())); 

		typename T2DImage<T>::iterator r = timage->begin(); 
		C2DFVectorfield::const_iterator v = _M_vf.begin(); 

		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(C2DFVector(x - v->x, y - v->y));
			}
		return P2DImage(timage); 
	}

	template <typename T> 
	void operator () (const T2DImage<T>& image, T2DImage<T>& result) const {
		assert(image.get_size() == result.get_size()); 

		std::shared_ptr<T2DInterpolator<T> >  interp(_M_ipfac.create(image.data())); 

		typename T2DImage<T>::iterator r = result.begin(); 
		C2DFVectorfield::const_iterator v = _M_vf.begin(); 

		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(C2DFVector(x - v->x, y - v->y));
			}
	}

private: 
	C2DFVectorfield _M_vf; 
	C2DInterpolatorFactory _M_ipfac; 
}; 

NS_MIA_END

#endif
