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

#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else
#  define EXPORT_2DDATAFIELD
#endif


#include <mia/3d/datafield.hh>
#include <mia/3d/datafield.cxx>
#include <mia/3d/iterator.cxx>
#include <mia/2d/iterator.cxx>
#include <mia/core/parameter.cxx>
#include <mia/core/attributes.cxx>

NS_MIA_BEGIN

template <>
T3DDatafield<bool>::value_type
T3DDatafield<bool>::strip_avg()
{
	throw std::invalid_argument("strip_avg() doesn't make sense for boolean fields");
}



#define INSTANCIATE(TYPE)						\
	template class  T3DDatafield<TYPE>;				\
	template class  range3d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator<T3DDatafield<TYPE>::const_iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::const_iterator>; \
	template class  range2d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  range2d_iterator<T3DDatafield<TYPE>::const_iterator>;



INSTANCIATE(double);
INSTANCIATE(float);
INSTANCIATE(unsigned int);
INSTANCIATE(int);

#ifdef LONG_64BIT
INSTANCIATE(signed long);
INSTANCIATE(unsigned long);
#endif
INSTANCIATE(short);
INSTANCIATE(unsigned short);
INSTANCIATE(unsigned char );
INSTANCIATE(signed char);
template class  T3DDatafield<bool>;

DEFINE_TYPE_DESCR2(C3DBounds, "3dbounds"); 
DEFINE_TYPE_DESCR2(C3DFVector, "3dfvector"); 

template class CTParameter<C3DBounds>;
template class CTParameter<C3DFVector>;
template class TTranslator<C3DFVector>; 
template class TAttribute<C3DFVector>; 

NS_MIA_END


