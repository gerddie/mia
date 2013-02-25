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

#include <Python.h>
#include <numpy/arrayobject.h>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/filter.hh>
#include <mia/3d/filter.hh>


using namespace std;
using namespace mia;

static PyObject *MiaError; 

template <typename in, typename out, template  <class> class Image>
struct get_image {
	static  Image<out> apply(PyArrayObject *input) {
	}
}; 

template <template  <class> class Image, typename T> 
struct __dispatch_copy_line {
	static void apply(typename Image<T>::iterator out, char *data, int size) {
		memcpy(&out[0], data, size);
	}
}; 

template <template  <class> class Image> 
struct __dispatch_copy_line<Image, bool> {
	static void apply(typename Image<bool>::iterator out, char *data, int size) {
		copy(data, data+size, out); 
	}
}; 


template <typename in, typename out>
struct get_image<in, out, T2DImage > {
	static typename T2DImage<out>::Pointer apply(PyArrayObject *input) {
		TRACE_FUNCTION; 
		typedef typename T2DImage<out>::dimsize_type Dimsize;
		
		Dimsize size(input->dimensions[1], input->dimensions[0]);
		T2DImage<out> *result = new T2DImage<out>(size); 
		typename T2DImage<out>::Pointer presult(result); 
	
		cvdebug() << "Create mia image of size " << size 
			  << " type " << __type_descr<out>::value 
			  << "\n"; 

		/////////////////////////////////////////////////////////////////////////////////////////
		// this code is taken from the on-line example about iterators and adapted to my needs
		// http://docs.scipy.org/doc/numpy/reference/c-api.iterator.html#simple-iteration-example
		// 
		auto iter = NpyIter_New(input, NPY_ITER_READONLY|
					NPY_ITER_EXTERNAL_LOOP|
					NPY_ITER_REFS_OK,NPY_KEEPORDER,  NPY_NO_CASTING, 
					NULL); 
		unique_ptr<NpyIter, int (*)(NpyIter *p)> piter(iter, NpyIter_Deallocate);
		if (!iter) 
			throw runtime_error("Unable create iterater for input array"); 
		
		auto iternext = NpyIter_GetIterNext(iter, NULL);
		if (iternext == NULL)
			throw runtime_error("Unable to iterate over input array"); 
		
		auto innerstride = NpyIter_GetInnerStrideArray(iter)[0];
		auto itemsize = NpyIter_GetDescrArray(iter)[0]->elsize;
		auto innersizeptr = NpyIter_GetInnerLoopSizePtr(iter);
		auto dataptrarray = NpyIter_GetDataPtrArray(iter);

		if (innerstride == sizeof(out)) {
			// fast copy if stride is equal to target item size 
			size_t y = 0; 
			do {
				__dispatch_copy_line<T2DImage, out>::apply(result->begin_at(0,y), dataptrarray[0], itemsize * (*innersizeptr));
				++y; 
			} while (iternext(iter));
		} else {
			auto ir = result->begin(); 
			do {
				npy_intp size = *innersizeptr;
				char *src = dataptrarray[0];
				for(npy_intp i = 0; i < size; i++, src += innerstride, ++ir) {
					*ir = *(in*)src;
				}
			} while (iternext(iter));
		}
		//
		// End of copying
		////////////////////////////////////////////////////////////////
		return presult; 
	}
}; 


template <typename in, typename out>
struct get_image<in, out, T3DImage > {
	static typename T3DImage<out>::Pointer apply(PyArrayObject *input) {
		TRACE_FUNCTION; 
		typedef typename T3DImage<out>::dimsize_type Dimsize;
		
		Dimsize size(input->dimensions[2], input->dimensions[1], input->dimensions[0]);

		cvdebug() << "Create mia image of size " << size 
			  << " and type " << __type_descr<out>::value << "\n"; 

		T3DImage<out> *result = new T3DImage<out>(size); 
		typename T3DImage<out>::Pointer presult(result); 
	
		/////////////////////////////////////////////////////////////////////////////////////////
		// this code is taken from the on-line example about iterators and adapted to my needs
		// http://docs.scipy.org/doc/numpy/reference/c-api.iterator.html#simple-iteration-example
		// 
		auto iter = NpyIter_New(input, NPY_ITER_READONLY|
					NPY_ITER_EXTERNAL_LOOP|
					NPY_ITER_REFS_OK, 
					NPY_KEEPORDER, 
					NPY_NO_CASTING, 
					NULL); 
		unique_ptr<NpyIter, int (*)(NpyIter *p)> piter(iter, NpyIter_Deallocate);
		if (!iter) 
			throw runtime_error("Unable create iterater for input array"); 
		
		auto iternext = NpyIter_GetIterNext(iter, NULL);
		if (iternext == NULL)
			throw runtime_error("Unable to iterate over input array"); 
		
		auto innerstride = NpyIter_GetInnerStrideArray(iter)[0];
		auto itemsize = NpyIter_GetDescrArray(iter)[0]->elsize;
		auto innersizeptr = NpyIter_GetInnerLoopSizePtr(iter);
		auto dataptrarray = NpyIter_GetDataPtrArray(iter);
		
		if (innerstride == sizeof(out)) {
			// fast copy if stride is equal to item size 
			size_t z = 0; 
			size_t y = 0; 

			do {
				__dispatch_copy_line<T3DImage, out>::apply(result->begin_at(0,y,z), dataptrarray[0], itemsize * (*innersizeptr));
				++y; 
				if ( y >= size.y ) 
					++z; 
			} while (iternext(iter));
		} else {
			auto ir = result->begin(); 
			do {
				npy_intp size = *innersizeptr;
				char *src = dataptrarray[0];
				for(npy_intp i = 0; i < size; i++, src += innerstride, ++ir) {
					*ir = *(in*)src;
				}
			} while (iternext(iter));
		}
		//
		// End of copying
		////////////////////////////////////////////////////////////////
		return presult; 
	}
}; 
	
