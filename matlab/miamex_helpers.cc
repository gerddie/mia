/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <stdexcept>
#include <mia/2d/2DImage.hh>
#include "mextypemap.hh"

using namespace mia; 
using namespace std; 

char *mex_get_string(const mxArray *prhs)
{
	int buflen = (mxGetM(prhs) * mxGetN(prhs)) + 1;
	char *s = (char *)mxCalloc(buflen, sizeof(char));
	int status = mxGetString(prhs, s, buflen);
	if(status != 0) 
		mexWarnMsgTxt("Not enough space. String is truncated.");
	return s; 
}

template <typename T>
P2DImage get_image(const C2DBounds& size, const mxArray *data)
{
	return P2DImage(new T2DImage<T>(size, (T*)mxGetData(data))); 
}

P2DImage mex_get_image2d(const mxArray *src)
{
	C2DBounds size(mxGetM(src), mxGetN(src));	
	if (size.x < 1 || size.y < 1)
		throw invalid_argument("mex_get_image2d: invalid image size, one dimension is zero");

	mxClassID class_id = mxGetClassID(src); 
	switch (class_id) {
	case mxINT8_CLASS:   return get_image<signed char>(size, src); break; 
	case mxUINT8_CLASS:  return get_image<unsigned char>(size, src); break; 
	case mxINT16_CLASS:  return get_image<signed short>(size, src); break; 
	case mxUINT16_CLASS: return get_image<unsigned short>(size, src); break; 
	case mxINT32_CLASS:  return get_image<signed int>(size, src); break; 
	case mxUINT32_CLASS: return get_image<unsigned int>(size, src); break; 
#if HAVE_INT64
	case mxINT64_CLASS:  return get_image<mia_int64>(size, src); break; 
	case mxUINT64_CLASS: return get_image<mia_uint64>(size, src); break; 
#endif 
	case mxSINGLE_CLASS: return get_image<float>(size, src); break; 
	case mxDOUBLE_CLASS: return get_image<double>(size, src); break; 
	default:{
		stringstream err; 
		err << "MIAMEX:reg2d: unknown input data type '" << mxGetClassName(src) << "'"; 
		throw invalid_argument(err.str()); 
	}
		
	}
}

template <typename T>
P3DImage get_image(const C3DBounds& size, const mxArray *data)
{
	return P3DImage(new T3DImage<T>(size, (T*)mxGetData(data))); 
}

P3DImage mex_get_image3d(const mxArray *src)
{
	if ( 3 != mxGetNumberOfDimensions(src) )
		throw invalid_argument("mex_get_image3d: expect a 3-dimensional array");
	
	const mwSize *dims = mxGetDimensions(src); 
	C3DBounds size(dims[0], dims[1], dims[2]);

	if (size.x < 1 || size.y < 1|| size.z < 1)
		throw invalid_argument("mex_get_image3d: invalid image size, one dimension is zero");

	mxClassID class_id = mxGetClassID(src); 
	switch (class_id) {
	case mxINT8_CLASS:   return get_image<signed char>(size, src); break; 
	case mxUINT8_CLASS:  return get_image<unsigned char>(size, src); break; 
	case mxINT16_CLASS:  return get_image<signed short>(size, src); break; 
	case mxUINT16_CLASS: return get_image<unsigned short>(size, src); break; 
	case mxINT32_CLASS:  return get_image<signed int>(size, src); break; 
	case mxUINT32_CLASS: return get_image<unsigned int>(size, src); break; 
#if HAVE_INT64
	case mxINT64_CLASS:  return get_image<mia_int64>(size, src); break; 
	case mxUINT64_CLASS: return get_image<mia_uint64>(size, src); break; 
#endif 
	case mxSINGLE_CLASS: return get_image<float>(size, src); break; 
	case mxDOUBLE_CLASS: return get_image<double>(size, src); break; 
	default:{
		stringstream err; 
		err << "MIAMEX:get_image: unknown input data type '" << mxGetClassName(src) << "'"; 
		throw invalid_argument(err.str()); 
	}
		
	}
}


template <typename T>
void copy_data(C2DFVectorfield& retval, const mxArray *src)
{
	T *p = (T*)mxGetData(src); 
	float *out = &retval(0,0).x; 
	std::copy(p, p + retval.size() * 2, out); 
}

