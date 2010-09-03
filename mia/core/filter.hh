/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_core_filter_hh
#define mia_core_filter_hh

#include <cassert>
#include <stdexcept>
#include <mia/core/pixeltype.hh>
#include <mia/core/product_base.hh>
#include <mia/core/factory.hh>

#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

struct EXPORT_CORE combiner_type {
	static const char *value;
};

struct EXPORT_CORE filter_type {
	static const char *value;
};

#define DC(T, D) dynamic_cast<const T&>(D)
#define DV(T, D) dynamic_cast<T&>(D)

/**
   Base class for all filters that can be used with the pixel type transparent
   filter functions
   \a filter and \a filter_equal
   Derived classes have to define an operator like
   template <typename T>
   TFilter<R>::result_type operator () (const T& data) const;
   or
   template <typename T>
   TFilter<R>::result_type operator () (const T& data);
*/
template <typename R>
struct TFilter {
	/// defines the return type of the filter function
	typedef R result_type;
};


template <class D>
class EXPORT_HANDLER TImageFilter: public TFilter< std::shared_ptr<D > >, public CProductBase {
public:
	typedef D plugin_data; 
	typedef filter_type plugin_type; 
	
	typedef typename TFilter< std::shared_ptr<D > >::result_type result_type;
	virtual ~TImageFilter();
	result_type filter(const D& image) const;
private:
	virtual result_type do_filter(const D& image) const = 0;
};

template <class Image>
class EXPORT_HANDLER TImageFilterPlugin: public TFactory<TImageFilter<Image> > {
public:
	TImageFilterPlugin(char const * const  name):
		TFactory<TImageFilter<Image> >(name)
	{}
private:
	virtual bool do_test() const {
		cvwarn() << "TImageFilterPlugin::do_test is obsolete, use individual tests for the plugins\n";
		return true;
	};
};


template <template <class> class  D>
struct __bind_all {
	typedef D<bool> Dbool;
	typedef D<signed char> Dsc;
	typedef D<unsigned char> Duc;
	typedef D<signed short> Dss;
	typedef D<unsigned short> Dus;
	typedef D<signed int> Dsi;
	typedef D<unsigned int> Dui;
#ifdef HAVE_INT64
	typedef D<mia_int64> Dsl;
	typedef D<mia_uint64> Dul;
#endif
	typedef D<float> Dfloat;
	typedef D<double> Ddouble;
};

template <typename B>
struct Binder {
};