template <typename T> 
struct __mia_pixel_type_numarray_id {
	static const int value = NPY_USERDEF;
	static const char *name; 
}; 

template <typename T> 
const char *__mia_pixel_type_numarray_id<T>::name = "NPY_USERDEF"; 


#define SPECIALICE_ID(type, VALUE)		\
template <>					\
struct __mia_pixel_type_numarray_id<type> {	\
	static const int value = VALUE;		\
	static const char *name;		\
};						\
						\
					\
const char *__mia_pixel_type_numarray_id<type>::name = #VALUE;

SPECIALICE_ID(signed char, NPY_BYTE);
SPECIALICE_ID(unsigned char, NPY_UBYTE); 
SPECIALICE_ID(signed short, NPY_SHORT); 
SPECIALICE_ID(unsigned short, NPY_USHORT); 
SPECIALICE_ID(signed int, NPY_INT); 
SPECIALICE_ID(unsigned int, NPY_UINT); 
SPECIALICE_ID(signed long, NPY_LONG); 
SPECIALICE_ID(unsigned long, NPY_ULONG); 
SPECIALICE_ID(float, NPY_FLOAT); 
SPECIALICE_ID(double, NPY_DOUBLE); 

struct FConvertToPyArray: public TFilter<PyArrayObject*> {
	template <typename T> 
	PyArrayObject * operator () (const T2DImage<T>& image) const; 

	template <typename T> 
	PyArrayObject * operator () (const T3DImage<T>& image) const; 

};


template <template <class> class Image, typename T> 
struct __dispatch_py_array_copy {
	static void apply(char *output, const Image<T>& image)	{
		memcpy(output, &image.begin()[0], image.size() * sizeof(T));
	}
};

template <template <class> class Image> 
struct __dispatch_py_array_copy<Image, bool> {
	static void apply(char *output, const Image<bool>& image)	{
		copy(image.begin(), image.end(), output); 
	}
}; 

template <typename T> 
PyArrayObject * FConvertToPyArray::operator () (const T2DImage<T>& image) const
{
	TRACE_FUNCTION; 
	npy_intp dims[2]; 
	dims[1] = image.get_size().x; 
	dims[0] = image.get_size().y;
	cvdebug() << "Create array of size " << image.get_size() << " numpy type " << __mia_pixel_type_numarray_id<T>::name <<"\n"; 
	PyArrayObject* out_array = (PyArrayObject*)(PyArray_SimpleNew(2, dims, __mia_pixel_type_numarray_id<T>::value)); 
	if (!out_array) {
		throw runtime_error("Unable to create output array"); 
	}
	__dispatch_py_array_copy<T2DImage, T>::apply(out_array->data, image); 
	return out_array; 
}

template <typename T> 
PyArrayObject * FConvertToPyArray::operator () (const T3DImage<T>& image) const
{
	TRACE_FUNCTION; 
	npy_intp dims[3]; 
	dims[2] = image.get_size().x; 
	dims[1] = image.get_size().y;
	dims[0] = image.get_size().z;
	cvdebug() << "Create array of size " << image.get_size() 
		  << " numpy type " << __mia_pixel_type_numarray_id<T>::name << "("
		  <<__mia_pixel_type_numarray_id<T>::value<<")\n"; 
	
	PyArrayObject* out_array = (PyArrayObject*)(PyArray_SimpleNew(3, dims, __mia_pixel_type_numarray_id<T>::value)); 
	if (!out_array) {
		throw runtime_error("Unable to create output array"); 
	}
	__dispatch_py_array_copy<T3DImage, T>::apply(out_array->data, image); 
	return out_array; 
}


