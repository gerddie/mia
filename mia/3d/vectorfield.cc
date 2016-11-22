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

#include <mia/core/parallel.hh>

#include <mia/3d/vectorfield.hh>

#include <mia/3d/datafield.cxx>
#include <mia/2d/datafield.cxx>
#include <mia/3d/iterator.cxx>
#include <mia/2d/iterator.cxx>

NS_MIA_BEGIN


EXPORT_3D C3DFVectorfield& operator += (C3DFVectorfield& a, const C3DFVectorfield& b)
{
	assert(a.get_size() == b.get_size());

	C3DFVectorfield help(a.get_size());
	std::copy(a.begin(), a.end(), help.begin());

	auto callback = [&a, &b, &help](const C1DParallelRange& range) {
		
		for (auto z = range.begin(); z != range.end();  ++z)  {
			C3DFVectorfield::iterator i = a.begin_at(0,0,z);
			C3DFVectorfield::const_iterator u = b.begin_at(0,0,z);
			for (size_t y = 0; y < a.get_size().y; ++y)  {
				for (size_t x = 0; x < a.get_size().x; ++x, ++i, ++u)  {
					C3DFVector xi = C3DFVector(x, y, z) - *u;
					*i = help.get_interpol_val_at(xi) +  *u;
				}
			}
		}
	}; 
	pfor( C1DParallelRange(0, a.get_size().z, 1), callback); 
	return a;
}



template <typename T>
T T3DVectorfield<T>::get_interpol_val_at(const C3DFVector& p) const
{
        // Calculate the coordinates and the distances
        size_t  x = (size_t)p.x;
        size_t  y = (size_t)p.y;
        size_t  z = (size_t)p.z;
        float  fx = p.x-x;
        float  fy = p.y-y;
        float  fz = p.z-z;

        float  dx = 1-fx;
        float  dy = 1-fy;
        float  dz = 1-fz;

        if ( x < this->get_size().x-1 && y  < this->get_size().y -1 && z < this->get_size().z -1 ) {

                const T *ptr = &(*this)[x + this->get_size().x * (y + this->get_size().y * z)];
                const T *ptr_h = &ptr[this->get_size().x];
                const T *ptr2 = &ptr[this->get_plane_size_xy()];
                const T *ptr2_h = &ptr2[this->get_size().x];
                const T a1 = T(dx * ptr[0]    + fx * ptr[1]);
                const T a3 = T(dx * ptr_h[0]  + fx * ptr_h[1]);
                const T a5 = T(dx * ptr2[0]   + fx * ptr2[1]);
                const T a7 = T(dx * ptr2_h[0] + fx * ptr2_h[1]);
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        } else {
                const T a1 = T(dx * (*this)(x  , y  , z  ) + fx * (*this)(x+1, y  , z  ));
                const T a3 = T(dx * (*this)(x  , y+1, z  ) + fx * (*this)(x+1, y+1, z  ));
                const T a5 = T(dx * (*this)(x  , y  , z+1) + fx * (*this)(x+1, y  , z+1));
                const T a7 = T(dx * (*this)(x  , y+1, z+1) + fx * (*this)(x+1, y+1, z+1));
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        }
}


#define INSTANCIATE(TYPE)						\
	template class  T3DDatafield<TYPE>;				\
	template class  range3d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator<T3DDatafield<TYPE>::const_iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::const_iterator>; 


#define INSTANCIATE2D(TYPE)						\
	template class  EXPORT_3D T2DDatafield<TYPE>;			\
	template class  range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class  range2d_iterator<T2DDatafield<TYPE>::const_iterator>;


INSTANCIATE2D(C3DFVector); 
INSTANCIATE2D(C3DDVector);

INSTANCIATE(C3DFVector);
INSTANCIATE(C3DDVector);
template class T3DVectorfield<C3DFVector>;
template class T3DVectorfield<C3DDVector>;



NS_MIA_END
