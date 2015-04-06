/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
	template class  EXPORT_2D T2DDatafield<TYPE>;			\
	template class  EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class  EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::const_iterator>; \
	template class  EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::iterator>; \
	template class  EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::const_iterator>;



INSTANCIATE(float); 

#ifdef LONG_64BIT
INSTANCIATE(signed long);
INSTANCIATE(unsigned long);
#endif
INSTANCIATE(double);
INSTANCIATE(unsigned int);
INSTANCIATE(signed int);
INSTANCIATE(unsigned short);
INSTANCIATE(signed short);
INSTANCIATE(bool);
INSTANCIATE(unsigned char);
INSTANCIATE(signed char);

DEFINE_TYPE_DESCR2(C2DBounds, "2dbounds"); 
DEFINE_TYPE_DESCR2(C2DFVector, "2dfvector"); 


template class EXPORT_2D  CTParameter<C2DFVector>;
template class EXPORT_2D  CTParameter<C2DBounds>;
template class EXPORT_2D  TTranslator<C2DFVector>; 

NS_MIA_END