template <template <class> class Image>
typename Image<int>::Pointer mia_image_from_pyarray(PyArrayObject *input)
{
	TRACE_FUNCTION; 
	cvdebug() << "Get image numpy type " << input->descr->type_num 
		  << "and is " 
		  <<  ( PyArray_IS_C_CONTIGUOUS(input) ? " c-array " : " fortran array" )
		  <<"\n"; 
	switch (input->descr->type_num) {
	case NPY_BOOL:   return get_image<signed char, bool, Image>::apply(input);
	case NPY_BYTE:   return get_image<signed char, signed char, Image>::apply(input);
	case NPY_UBYTE:  return get_image<unsigned char, unsigned char, Image>::apply(input);
	case NPY_SHORT:  return get_image<signed short, signed short, Image>::apply(input);
	case NPY_USHORT: return get_image<unsigned short, unsigned short, Image>::apply(input);
	case NPY_INT:    return get_image<signed int, signed int, Image>::apply(input);
	case NPY_UINT:	 return get_image<unsigned int, unsigned int, Image>::apply(input);
#ifdef HAVE_INT64
	case NPY_LONG:   return get_image<signed long, signed long, Image>::apply(input);
	case NPY_ULONG:  return get_image<unsigned long, unsigned long, Image>::apply(input);
#else 
	case NPY_LONG:   return get_image<signed long, signed int, Image>::apply(input);
	case NPY_ULONG:  return get_image<unsigned long, unsigned int, Image>::apply(input);
#endif 
	case NPY_FLOAT:  return get_image<float, float, Image>::apply(input); 
	case NPY_DOUBLE: return get_image<double, double, Image>::apply(input); 
	default:
		THROW(invalid_argument, "mia doesn't support images of type number " << input->descr->type_num);
	}
}

template <typename Image>
PyArrayObject * mia_pyarray_from_image(const Image& image)
{
	TRACE_FUNCTION; 
	FConvertToPyArray convert; 
	return mia::filter(convert, image); 
}


static PyArrayObject *run_filters_2d(PyArrayObject *input, const vector<const char*>& filter_list) 
{
	TRACE_FUNCTION; 
	auto image = mia_image_from_pyarray<T2DImage>(input);
	auto result = run_filter_chain(image, filter_list);
	return mia_pyarray_from_image(*result);
}

static PyArrayObject *run_filters_3d(PyArrayObject *input, const vector<const char*>& filter_list) 
{
	TRACE_FUNCTION; 
	auto image = mia_image_from_pyarray<T3DImage>(input); 
	auto result = run_filter_chain(image, filter_list); 
	return mia_pyarray_from_image(*result);
}



static PyObject *run_filters(PyObject  */*self*/, PyObject *args)
{
	PyArrayObject *py_resultarray = NULL;
	PyArrayObject *py_inputarray;
	PyObject *py_filterlist;
	

	/* parse input */
	if (!PyArg_ParseTuple(args,"O!O",&PyArray_Type, &py_inputarray, &py_filterlist))
		return NULL;

	
	// get the filter descriptions
	auto mypy_filterlist = PySequence_Fast(py_filterlist, "mia.run_filters: expect some kind of sequence of filter descriptions.");
	auto length = PySequence_Length(mypy_filterlist); 

	vector<const char *> filter_list(length); 
	for (Py_ssize_t i = 0; i < length; ++i) {
		auto help = PySequence_Fast_GET_ITEM(mypy_filterlist,i); 
		const char *s = PyString_AsString(help); 
		if (!s) {
			THROW(invalid_argument, "mia.run_filter: filter " << i << " is not a string description");  
		}
		filter_list[i] = s; 
	}
	
	try {
		switch (py_inputarray->nd) {
		case 2: py_resultarray = run_filters_2d(py_inputarray, filter_list); 
			break; 
		case 3: py_resultarray = run_filters_3d(py_inputarray, filter_list); 
			break; 
		default:
			THROW(invalid_argument, "mia dosn't support images of " << py_inputarray->nd << " dimensions"); 
		}
	}
	
	catch (std::exception& x) {
		// translate c++ exception to python exception and re-throw python style 
		PyErr_SetString(MiaError, x.what());
		return NULL; 
	}
	
	// return result
	return PyArray_Return(py_resultarray);
}

static PyObject *set_verbose(PyObject  */*self*/, PyObject *args)
{
	const char *verbosity; 
	if (!PyArg_ParseTuple(args,"s", &verbosity))
		return NULL;
	
	try {
		cverb.set_verbosity(g_verbose_dict.get_value( verbosity) ); 
	}
	catch (invalid_argument& x) {
		PyErr_SetString(MiaError, x.what());
		return NULL;
	}
	return Py_BuildValue("i", 1);
}


static struct PyMethodDef mia_methods[]={
	{ "run_filters",  run_filters, METH_VARARGS, "run the given filter chain on an input image" },
	{ "set_verbose",  set_verbose, METH_VARARGS, "set the verbosity of the MIA output" },
	{ NULL, NULL, 0, NULL} /* stop mark */
};

/* ----------- module initialization -------------------------- */
extern "C" void EXPORT initmia() {
	PyObject *m,*d ;
	
	m=Py_InitModule("mia", mia_methods);
	
	/* initialize exception object */
	d=PyModule_GetDict(m) ; /* get module dictionary */
	MiaError=PyErr_Newcreate_exceptionion("mia.error",NULL,NULL) ;
	PyDict_SetItemString(d,"error",MiaError) ;
	
	import_array() ;
	
	if (PyErr_Occurred()) /* something went wrong ?*/
		Py_FatalError("can't initialize module mia") ;
}


