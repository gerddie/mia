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

#include <mia/2d/defines2d.hh>
#include <mia/2d/datafield.cxx>
#include <mia/2d/iterator.cxx>
#include <mia/core/parameter.cxx>
#include <mia/core/attributes.cxx>

NS_MIA_BEGIN

#define INSTANCIATE(TYPE) \
	template class EXPORT_2D T2DDatafield<TYPE>;			\
	template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::const_iterator>; \
	template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::iterator>; \
	template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::const_iterator>;

#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif



INSTANCIATE(float); 
INSTANCIATE(double);
INSTANCIATE(int64_t);
INSTANCIATE(uint64_t);
INSTANCIATE(int32_t);
INSTANCIATE(uint32_t);
INSTANCIATE(int16_t);
INSTANCIATE(uint16_t);
INSTANCIATE(int8_t);
INSTANCIATE(uint8_t);

INSTANCIATE(C2DBounds);
INSTANCIATE(C2DFVector)
INSTANCIATE(C2DDVector)

template class  EXPORT_2D T2DDatafield<bool>;

DEFINE_TYPE_DESCR2(C2DBounds, "2dbounds"); 
DEFINE_TYPE_DESCR2(C2DFVector, "2dfvector"); 

template class EXPORT_2D CTParameter<C2DFVector>;
template class EXPORT_2D CTParameter<C2DBounds>;
template class EXPORT_2D TTranslator<C2DFVector>; 
template class EXPORT_2D TAttribute<C2DFVector>; 

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif 

NS_MIA_END

