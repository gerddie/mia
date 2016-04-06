/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef reg3d_deformer_hh
#define reg3d_deformer_hh

#include <memory>
#include <mia/core/parallel.hh>


#include <mia/3d/image.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/interpolator.hh>
#include <mia/core/threadedmsg.hh>


NS_MIA_BEGIN 

/**
   @ingroup registration 
   @brief A filter to transform  an image
   @remark obsolete, should be done by using C3DTransformation
*/

struct FDeformer3D: public TFilter<P3DImage> {
	FDeformer3D(const C3DFVectorfield& vf, const C3DInterpolatorFactory& ipfac): 
		m_vf(vf), 
		m_ipfac(ipfac)
		{
		}
	template <typename T> 
	P3DImage operator () (const T3DImage<T>& image) const {
		T3DImage<T> *timage = new T3DImage<T>(m_vf.get_size(), image); 
		P3DImage result(timage); 
		this->operator()(image, *timage); 
		return result; 
	}

	template <typename T> 
	P3DImage operator () (const T3DImage<T>& image, T3DImage<T>& result) const {
		assert(result.get_size() == m_vf.get_size()); 
		std::unique_ptr<T3DConvoluteInterpolator<T> > interp(m_ipfac.create(image.data())); 
		const auto& rinterp = *interp; 

		auto callback = [this, &rinterp, &result](const C1DParallelRange& range){
			CThreadMsgStream thread_stream;
			CWeightCache cache = rinterp.create_cache(); 
			for (auto z = range.begin(); z != range.end();++z) {
				auto r = result.begin_at(0,0,z); 
				auto v = m_vf.begin_at(0,0,z); 
				for (size_t y = 0; y < result.get_size().y; ++y)
					for (size_t x = 0; x < result.get_size().x; ++x, ++r, ++v)
						*r = rinterp(C3DFVector(x - v->x, y - v->y, z - v->z), cache);
			}
		}; 
		pfor(C1DParallelRange(0, result.get_size().z, 1), callback); 
		return P3DImage(); 
	}
	
private: 
	C3DFVectorfield m_vf; 
	C3DInterpolatorFactory m_ipfac; 
}; 


NS_MIA_END

#endif