/**
   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   The filter object is const.
   \param f a filter to be applied to the data.
   \param data the input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a
   result of its operator ()
*/
template <typename F, typename B>
static typename F::result_type filter(const F& f, const B& b)
{
	typedef typename Binder<B>::Derived D;
	switch (b.get_pixel_type()) {
	case it_bit:   return f(DC(typename D::Dbool,b));
	case it_sbyte: return f(DC(typename D::Dsc,b));
	case it_ubyte: return f(DC(typename D::Duc,b));
	case it_sshort:return f(DC(typename D::Dss,b));
	case it_ushort:return f(DC(typename D::Dus,b));
	case it_sint:  return f(DC(typename D::Dsi,b));
	case it_uint:	 return f(DC(typename D::Dui,b));
#ifdef HAVE_INT64
	case it_slong: return f(DC(typename D::Dsl,b));
	case it_ulong: return f(DC(typename D::Dul,b));
#endif
	case it_float: return f(DC(typename D::Dfloat,b));
	case it_double:return f(DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   The filter object is const.
   \param f a filter to be applied to the data.
   \param data the input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a result of its operator ()
*/
template <typename F, typename B>
static typename F::result_type filter_inplace(const F& f, B& b)
{
	typedef typename Binder<B>::Derived D;
	switch (b.get_pixel_type()) {
	case it_bit:   return f(DV(typename D::Dbool,b));
	case it_sbyte: return f(DV(typename D::Dsc,b));
	case it_ubyte: return f(DV(typename D::Duc,b));
	case it_sshort:return f(DV(typename D::Dss,b));
	case it_ushort:return f(DV(typename D::Dus,b));
	case it_sint:  return f(DV(typename D::Dsi,b));
	case it_uint:	 return f(DV(typename D::Dui,b));
#ifdef HAVE_INT64
	case it_slong: return f(DV(typename D::Dsl,b));
	case it_ulong: return f(DV(typename D::Dul,b));
#endif
	case it_float: return f(DV(typename D::Dfloat,b));
	case it_double:return f(DV(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   A accumulator function that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independen base class.
   Data type dependency is introduced by using a templated derivative.
   The accumulator object is not const and can be used to accumulate data.
   \param f a accumulator to be applied to the data.
   \param data the input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a result of its operator ()
*/
template <typename F, typename B>
static typename F::result_type accumulate(F& f, const B& data)
{
	typedef typename Binder<B>::Derived D;
	switch (data.get_pixel_type()) {
	case it_bit:   return f(DC(typename D::Dbool,data));
	case it_sbyte: return f(DC(typename D::Dsc,data));
	case it_ubyte: return f(DC(typename D::Duc,data));
	case it_sshort:return f(DC(typename D::Dss,data));
	case it_ushort:return f(DC(typename D::Dus,data));
	case it_sint:  return f(DC(typename D::Dsi,data));
	case it_uint:	 return f(DC(typename D::Dui,data));
#ifdef HAVE_INT64
	case it_slong: return f(DC(typename D::Dsl,data));
	case it_ulong: return f(DC(typename D::Dul,data));
#endif
	case it_float: return f(DC(typename D::Dfloat,data));
	case it_double:return f(DC(typename D::Ddouble,data));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   Both input objects need to use the same data type
   \param f a filter to be applied to the data.
   \param a input data to be filtered
   \param b input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a result of its operator ()
*/
template <typename F, typename B>
static typename F::result_type filter_equal(const F& f, const B& a, const B& b)
{
	assert(a.get_pixel_type() == b.get_pixel_type());
	typedef typename Binder<B>::Derived D;
	switch (a.get_pixel_type()) {
	case it_bit:   return f(DC(typename D::Dbool, a), DC(typename D::Dbool,b));
	case it_sbyte: return f( DC(typename D::Dsc, a), DC(typename D::Dsc,b));
	case it_ubyte: return f( DC(typename D::Duc, a), DC(typename D::Duc,b));
	case it_sshort:return f( DC(typename D::Dss, a), DC(typename D::Dss,b));
	case it_ushort:return f( DC(typename D::Dus, a), DC(typename D::Dus,b));
	case it_sint:  return f( DC(typename D::Dsi, a), DC(typename D::Dsi,b));
	case it_uint:	 return f( DC(typename D::Dui, a), DC(typename D::Dui,b));
#ifdef HAVE_INT64
	case it_slong: return f( DC(typename D::Dsl, a), DC(typename D::Dsl,b));
	case it_ulong: return f( DC(typename D::Dul, a), DC(typename D::Dul,b));
#endif
	case it_float: return f( DC(typename D::Dfloat, a), DC(typename D::Dfloat,b));
	case it_double:return f( DC(typename D::Ddouble, a), DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


/**
   A filter type that handles data containers of equal types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   Both input objects need to use the same data type
   \param f a filter to be applied to the data.
   \param a input data to be filtered
   \param b output of filtered data
*/
template <typename F, typename B>
static void filter_equal_inplace(const F& f, const B& a, B& b)
{
	assert(a.get_pixel_type() == b.get_pixel_type());
	typedef typename Binder<B>::Derived D;
	switch (a.get_pixel_type()) {
	case it_bit:    f(DC(typename D::Dbool, a), DV(typename D::Dbool,b));break;
	case it_sbyte:  f( DC(typename D::Dsc, a), DV(typename D::Dsc,b));break;
	case it_ubyte:  f( DC(typename D::Duc, a), DV(typename D::Duc,b));break;
	case it_sshort: f( DC(typename D::Dss, a), DV(typename D::Dss,b));break;
	case it_ushort: f( DC(typename D::Dus, a), DV(typename D::Dus,b));break;
	case it_sint:   f( DC(typename D::Dsi, a), DV(typename D::Dsi,b));break;
	case it_uint:	 f( DC(typename D::Dui, a), DV(typename D::Dui,b));break;
#ifdef HAVE_INT64
	case it_slong:  f( DC(typename D::Dsl, a), DV(typename D::Dsl,b));break;
	case it_ulong:  f( DC(typename D::Dul, a), DV(typename D::Dul,b));break;
#endif
	case it_float:  f( DC(typename D::Dfloat, a), DV(typename D::Dfloat,b));break;
	case it_double: f( DC(typename D::Ddouble, a), DV(typename D::Ddouble,b));break;
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


template <typename F, typename A, typename B>
static typename F::result_type _filter(const F& f, const A& a, const B& b)
{
	typedef typename Binder<A>::Derived D;
	switch (a.get_pixel_type()) {
	case it_bit:   return f(DC(typename D::Dbool, a), b);
	case it_sbyte: return f(DC(typename D::Dsc,   a), b);
	case it_ubyte: return f(DC(typename D::Duc,   a), b);
	case it_sshort:return f(DC(typename D::Dss,   a), b);
	case it_ushort:return f(DC(typename D::Dus,   a), b);
	case it_sint:  return f(DC(typename D::Dsi,   a), b);
	case it_uint:	 return f(DC(typename D::Dui,   a), b);
#ifdef HAVE_INT64
	case it_slong: return f(DC(typename D::Dsl,   a), b);
	case it_ulong: return f(DC(typename D::Dul,   a), b);
#endif
	case it_float: return f(DC(typename D::Dfloat,a), b);
	case it_double:return f(DC(typename D::Ddouble,a), b);
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   Input objects can be of different types
   \param f a filter to be applied to the data.
   \param a input data to be filtered
   \param b input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a result of its operator ()
*/
template <typename F, typename A, typename B>
static typename F::result_type filter(const F& f, const A& a, const B& b)
{
	typedef typename Binder<B>::Derived D;
	switch (b.get_pixel_type()) {
	case it_bit:   return _filter(f, a, DC(typename D::Dbool,  b));
	case it_sbyte: return _filter(f, a, DC(typename D::Dsc,    b));
	case it_ubyte: return _filter(f, a, DC(typename D::Duc,    b));
	case it_sshort:return _filter(f, a, DC(typename D::Dss,    b));
	case it_ushort:return _filter(f, a, DC(typename D::Dus,    b));
	case it_sint:  return _filter(f, a, DC(typename D::Dsi,    b));
	case it_uint:	 return _filter(f, a, DC(typename D::Dui,    b));
#ifdef HAVE_INT64
	case it_slong: return _filter(f, a, DC(typename D::Dsl,    b));
	case it_ulong: return _filter(f, a, DC(typename D::Dul,    b));
#endif
	case it_float: return _filter(f, a, DC(typename D::Dfloat, b));
	case it_double:return _filter(f, a, DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}



template <typename F, typename A, typename B>
static typename F::result_type _accumulate(F& f, const A& a, const B& b)
{
	typedef typename Binder<A>::Derived D;
	switch (a.get_pixel_type()) {
	case it_bit:   return f(DC(typename D::Dbool, a), b);
	case it_sbyte: return f(DC(typename D::Dsc,   a), b);
	case it_ubyte: return f(DC(typename D::Duc,   a), b);
	case it_sshort:return f(DC(typename D::Dss,   a), b);
	case it_ushort:return f(DC(typename D::Dus,   a), b);
	case it_sint:  return f(DC(typename D::Dsi,   a), b);
	case it_uint:	 return f(DC(typename D::Dui,   a), b);
#ifdef HAVE_INT64
	case it_slong: return f(DC(typename D::Dsl,   a), b);
	case it_ulong: return f(DC(typename D::Dul,   a), b);
#endif
	case it_float: return f(DC(typename D::Dfloat,a), b);
	case it_double:return f(DC(typename D::Ddouble,a), b);
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


/**
   A accumulatro type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   Input objects can be of different types
   \param f a filter to be applied to the data.
   \param a input data to be filtered
   \param b input data to be filtered
   \returns whathever the filter \a f defines as return type and provides as a result of its operator ()
*/
template <typename F, typename A, typename B>
static typename F::result_type accumulate(F& f, const A& a, const B& b)
{
	typedef typename Binder<B>::Derived D;
	switch (b.get_pixel_type()) {
	case it_bit:   return _accumulate(f, a, DC(typename D::Dbool,  b));
	case it_sbyte: return _accumulate(f, a, DC(typename D::Dsc,    b));
	case it_ubyte: return _accumulate(f, a, DC(typename D::Duc,    b));
	case it_sshort:return _accumulate(f, a, DC(typename D::Dss,    b));
	case it_ushort:return _accumulate(f, a, DC(typename D::Dus,    b));
	case it_sint:  return _accumulate(f, a, DC(typename D::Dsi,    b));
	case it_uint:	 return _accumulate(f, a, DC(typename D::Dui,    b));
#ifdef HAVE_INT64
	case it_slong: return _accumulate(f, a, DC(typename D::Dsl,    b));
	case it_ulong: return _accumulate(f, a, DC(typename D::Dul,    b));
#endif
	case it_float: return _accumulate(f, a, DC(typename D::Dfloat, b));
	case it_double:return _accumulate(f, a, DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw invalid_argument("mia::accumulate: unsupported pixel type in image");
	}
}


#undef DC
#undef DV


template <class D>
TImageFilter<D>::~TImageFilter()
{
}

template <class D>
typename TImageFilter<D>::result_type
TImageFilter<D>::filter(const D& image) const
{
	return do_filter(image);
}

NS_MIA_END

#endif
