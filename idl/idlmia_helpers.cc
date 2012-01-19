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

#include <stdio.h>

#include <idl_export.h>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/3d.hh>

#include <idl/idl_helpers.hh>

using namespace std; 
using namespace mia; 

	
template <typename T, template <typename> class  I>
I<T> *create_image(typename I<T>::dimsize_type& size, const void *data) 
{
	return new I<T>(size, (T *)data); 
}


P2DImage get_2dimage(IDL_INT nx, IDL_INT ny, IDL_STRING *idl_in_type, const void *data) 
{
	string in_type = idl_get_string(idl_in_type); 
	EPixelType in_pix_type = CPixelTypeDict.get_value(in_type.c_str());
	C2DBounds size(nx, ny); 

	switch (in_pix_type) {
	case it_bit:   return P2DImage(create_image<bool, T2DImage>(size, data)); 
	case it_sbyte: return P2DImage(create_image<signed char, T2DImage>(size, data)); 
	case it_ubyte: return P2DImage(create_image<unsigned char, T2DImage>(size, data)); 
	case it_sshort:return P2DImage(create_image<signed short, T2DImage>(size, data)); 
	case it_ushort:return P2DImage(create_image<unsigned short, T2DImage>(size, data)); 
	case it_sint:  return P2DImage(create_image<signed int, T2DImage>(size, data)); 
	case it_uint:	return P2DImage(create_image<unsigned int, T2DImage>(size, data)); 
#ifdef HAVE_INT64
	case it_slong: return P2DImage(create_image<mia_int64, T2DImage>(size, data)); 
	case it_ulong: return P2DImage(create_image<mia_uint64, T2DImage>(size, data)); 
#endif
	case it_float: return P2DImage(create_image<float, T2DImage>(size, data)); 
	case it_double:return P2DImage(create_image<double, T2DImage>(size, data)); 
	default: 
		throw invalid_argument("get_2dimage: unknown input pixel type"); 
	}
	// code not reached
	return P2DImage(); 
}

P3DImage get_3dimage(IDL_INT nx, IDL_INT ny, IDL_INT nz, IDL_STRING *idl_in_type, const void *data) 
{
	string in_type = idl_get_string(idl_in_type); 
	EPixelType in_pix_type = CPixelTypeDict.get_value(in_type.c_str());
	C3DBounds size(nx, ny, nz); 

	switch (in_pix_type) {
	case it_bit:   return P3DImage(create_image<bool, T3DImage>(size, data)); 
	case it_sbyte: return P3DImage(create_image<signed char, T3DImage>(size, data)); 
	case it_ubyte: return P3DImage(create_image<unsigned char, T3DImage>(size, data)); 
	case it_sshort:return P3DImage(create_image<signed short, T3DImage>(size, data)); 
	case it_ushort:return P3DImage(create_image<unsigned short, T3DImage>(size, data)); 
	case it_sint:  return P3DImage(create_image<signed int, T3DImage>(size, data)); 
	case it_uint:	return P3DImage(create_image<unsigned int, T3DImage>(size, data)); 
#ifdef HAVE_INT64
	case it_slong: return P3DImage(create_image<mia_int64, T3DImage>(size, data)); 
	case it_ulong: return P3DImage(create_image<mia_uint64, T3DImage>(size, data)); 
#endif
	case it_float: return P3DImage(create_image<float, T3DImage>(size, data)); 
	case it_double:return P3DImage(create_image<double, T3DImage>(size, data)); 
	default: 
		throw invalid_argument("get_3dimage: unknown input pixel type"); 
	}
	// code not reached
	return P3DImage(); 
}

