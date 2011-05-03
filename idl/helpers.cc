/* -*- mia-c++ -*-
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

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <idl_export.h>
#include <mia/2d.hh>
#include <mia/3d.hh>

using namespace std; 
using namespace mia; 


string idl_get_string(IDL_VPTR s)
{
	if (s->type != IDL_TYP_STRING)
		throw invalid_argument("idl_get_string expects an IDL_TYPE_STRING"); 
	return string(s->value.str.s, s->value.str.slen);
}

string idl_get_string(const string& name, IDL_VPTR s)
{
	if (s->type != IDL_TYP_STRING)
		throw invalid_argument(name + ": expect an IDL_TYPE_STRING"); 
	return string(s->value.str.s, s->value.str.slen);
}


int  idl_get_int(IDL_VPTR s)
{
	if (s->type == IDL_TYP_INT)
		return s->value.i; 
	if (s->type != IDL_TYP_UINT)
		return s->value.ui; 
	if (s->type != IDL_TYP_LONG)
		return s->value.l; 
	if (s->type != IDL_TYP_ULONG)
		return s->value.ul; 
	throw invalid_argument("idl_get_int expects an interger type"); 
}

double  idl_get_double(IDL_VPTR s)
{
	if (s->type == IDL_TYP_FLOAT)
 		return s->value.f;
	if (s->type == IDL_TYP_DOUBLE)
		return s->value.d;
	try {
		return idl_get_int(s); 
	}
	catch (invalid_argument& x) {
		throw invalid_argument("idl_get_int expects a scalar type"); 
	}
}	

template <typename T> 
P2DImage create_image(const C2DBounds& size, const void *data) 
{
	return P2DImage(new T2DImage<T>(size, (T*)data)); 
}

P2DImage idl2mia_image2d(IDL_VPTR s)
{
	if (!(s->flags & IDL_V_ARR))
		throw invalid_argument("idl_get_image2d: expects an IDL_TYPE_ARRAY"); 
	
	if (s->value.arr->n_dim != 2) 
		throw invalid_argument("idl_get_image2d: expects a 2-dimensional array"); 

	C2DBounds size(s->value.arr->dim[0], s->value.arr->dim[1]); 

	// check wheather elt_len,  arr_len and n_elts don't have surprises:
	

	switch (s->type) {
	case IDL_TYP_BYTE: return create_image<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_INT:  return create_image<signed short>(size, s->value.arr->data); 
	case IDL_TYP_UINT: return create_image<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_LONG: return create_image<signed int>(size, s->value.arr->data); 
	case IDL_TYP_ULONG:return create_image<unsigned int>(size, s->value.arr->data); 
#ifdef HAVE_INT64
	case IDL_TYP_LONG64:return create_image<mia_int64>(size, s->value.arr->data); 
	case IDL_TYP_ULONG64:return create_image<mia_uint64>(size, s->value.arr->data); 
#endif		
	case IDL_TYP_FLOAT: return create_image<float>(size, s->value.arr->data); 
	case IDL_TYP_DOUBLE:return create_image<double>(size, s->value.arr->data); 
	default: 
		throw invalid_argument("idl_get_image2d: input type not supported"); 
	}
	// never reached, only to avoid varnings of no return value; 
	return P2DImage(); 
}

template <typename T>
struct __idl_type_id {
};

template <>
struct __idl_type_id<bool> {
	enum {value = IDL_TYP_BYTE}; 
};

template <>
struct __idl_type_id<unsigned char> {
	enum {value = IDL_TYP_BYTE}; 
};

template <>
struct __idl_type_id<signed char> {
	enum {value = IDL_TYP_BYTE}; 
};


template <>
struct __idl_type_id<signed short> {
	enum {value = IDL_TYP_INT}; 
};

template <>
struct __idl_type_id<unsigned short> {
	enum {value = IDL_TYP_UINT}; 
};


template <>
struct __idl_type_id<signed int> {
	enum {value = IDL_TYP_LONG}; 
};

template <>
struct __idl_type_id<unsigned int> {
	enum {value = IDL_TYP_ULONG}; 
};


#ifdef HAVE_INT64
template <>
struct __idl_type_id<mia_int64> {
	enum {value = IDL_TYP_LONG64}; 
};

template <>
struct __idl_type_id<mia_uint64> {
	enum {value = IDL_TYP_ULONG64}; 
};
#endif

template <>
struct __idl_type_id<float> {
	enum {value = IDL_TYP_FLOAT}; 
};
template <>
struct __idl_type_id<double> {
	enum {value = IDL_TYP_DOUBLE}; 
};

struct __mia2idl : public TFilter<IDL_VPTR> {
	template <typename T> 
	IDL_VPTR operator() (const T2DImage<T>& image) const 
	{
		IDL_VPTR retval = (IDL_VPTR) IDL_MemAlloc(sizeof(IDL_VARIABLE), 0, 0); 
		if (!retval)
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		
		cvdebug() << "obtained pointer\n"; 

		retval->type = __idl_type_id<T>::value; 
		retval->flags = IDL_V_ARR; 
		retval->value.arr = (IDL_ARRAY*) IDL_MemAlloc(sizeof(IDL_ARRAY), 0, 0); 
		if (!retval->value.arr) {
			IDL_MemFree(retval, 0, 0); 
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		}

		cvdebug() << "allocate some more memory\n"; 
		retval->value.arr->data = (UCHAR *)IDL_MemAlloc(image.size() * sizeof(T), 0, 0); 

		if (!retval->value.arr->data) {
			IDL_MemFree(retval->value.arr, 0, 0); 
			IDL_MemFree(retval, 0, 0); 
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		}
		
		cvdebug() << "fill array info\n"; 
		retval->value.arr->elt_len = sizeof(T);
		retval->value.arr->arr_len = sizeof(T) * image.size();
		retval->value.arr->n_elts = image.size(); 
		retval->value.arr->n_dim = 2; 
		retval->value.arr->dim[0] = image.get_size().x; 
		retval->value.arr->dim[1] = image.get_size().y; 
		retval->value.arr->file_unit = 0; 
		retval->value.arr->flags = 0; 
		
		cvdebug() << "copy data\n"; 
		std::copy(image.begin(), image.end(), (T*)retval->value.arr->data);
		cvdebug() << "return image\n"; 
		return retval; 
	}
	
	template <typename T> 
	IDL_VPTR operator() (const T3DImage<T>& image) const 
	{
		IDL_VPTR retval = (IDL_VPTR) IDL_MemAlloc(sizeof(IDL_VARIABLE), 0, 0); 
		if (!retval)
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		
		retval->type = __idl_type_id<T>::value; 
		retval->flags = IDL_V_ARR; 

		retval->value.arr = (IDL_ARRAY*) IDL_MemAlloc(sizeof(IDL_ARRAY), 0, 0); 
		if (!retval->value.arr) {
			IDL_MemFree(retval, 0, 0); 
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		}


		retval->value.arr->data = (UCHAR *)IDL_MemAlloc(image.size() * sizeof(T), 0, 0); 
		if (!retval->value.arr->data) {
			IDL_MemFree(retval->value.arr, 0, 0); 
			IDL_MemFree(retval, 0, 0); 
			throw runtime_error("mia2idl_image2d: unable to allocate memory"); 
		}
		
		retval->value.arr->elt_len = sizeof(T);
		retval->value.arr->arr_len = sizeof(T) * image.size();
		retval->value.arr->n_elts = image.size(); 
		retval->value.arr->n_dim = 3; 
		retval->value.arr->dim[0] = image.get_size().x; 
		retval->value.arr->dim[1] = image.get_size().y; 
		retval->value.arr->dim[2] = image.get_size().z; 
		retval->value.arr->file_unit = 0; 
		retval->value.arr->flags = 0; 
		
		std::copy(image.begin(), image.end(), (T*)retval->value.arr->data);
		return retval; 
	}
	

}; 
	


IDL_VPTR mia2idl_image2d(const C2DImage& image) 
{
	return mia::filter(__mia2idl(), image);
}

IDL_VPTR mia2idl_image3d(const C3DImage& image) 
{
	return mia::filter(__mia2idl(), image);
}


template <typename T> 
P3DImage create_image(const C3DBounds& size, const void *data) 
{
	return P3DImage(new T3DImage<T>(size, (T*)data)); 
}

template <typename T>
C2DFVectorfield copy_to_field(const C2DBounds& size, const void *data) 
{
	C2DFVectorfield retval(size); 
	T *p  = (T *)data; 
	std::copy(p, p + retval.size() * 2, &retval(0,0).x); 
	return retval; 
}

template <typename T>
C3DFVectorfield copy_to_field(const C3DBounds& size, const void *data) 
{
	C3DFVectorfield retval(size); 
	T *p  = (T *)data; 
	std::copy(p, p + retval.size() * 3, &retval(0,0,0).x); 
	return retval; 
}


P3DImage idl2mia_image3d(IDL_VPTR s)
{
	if (!(s->flags & IDL_V_ARR))
		throw invalid_argument("idl_get_image3d: expects an IDL_TYP_ARRAY"); 
	
	if (s->value.arr->n_dim != 3) 
		throw invalid_argument("idl_get_image3d: expects a 2-dimensional array"); 

	C3DBounds size(s->value.arr->dim[0], s->value.arr->dim[1], s->value.arr->dim[2]); 

	// check wheather elt_len,  arr_len and n_elts don't have surprises:
	

	switch (s->type) {
	case IDL_TYP_BYTE: return create_image<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_INT:  return create_image<signed short>(size, s->value.arr->data); 
	case IDL_TYP_UINT: return create_image<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_LONG: return create_image<signed int>(size, s->value.arr->data); 
	case IDL_TYP_ULONG:return create_image<unsigned int>(size, s->value.arr->data); 
#ifdef HAVE_INT64
	case IDL_TYP_LONG64:return create_image<mia_int64>(size, s->value.arr->data); 
	case IDL_TYP_ULONG64:return create_image<mia_uint64>(size, s->value.arr->data); 
#endif		
	case IDL_TYP_FLOAT: return create_image<float>(size, s->value.arr->data); 
	case IDL_TYP_DOUBLE:return create_image<double>(size, s->value.arr->data); 
	default: 
		throw invalid_argument("idl_get_image3d: input type not supported"); 
	}
	// never reached, only to avoid varnings of no return value; 
	return P3DImage(); 
}

C2DFVectorfield  idl2mia_field2d(IDL_VPTR s)
{
	if (!(s->flags & IDL_V_ARR))
		throw invalid_argument("idl2mia_field2d: expects an IDL_TYPE_ARRAY"); 
	
	if (s->value.arr->n_dim != 3) 
		throw invalid_argument("idl2mia_field2d: expects a 3-dimensional array"); 

	if (s->value.arr->dim[0] != 2) 
		throw invalid_argument("idl2mia_field2d: first dimension must be 2"); 
	
	C2DBounds size(s->value.arr->dim[1], s->value.arr->dim[2]); 

	// check wheather elt_len,  arr_len and n_elts don't have surprises:
	

	switch (s->type) {
	case IDL_TYP_BYTE: return copy_to_field<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_INT:  return copy_to_field<signed short>(size, s->value.arr->data); 
	case IDL_TYP_UINT: return copy_to_field<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_LONG: return copy_to_field<signed int>(size, s->value.arr->data); 
	case IDL_TYP_ULONG:return copy_to_field<unsigned int>(size, s->value.arr->data); 
#ifdef HAVE_INT64
	case IDL_TYP_LONG64:return copy_to_field<mia_int64>(size, s->value.arr->data); 
	case IDL_TYP_ULONG64:return copy_to_field<mia_uint64>(size, s->value.arr->data); 
#endif		
	case IDL_TYP_FLOAT: return copy_to_field<float>(size, s->value.arr->data); 
	case IDL_TYP_DOUBLE:return copy_to_field<double>(size, s->value.arr->data); 
	default: 
		throw invalid_argument("idl2mia_field2d: input type not supported"); 
	}
	// never reached, only to avoid varnings of no return value; 
	return C2DFVectorfield(); 	
}

IDL_VPTR  mia2idl_field2d(const C2DFVectorfield& field)
{
	IDL_VPTR retval = (IDL_VPTR) IDL_MemAlloc(sizeof(IDL_VARIABLE), 0, 0); 
	if (!retval)
		throw runtime_error("mia2idl_field2d: unable to allocate memory"); 
	
	cvdebug() << "obtained pointer\n"; 
	
	retval->type = IDL_TYP_FLOAT; 
	retval->flags = IDL_V_ARR; 
	retval->value.arr = (IDL_ARRAY*) IDL_MemAlloc(sizeof(IDL_ARRAY), 0, 0); 
	if (!retval->value.arr) {
		IDL_MemFree(retval, 0, 0); 
		throw runtime_error("mia2idl_field2d: unable to allocate memory"); 
	}
	
	cvdebug() << "allocate some more memory\n"; 
	retval->value.arr->data = (UCHAR *)IDL_MemAlloc(field.size() * sizeof(float) * 2, 0, 0); 
	
	if (!retval->value.arr->data) {
		IDL_MemFree(retval->value.arr, 0, 0); 
		IDL_MemFree(retval, 0, 0); 
		throw runtime_error("mia2idl_field2d: unable to allocate memory"); 
	}
		
	cvdebug() << "fill array info\n"; 
	retval->value.arr->elt_len = sizeof(float);
	retval->value.arr->arr_len = sizeof(float) * field.size() * 2;
	retval->value.arr->n_elts = field.size(); 
	retval->value.arr->n_dim = 3; 
	retval->value.arr->dim[0] = 2; 
	retval->value.arr->dim[1] = field.get_size().x; 
	retval->value.arr->dim[2] = field.get_size().y; 
	retval->value.arr->file_unit = 0; 
	retval->value.arr->flags = 0; 
	
	cvdebug() << "copy data\n"; 
	std::copy(&field(0,0).x, &field(0,0).x + field.size() * 2, (float*)retval->value.arr->data);
	cvdebug() << "return field\n"; 
	return retval; 
	
}

C3DFVectorfield  idl2mia_field3d(IDL_VPTR s)
{
	if (!(s->flags & IDL_V_ARR))
		throw invalid_argument("idl2mia_field3d: expects an IDL_TYPE_ARRAY"); 
	
	if (s->value.arr->n_dim != 4) 
		throw invalid_argument("idl2mia_field3d: expects a 4-dimensional array"); 

	if (s->value.arr->dim[0] != 3) 
		throw invalid_argument("idl2mia_field3d: first dimension must be 3"); 
	
	C3DBounds size(s->value.arr->dim[1], s->value.arr->dim[2], s->value.arr->dim[3]); 

	// check wheather elt_len,  arr_len and n_elts don't have surprises:
	

	switch (s->type) {
	case IDL_TYP_BYTE: return copy_to_field<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_INT:  return copy_to_field<signed short>(size, s->value.arr->data); 
	case IDL_TYP_UINT: return copy_to_field<unsigned char>(size, s->value.arr->data); 
	case IDL_TYP_LONG: return copy_to_field<signed int>(size, s->value.arr->data); 
	case IDL_TYP_ULONG:return copy_to_field<unsigned int>(size, s->value.arr->data); 
#ifdef HAVE_INT64
	case IDL_TYP_LONG64:return copy_to_field<mia_int64>(size, s->value.arr->data); 
	case IDL_TYP_ULONG64:return copy_to_field<mia_uint64>(size, s->value.arr->data); 
#endif		
	case IDL_TYP_FLOAT: return copy_to_field<float>(size, s->value.arr->data); 
	case IDL_TYP_DOUBLE:return copy_to_field<double>(size, s->value.arr->data); 
	default: 
		throw invalid_argument("idl2mia_field3d: input type not supported"); 
	}
	// never reached, only to avoid varnings of no return value; 
	return C3DFVectorfield(); 	
}

IDL_VPTR  mia2idl_field3d(const C3DFVectorfield& field)
{
	IDL_VPTR retval = (IDL_VPTR) IDL_MemAlloc(sizeof(IDL_VARIABLE), 0, 0); 
	if (!retval)
		throw runtime_error("mia2idl_field3d: unable to allocate memory"); 
	
	cvdebug() << "obtained pointer\n"; 
	
	retval->type = IDL_TYP_FLOAT; 
	retval->flags = IDL_V_ARR; 
	retval->value.arr = (IDL_ARRAY*) IDL_MemAlloc(sizeof(IDL_ARRAY), 0, 0); 
	if (!retval->value.arr) {
		IDL_MemFree(retval, 0, 0); 
		throw runtime_error("mia2idl_field3d: unable to allocate memory"); 
	}
	
	cvdebug() << "allocate some more memory\n"; 
	retval->value.arr->data = (UCHAR *)IDL_MemAlloc(field.size() * sizeof(float) * 3, 0, 0); 
	
	if (!retval->value.arr->data) {
		IDL_MemFree(retval->value.arr, 0, 0); 
		IDL_MemFree(retval, 0, 0); 
		throw runtime_error("mia2idl_field3d: unable to allocate memory"); 
	}
		
	cvdebug() << "fill array info\n"; 
	retval->value.arr->elt_len = sizeof(float);
	retval->value.arr->arr_len = sizeof(float) * field.size() * 3;
	retval->value.arr->n_elts = field.size(); 
	retval->value.arr->n_dim = 4; 
	
	retval->value.arr->dim[0] = 3;
	retval->value.arr->dim[1] = field.get_size().x; 
	retval->value.arr->dim[2] = field.get_size().y; 
	retval->value.arr->dim[3] = field.get_size().z; 

	retval->value.arr->file_unit = 0; 
	retval->value.arr->flags = 0; 
	
	cvdebug() << "copy data\n"; 
	std::copy(&field(0,0,0).x, &field(0,0,0).x + field.size() * 3, (float*)retval->value.arr->data);
	cvdebug() << "return field\n"; 
	return retval; 
	
}




