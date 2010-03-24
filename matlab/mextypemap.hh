
/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
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

#ifndef mextypemap_hh
#define mextypemap_hh

#include <mex.h>
#include <mia/2d.hh>
#include <mia/3d.hh>

#if MX_API_VER < 0x07040000
	typedef int mwSize; 
#else
	typedef size_t mwSize; 
#endif

template <typename T>
struct TMexTypeMap {
	enum { value = mxUNKNOWN_CLASS }; 
}; 


#define TYPEMAP(TYPE, CLASS) \
	template <> \
	struct TMexTypeMap<TYPE> { \
		enum { value = CLASS };  \
	}
	
TYPEMAP(signed char, mxINT8_CLASS);
TYPEMAP(unsigned char, mxUINT8_CLASS);
TYPEMAP(signed short, mxINT16_CLASS);
TYPEMAP(unsigned short, mxUINT16_CLASS);
TYPEMAP(signed int, mxINT32_CLASS);
TYPEMAP(unsigned int ,mxUINT32_CLASS);
#if HAVE_INT64
TYPEMAP(mia_int64 ,mxINT64_CLASS);
TYPEMAP(mia_uint64 ,mxUINT64_CLASS);
#endif 
TYPEMAP(float, mxSINGLE_CLASS);
TYPEMAP(double, mxDOUBLE_CLASS);

#undef TYPEMAP

char *mex_get_string(const mxArray *prhs); 
mia::P2DImage mex_get_image2d(const mxArray *src); 
mia::C2DFVectorfield mex_get_field2d(const mxArray *src); 
mia::P3DImage mex_get_image3d(const mxArray *src); 
mia::C3DFVectorfield mex_get_field3d(const mxArray *src); 


class MxResultGetter: public mia::TFilter<mxArray *> {
public: 	
	template <typename T>
	mxArray *operator () (const mia::T2DImage<T>& image) const {
		mxClassID type_class = (mxClassID) TMexTypeMap<T>::value; 
		mxArray * result = mxCreateNumericMatrix(image.get_size().x, image.get_size().y,
							 type_class, mxREAL);
		std::copy(image.begin(), image.end(), (T *)mxGetData(result)); 
		return result; 
	}
	template <typename T>
	mxArray *operator () (const mia::T3DImage<T>& image) const {
		mxClassID type_class = (mxClassID) TMexTypeMap<T>::value; 
		const mwSize mx_size[3] = {image.get_size().x, image.get_size().y, image.get_size().z}; 

		mxArray *result = mxCreateNumericArray(3, mx_size, type_class, mxREAL);
		std::copy(image.begin(), image.end(), (T *)mxGetData(result)); 
		return result; 
	}
}; 


#endif