C2DFVectorfield mex_get_field2d(const mxArray *src)
{
	C2DBounds size(mxGetM(src), mxGetN(src));
	if (size.x < 1 || size.y < 1)
		throw invalid_argument("mex_get_field2d: invalid image size, one dimension is zero");
	
	size.x /= 2; 
	C2DFVectorfield retval(size); 
	
	switch (mxGetClassID(src)) {
	case mxINT8_CLASS:   copy_data<signed char>(retval, src); break; 
	case mxUINT8_CLASS:  copy_data<unsigned char>(retval, src); break; 
	case mxINT16_CLASS:  copy_data<signed short>(retval, src); break; 
	case mxUINT16_CLASS: copy_data<unsigned short>(retval, src); break; 
	case mxINT32_CLASS:  copy_data<signed int>(retval, src); break; 
	case mxUINT32_CLASS: copy_data<unsigned int>(retval, src); break; 
#if HAVE_INT64
	case mxINT64_CLASS:  copy_data<mia_int64>(retval, src); break; 
	case mxUINT64_CLASS: copy_data<mia_uint64>(retval, src); break; 
#endif 
	case mxSINGLE_CLASS: copy_data<float>(retval, src); break; 
	case mxDOUBLE_CLASS: copy_data<double>(retval, src); break; 
	default:{
		stringstream err; 
		err << "MIAMEX:reg2d: unknown input data type '" << mxGetClassName(src) << "'"; 
		throw invalid_argument(err.str()); 
	}
		
	}
	return retval; 
}
	

template <typename T>
void copy_data(C3DFVectorfield& retval, const mxArray *src)
{
	T *p = (T*)mxGetData(src); 
	float *out = &retval(0,0, 0).x; 
	std::copy(p, p + retval.size() * 3, out); 
}


mxArray *mex_create_field3d(const C3DFVectorfield& field) 
{
	const mwSize mxsize[4] = {3, field.get_size().x, field.get_size().y, field.get_size().z }; 
	mxArray * result = mxCreateNumericArray( 4, mxsize, mxSINGLE_CLASS, mxREAL );
	memcpy(mxGetData(result), &field(0,0,0).x, field.size() * 3 * sizeof(float)); 
	return result; 
}


mxArray *mex_create_field2d(const C2DFVectorfield& field) 
{
	const mwSize mxsize[3] = {2, field.get_size().x, field.get_size().y }; 
	mxArray * result = mxCreateNumericArray( 3, mxsize, mxSINGLE_CLASS, mxREAL );
	memcpy(mxGetData(result), &field(0,0).x, field.size() * 2 * sizeof(float)); 
	return result; 
}


C3DFVectorfield mex_get_field3d(const mxArray *src)
{
	if ( 4 != mxGetNumberOfDimensions(src) )
		throw invalid_argument("mex_get_field3d: expect a 4-dimensional description array");
	
	const mwSize *dims = mxGetDimensions(src); 

	if (dims[0] != 3) 
		throw invalid_argument("mex_get_field3d: expect a 3d vectors");

	C3DBounds size(dims[1], dims[2], dims[3]);

	if (size.x < 1 || size.y < 1 || size.z < 1)
		throw invalid_argument("mex_get_field3d: invalid image size, one dimension is zero");
	
	C3DFVectorfield retval(size); 

	switch (mxGetClassID(src)) {
	case mxINT8_CLASS:   copy_data<signed char>(retval, src); break; 
	case mxUINT8_CLASS:  copy_data<unsigned char>(retval, src); break; 
	case mxINT16_CLASS:  copy_data<signed short>(retval, src); break; 
	case mxUINT16_CLASS: copy_data<unsigned short>(retval, src); break; 
	case mxINT32_CLASS:  copy_data<signed int>(retval, src); break; 
	case mxUINT32_CLASS: copy_data<unsigned int>(retval, src); break; 
#if HAVE_INT64
	case mxINT64_CLASS:  copy_data<mia_int64>(retval, src); break; 
	case mxUINT64_CLASS: copy_data<mia_uint64>(retval, src); break; 
#endif 
	case mxSINGLE_CLASS: copy_data<float>(retval, src); break; 
	case mxDOUBLE_CLASS: copy_data<double>(retval, src); break; 
	default:{
		stringstream err; 
		err << "MIAMEX:reg2d: unknown input data type '" << mxGetClassName(src) << "'"; 
		throw invalid_argument(err.str()); 
	}
		
	}
	return retval; 
}

