/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else
#  define EXPORT_2DDATAFIELD
#endif


#include <mia/3d/3DDatafield.hh>
#include <mia/3d/3DDatafield.cxx>
#include <mia/3d/iterator.cxx>
#include <mia/2d/iterator.cxx>
#include <mia/core/parameter.cxx>

NS_MIA_BEGIN

template <>
T3DDatafield<bool>::value_type
T3DDatafield<bool>::strip_avg()
{
	throw std::invalid_argument("strip_avg() doesn't make sense for boolean fields");
}


template <>
T3DDatafield<bool>::value_type
T3DDatafield<bool>::get_trilin_interpol_val_at(const T3DVector<float >& p) const
{
        // Calculate the coordinates and the distances
        size_t  x = (size_t )p.x;
        size_t  y = (size_t )p.y;
        size_t  z = (size_t )p.z;
        float  fx = p.x-x;
        float  fy = p.y-y;
        float  fz = p.z-z;

        float  dx = 1-fx;
        float  dy = 1-fy;
        float  dz = 1-fz;

        register float a1,a3,a5,a7;

	a1 = (dx * (*this)(x  , y  , z  ) + fx * (*this)(x+1, y  , z  ));
	a3 = (dx * (*this)(x  , y+1, z  ) + fx * (*this)(x+1, y+1, z  ));
	a5 = (dx * (*this)(x  , y  , z+1) + fx * (*this)(x+1, y  , z+1));
	a7 = (dx * (*this)(x  , y+1, z+1) + fx * (*this)(x+1, y+1, z+1));

        register float b1 = dy * a1 + fy * a3;
        register float b2 = dy * a5 + fy * a7;

	return  (dz * b1 + fz * b2) > 0.5;
}

#define INSTANCIATE(TYPE) \
	template class  EXPORT_3D T3DDatafield<TYPE>;			\
	template class  EXPORT_3D range3d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  EXPORT_3D range3d_iterator<T3DDatafield<TYPE>::const_iterator>; \
	template class  EXPORT_3D range2d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  EXPORT_3D range2d_iterator<T3DDatafield<TYPE>::const_iterator>;



INSTANCIATE(double);
INSTANCIATE(float);
INSTANCIATE(unsigned int);
INSTANCIATE(int);

#ifdef HAVE_INT64
INSTANCIATE(mia_int64);
INSTANCIATE(mia_uint64);
#endif
INSTANCIATE(short);
INSTANCIATE(unsigned short);
INSTANCIATE(unsigned char );
INSTANCIATE(signed char);
INSTANCIATE(bool);

DEFINE_TYPE_DESCR2(C3DBounds, "3dbounds"); 
DEFINE_TYPE_DESCR2(C3DFVector, "3dfvector"); 
template class EXPORT_3D  CTParameter<C3DBounds>;
template class EXPORT_2D  CTParameter<C3DFVector>;

NS_MIA_END


