/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/datafield.cxx>
#include <mia/2d/iterator.cxx>
#include <mia/core/parameter.cxx>

NS_MIA_BEGIN

#define INSTANCIATE(TYPE) \
	template class  EXPORT_2D T2DDatafield<TYPE>;			\
	template class  EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class  EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::const_iterator>;


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



NS_MIA_END

