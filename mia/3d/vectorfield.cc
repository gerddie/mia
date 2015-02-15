/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


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

	auto callback = [&a, &b, &help](const tbb::blocked_range<size_t>& range) {
		
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
	tbb::parallel_for( tbb::blocked_range<size_t>(0, a.get_size().z, 1), callback); 
	return a;
}

#define INSTANCIATE(TYPE) \
	template class  T3DDatafield<TYPE>;			\
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
