/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef reg3d_deformer_hh
#define reg3d_deformer_hh

#include <memory>
#include <mia/3d/3DImage.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/interpolator.hh>


NS_MIA_BEGIN 

struct FDeformer3D: public TFilter<P3DImage> {
	FDeformer3D(const C3DFVectorfield& vf, const C3DInterpolatorFactory& ipfac): 
		_M_vf(vf), 
		_M_ipfac(ipfac)
		{
		}
	template <typename T> 
	P3DImage operator () (const T3DImage<T>& image) const {
		T3DImage<T> *timage = new T3DImage<T>(image.get_size(), image.get_attribute_list()); 
		P3DImage result(timage); 
		std::auto_ptr<T3DInterpolator<T> > interp(_M_ipfac.create(image.data())); 
		
#ifdef __OPENMP                
#pragma omp parallel for
#endif                
		for (size_t z = 0; z < image.get_size().z; ++z) {
			typename T3DImage<T>::iterator r = timage->begin_at(0,0,z); 
			C3DFVectorfield::const_iterator v = _M_vf.begin_at(0,0,z); 
			for (size_t y = 0; y < image.get_size().y; ++y)
				for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v)
					*r = (*interp)(C3DFVector(x - v->x, y - v->y, z - v->z));
		}
		return result; 
	}

	template <typename T> 
	P3DImage operator () (const T3DImage<T>& image, T3DImage<T>& result) const {
		std::auto_ptr<T3DInterpolator<T> > interp(_M_ipfac.create(image.data())); 
#ifdef __OPENMP                
#pragma omp parallel for
#endif                
		for (size_t z = 0; z < image.get_size().z; ++z) {
			typename T3DImage<T>::iterator r = result.begin_at(0,0,z); 
			C3DFVectorfield::const_iterator v = _M_vf.begin_at(0,0,z); 
			for (size_t y = 0; y < image.get_size().y; ++y)
				for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v)
					*r = (*interp)(C3DFVector(x - v->x, y - v->y, z - v->z));
		}
		return P3DImage(); 
	}
	
private: 
	C3DFVectorfield _M_vf; 
	C3DInterpolatorFactory _M_ipfac; 
}; 


NS_MIA_END

#endif
