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

#ifndef mia_core_filter_hh
#define mia_core_filter_hh

#include <cassert>
#include <stdexcept>
#include <mia/core/pixeltype.hh>
#include <mia/core/product_base.hh>
#include <mia/core/factory.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

/** @cond INTERNAL
    \brief type for combiner plug-in path handling 
 */
struct EXPORT_CORE combiner_type {
	static const char *type_descr;
};

/** 
    \brief type for combiner plug-in path handling 
*/
struct EXPORT_CORE filter_type {
	static const char *type_descr;
};

#define DC(T, D) dynamic_cast<const T&>(D)
#define DV(T, D) dynamic_cast<T&>(D)

/// @endcond 

/**
   \ingroup filtering 
   
   \brief base class for all filer type functors. 

   Base class for all filters that can be used with the pixel type transparent
   filter functions
   \a filter and \a filter_equal
   Derived classes have to define an operator like
   template &lt typename T &lt
   TFilter &ltR&gt::result_type operator () (const T& data) const;
   or
   template &lttypename T &gt 
   TFilter &ltR&gt::result_type operator () (const T& data);
*/
template <typename R>
struct TFilter {
	/// defines the return type of the filter function
	typedef R result_type;
};

/**
   \ingroup filtering

   \brief Generic interface class to data filters. 

   This class provides the basic interface to image filtering.  
   \tparam D the data type to be filtered 
*/


template <class D>
class EXPORT_HANDLER TDataFilter: public TFilter< std::shared_ptr<D > >, public CProductBase {
public:

	/// plugin handler helper type 
	typedef D plugin_data;
	/// plugin handler helper type 
	typedef filter_type plugin_type; 

	/// defines the image type handled by the image filter 
	typedef D Image; 
	
	/// pointer type of the data filtered by this filter 
	typedef std::shared_ptr<TDataFilter<D> > Pointer; 

	/// result type of this filter 
	typedef typename TFilter< std::shared_ptr<D > >::result_type result_type;
	
	virtual ~TDataFilter();

	/** run the filter 
	   \param image must be of a type that has Binder trait defined.  
	 */ 
	result_type filter(const Image& image) const;

	/** run the filter from a pointer type 
	   \param pimage must be of a type D that has Binder trait defined. 
	 */ 
	result_type filter(std::shared_ptr<D> pimage) const;

private:
	virtual result_type do_filter(const Image& image) const = 0;
	virtual result_type do_filter(std::shared_ptr<D> image) const;

};

template <class D>
class EXPORT_HANDLER TDataFilterChained: public TDataFilter<D> {
	typedef typename TDataFilter<D>::result_type result_type; 
public: 
	typedef typename TDataFilter<D>::Pointer Pointer; 
	
	void push_back(Pointer f) {
		m_chain.push_back(f); 
	}
private: 
	virtual result_type do_filter(const D& image) const {
		assert(m_chain.size() > 0); 
		
		cvdebug() << "Run chained filter '" << m_chain[0]->get_init_string() << "'\n"; 
		result_type result = m_chain[0]->filter(image); 
		for(auto f = m_chain.begin() + 1; f != m_chain.end(); ++f)  {
		cvdebug() << "Run chained filter '" << (*f)->get_init_string() << "'\n"; 
			result = (*f)->filter(*result); 
		}
		return result; 
	}
	std::vector<Pointer> m_chain; 
}; 


/**
   \ingroup filtering

   \brief Generic image filter plugin base 

   This class provides the generic base class for image filter 
   plug-ins. 
   \tparam Image the image type for which the filters are defined. 
   
 */

template <class Image>
class EXPORT_HANDLER TDataFilterPlugin: public TFactory<TDataFilter<Image> > {
public:
	typedef typename TFactory<TDataFilter<Image> >::Product Product; 
	/// Constructor that sets the plug-in name 
	TDataFilterPlugin(char const * const  name):
		TFactory<TDataFilter<Image> >(name)
	{}
};

template <typename D>
struct plugin_can_chain<TDataFilterPlugin<D>> {
	static constexpr bool value = true; 
	typedef TDataFilterChained<D> Chained; 
}; 


/**
   @cond INTERNAL 
   \ingroup traits
   \brief helper class to get the type names of a templated derived class for all supported pixel types 
*/
template <template <class> class  D>
struct __bind_all {
	typedef D<bool> Dbool;
	typedef D<signed char> Dsc;
	typedef D<unsigned char> Duc;
	typedef D<signed short> Dss;
	typedef D<unsigned short> Dus;
	typedef D<signed int> Dsi;
	typedef D<unsigned int> Dui;
#ifdef LONG_64BIT
	typedef D<signed long> Dsl;
	typedef D<unsigned long> Dul;
#endif
	typedef D<float> Dfloat;
	typedef D<double> Ddouble;
};

