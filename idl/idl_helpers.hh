/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef idl_helpers_hh
#define idl_helpers_hh

#include <stdexcept>
#include <idl_export.h>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/3d.hh>


inline std::string idl_get_string(IDL_STRING *s)
{
	return std::string(s->s, s->slen);
}

mia::P2DImage get_2dimage(IDL_INT nx, IDL_INT ny, IDL_STRING *idl_in_type, const void *data); 
mia::P3DImage get_3dimage(IDL_INT nx, IDL_INT ny, IDL_INT nz, IDL_STRING *idl_in_type, const void *data); 

template <typename S, typename T> 
struct __dispatch_copy {
	static void apply(const mia::T2DImage<S>& image, T * target) {
		throw std::invalid_argument("Inage conversion not implemented"); 
	}
	static void apply(const mia::T3DImage<S>& image, T * target) {
		throw std::invalid_argument("Inage conversion not implemented"); 
	}
}; 

template <typename T> 
struct __dispatch_copy<T,T> {
	static void apply(const mia::T2DImage<T>& image, T * target) {
		std::copy(image.begin(), image.end(), target); 
	}
	static void apply(const mia::T3DImage<T>& image, T * target) {
		std::copy(image.begin(), image.end(), target); 
	}
}; 

template <typename T> 
struct FIDLImageCopy: public mia::TFilter<void> {
	
	FIDLImageCopy(void *out_data): 
		m_out_data((T *)out_data)
	{
	}
	template <typename Image> 
	void operator () ( const Image& image) const {
		__dispatch_copy<typename Image::value_type, T>::apply(image, m_out_data); 
	}
	
private: 
	T *m_out_data; 
}; 

template <typename I>
void save_image(const I& image, IDL_STRING *idl_out_type, void *out_data)
{
	std::string out_type = idl_get_string(idl_out_type); 
	mia::EPixelType out_pix_type = mia::CPixelTypeDict.get_value(out_type.c_str());

	switch (out_pix_type) {
	case mia::it_bit:   mia::filter(FIDLImageCopy<bool>(out_data), image); break; 
	case mia::it_sbyte: mia::filter(FIDLImageCopy<signed char>(out_data), image); break;
	case mia::it_ubyte: mia::filter(FIDLImageCopy<unsigned char>(out_data), image); break;
	case mia::it_sshort:mia::filter(FIDLImageCopy<signed short>(out_data), image); break;
	case mia::it_ushort:mia::filter(FIDLImageCopy<unsigned short>(out_data), image); break;
	case mia::it_sint:  mia::filter(FIDLImageCopy<signed int>(out_data), image); break;
	case mia::it_uint:  mia::filter(FIDLImageCopy<unsigned int>(out_data), image); break;
#ifdef HAVE_INT64
	case mia::it_slong: mia::filter(FIDLImageCopy<mia_int64>(out_data), image); break;
	case mia::it_ulong: mia::filter(FIDLImageCopy<mia_uint64>(out_data), image); break;
#endif
	case mia::it_float: mia::filter(FIDLImageCopy<float>(out_data), image); break;
	case mia::it_double:mia::filter(FIDLImageCopy<double>(out_data), image); break; 
	default:
		throw std::invalid_argument("filter3d: unknown output pixel type"); 
	}
}


#endif

