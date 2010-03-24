/*  -*- mia-c++  -*-
 * Copyright (c) 2004 
 *   Max-Planck-Institute for Human Cognitive and Brain Science	
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: 3DImage.hh 732 2005-11-14 18:06:39Z write1 $

/*! \brief A 3D templated image class

2A fully templated class for 3D images

\file 3DImage.hh
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else 
#  define EXPORT_2DDATAFIELD 
#endif


#include <mia/3d/3DDatafield.hh>
#include <mia/3d/3DDatafield.cxx>

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



template class  EXPORT_3D T3DDatafield<double>; 
template class  EXPORT_3D T3DDatafield<float>; 
template class  EXPORT_3D T3DDatafield<unsigned int>; 
template class  EXPORT_3D T3DDatafield<int>; 

#ifdef HAVE_INT64
template class  EXPORT_3D T3DDatafield<mia_int64>; 
template class  EXPORT_3D T3DDatafield<mia_uint64>; 
#endif
template class  EXPORT_3D T3DDatafield<short>; 
template class  EXPORT_3D T3DDatafield<unsigned short>; 
template class  EXPORT_3D T3DDatafield<unsigned char >;
template class  EXPORT_3D T3DDatafield<signed char >;
template class  EXPORT_3D T3DDatafield<bool>;



NS_MIA_END