/**
   \ingroup traits

   \brief a trait to define types for images of all pixel types that are derived 
   from a common base class. 

   The class needs to have a typedef 
   - typedef __bind_all<sometype> Derived; 
   See Binder<C2DImage> how to do this properly. 
   \tparam B the base class from which the templated images are derived
 */
template <typename B>
struct Binder {
};
/// @endcond 


/**
   \ingroup filtering

   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   The filter object is const.
   \tparam a Filter that must provide a type result_type, and an operator 
   template &lt; typename T &gt; 
        F::result_type F::operator()(const D<T>& b) const; 
	with D&lt;T&gt; being a derived class of B 
   \tparam b the data to be processed in form of a reference to the base class of a class 
             template D&lt;T&gt; 
   \param f a filter to be applied to the data.
   \param b the input data to be filtered
   \returns whathever the filter \a F defines as return type and provides as a
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
#ifdef LONG_64BIT
	case it_slong: return f(DC(typename D::Dsl,b));
	case it_ulong: return f(DC(typename D::Dul,b));
#endif
	case it_float: return f(DC(typename D::Dfloat,b));
	case it_double:return f(DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   \ingroup filtering

   The in-place type of the function typename F::result_type filter(const F& f, const B& b). 
   A filter type that handles data containers of different types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   The filter object is const.
   \param f a filter to be applied to the data.
   \param b the input data to be filtered
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
#ifdef LONG_64BIT
	case it_slong: return f(DV(typename D::Dsl,b));
	case it_ulong: return f(DV(typename D::Dul,b));
#endif
	case it_float: return f(DV(typename D::Dfloat,b));
	case it_double:return f(DV(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

/**
   \ingroup filtering

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
#ifdef LONG_64BIT
	case it_slong: return f(DC(typename D::Dsl,data));
	case it_ulong: return f(DC(typename D::Dul,data));
#endif
	case it_float: return f(DC(typename D::Dfloat,data));
	case it_double:return f(DC(typename D::Ddouble,data));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

template <typename F, typename INOUT, typename IN>
static typename F::result_type _combine_inplace(const F& f, INOUT& inout, const IN& in)
{
	typedef typename Binder<IN>::Derived D;
	switch (in.get_pixel_type()) {
	case it_bit:   return f(inout, DC(typename D::Dbool, in));
	case it_sbyte: return f(inout, DC(typename D::Dsc, in));
	case it_ubyte: return f(inout, DC(typename D::Duc, in));
	case it_sshort:return f(inout, DC(typename D::Dss, in));
	case it_ushort:return f(inout, DC(typename D::Dus, in));
	case it_sint:  return f(inout, DC(typename D::Dsi, in));
	case it_uint:	 return f(inout, DC(typename D::Dui, in));
#ifdef LONG_64BIT
	case it_slong: return f(inout, DC(typename D::Dsl, in));
	case it_ulong: return f(inout, DC(typename D::Dul, in));
#endif
	case it_float: return f(inout, DC(typename D::Dfloat, in));
	case it_double:return f(inout, DC(typename D::Ddouble, in));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}

template <typename F, typename INOUT, typename IN>
static typename F::result_type combine_inplace(const F& f, INOUT& inout, const IN& in)
{
	typedef typename Binder<INOUT
>::Derived D;
	switch (inout.get_pixel_type()) {
	case it_bit:   return _combine_inplace(f, DV(typename D::Dbool, inout), in);
	case it_sbyte: return _combine_inplace(f, DV(typename D::Dsc, inout), in);
	case it_ubyte: return _combine_inplace(f, DV(typename D::Duc, inout), in);
	case it_sshort:return _combine_inplace(f, DV(typename D::Dss, inout), in);
	case it_ushort:return _combine_inplace(f, DV(typename D::Dus, inout), in);
	case it_sint:  return _combine_inplace(f, DV(typename D::Dsi, inout), in);
	case it_uint:  return _combine_inplace(f, DV(typename D::Dui, inout), in);
#ifdef LONG_64BIT
	case it_slong: return _combine_inplace(f, DV(typename D::Dsl, inout), in);
	case it_ulong: return _combine_inplace(f, DV(typename D::Dul, inout), in);
#endif
	case it_float: return _combine_inplace(f, DV(typename D::Dfloat, inout), in);
	case it_double:return _combine_inplace(f, DV(typename D::Ddouble, inout), in);
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}

	
}


/**
   \ingroup filtering

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
#ifdef LONG_64BIT
	case it_slong: return f( DC(typename D::Dsl, a), DC(typename D::Dsl,b));
	case it_ulong: return f( DC(typename D::Dul, a), DC(typename D::Dul,b));
#endif
	case it_float: return f( DC(typename D::Dfloat, a), DC(typename D::Dfloat,b));
	case it_double:return f( DC(typename D::Ddouble, a), DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


/**
   \ingroup filtering

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
#ifdef LONG_64BIT
	case it_slong:  f( DC(typename D::Dsl, a), DV(typename D::Dsl,b));break;
	case it_ulong:  f( DC(typename D::Dul, a), DV(typename D::Dul,b));break;
#endif
	case it_float:  f( DC(typename D::Dfloat, a), DV(typename D::Dfloat,b));break;
	case it_double: f( DC(typename D::Ddouble, a), DV(typename D::Ddouble,b));break;
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


/**
   \ingroup filtering

   A filter type that handles data containers of equal types.
   The data container is provided by a pointer or reference to its type independedn base class.
   Data type dependency is introduced by using a templated derivative.
   Both input objects need to use the same data type
   \param f a filter to be applied to the data.
   \param a input data to be filtered
   \param b output of filtered data
*/
template <typename F, typename B, typename O>
static typename F::result_type filter_and_output(const F& f, const B& a, O& b)
{
	typedef typename Binder<B>::Derived D;
	switch (a.get_pixel_type()) {
	case it_bit:    return f(DC(typename D::Dbool, a), b);break;
	case it_sbyte:  return f(DC(typename D::Dsc, a), b);break;
	case it_ubyte:  return f(DC(typename D::Duc, a), b);break;
	case it_sshort: return f(DC(typename D::Dss, a), b);break;
	case it_ushort: return f(DC(typename D::Dus, a), b);break;
	case it_sint:   return f(DC(typename D::Dsi, a), b);break;
	case it_uint:	return f(DC(typename D::Dui, a), b);break;
#ifdef HAVE_INT64
	case it_slong:  return f(DC(typename D::Dsl, a), b);break;
	case it_ulong:  return f(DC(typename D::Dul, a), b);break;
#endif
	case it_float:  return f(DC(typename D::Dfloat, a), b);break;
	case it_double: return f(DC(typename D::Ddouble, a), b);break;
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter_and_output: unsupported pixel type in image");
	}
}

/// \cond INTERNAL 

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
#ifdef LONG_64BIT
	case it_slong: return f(DC(typename D::Dsl,   a), b);
	case it_ulong: return f(DC(typename D::Dul,   a), b);
#endif
	case it_float: return f(DC(typename D::Dfloat,a), b);
	case it_double:return f(DC(typename D::Ddouble,a), b);
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}
/// \endcond


/**
   \ingroup filtering

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
#ifdef LONG_64BIT
	case it_slong: return _filter(f, a, DC(typename D::Dsl,    b));
	case it_ulong: return _filter(f, a, DC(typename D::Dul,    b));
#endif
	case it_float: return _filter(f, a, DC(typename D::Dfloat, b));
	case it_double:return _filter(f, a, DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}


/// \cond INTERNAL 
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
	case it_uint:  return f(DC(typename D::Dui,   a), b);
#ifdef LONG_64BIT
	case it_slong: return f(DC(typename D::Dsl,   a), b);
	case it_ulong: return f(DC(typename D::Dul,   a), b);
#endif
	case it_float: return f(DC(typename D::Dfloat,a), b);
	case it_double:return f(DC(typename D::Ddouble,a), b);
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::filter: unsupported pixel type in image");
	}
}
/// \endcond

/**
   \ingroup filtering

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
	case it_uint:  return _accumulate(f, a, DC(typename D::Dui,    b));
#ifdef LONG_64BIT
	case it_slong: return _accumulate(f, a, DC(typename D::Dsl,    b));
	case it_ulong: return _accumulate(f, a, DC(typename D::Dul,    b));
#endif
	case it_float: return _accumulate(f, a, DC(typename D::Dfloat, b));
	case it_double:return _accumulate(f, a, DC(typename D::Ddouble,b));
	default:
		assert(!"unsupported pixel type in image");
		throw std::invalid_argument("mia::accumulate: unsupported pixel type in image");
	}
}


#undef DC
#undef DV


template <class D>
TDataFilter<D>::~TDataFilter()
{
}

template <class D>
typename TDataFilter<D>::result_type
TDataFilter<D>::filter(const D& image) const
{
	return do_filter(image);
}

template <class D>
typename TDataFilter<D>::result_type
TDataFilter<D>::filter(std::shared_ptr<D> pimage) const
{
	return do_filter(pimage);
}

template <class D>
typename TDataFilter<D>::result_type
TDataFilter<D>::do_filter(std::shared_ptr<D> pimage) const
{
	return do_filter(*pimage); 
}

NS_MIA_END

#endif
