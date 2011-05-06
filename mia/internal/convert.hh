/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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


#ifndef __mia_internal_convert_hh
#define __mia_internal_convert_hh

#include <boost/type_traits.hpp>
#include <mia/core/filter.hh>


NS_MIA_BEGIN

/*
  This class implements the image pixel type conversion. 
  Various mappings are supported and defined as  EPixelConversion. 
  \tparam Image the image type that must provide STL-like iterators. 
 */
template <class Image>	
class TConvert: public TImageFilter<Image> {
public: 	
	typedef typename TImageFilter<Image>::result_type result_type; 
	
	/**
	   Create the conversion filter with the parameters
	   @param pt output pixel type
	   @param ct conversion mapping to be applied 
	   @param a scale factor for linear mapping 
	   @param b shift factor for linear mapping 
	 */
	TConvert(mia::EPixelType pt, EPixelConversion ct, float a, float b); 

	/**
	   \tparam container type must support STL like forward iterator
	   \tparam pixel type, must be a scalar data type 
	   \param data the data to be converted 
	   \returns the container with the converted data 
	   \remark review the template parameters, there seem to be more then necessary  
	 */
	template <template  <typename> class Data, typename T>
	typename TConvert<Image>::result_type operator () (const Data<T>& data) const; 
	
private: 
	template <template  <typename> class Data, typename  S, typename  T>
	typename TConvert<Image>::result_type convert(const Data<S>& src) const; 
	typename TConvert::result_type do_filter(const Image& image) const;
	
	EPixelType m_pt; 
	EPixelConversion m_ct; 
	float m_a; 
	float m_b; 
};

template <class Image>	
class TConvertFilterPlugin: public TImageFilterPlugin<Image> {
public: 
	TConvertFilterPlugin();
private: 
	virtual typename TImageFilterPlugin<Image>::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const; 
	
	std::string m_pixeltype; 
	std::string m_convert; 
	float m_a; 
	float m_b; 

};


template <typename  T, bool is_float>
struct mia_round {
	static T apply(T x) {
		return x; 
	}
};

template <typename  T>
struct mia_round<T, false> {
	static T apply(long double x) {
		return static_cast<T>(floor(x + 0.5));
	}
};

template <typename T, bool is_float> 
struct __dispatch_minmax {
	static std::pair<T, T> apply() {
		return std::pair<T, T>(std::numeric_limits<T>::min(),std::numeric_limits<T>::max()); 
	}
}; 

template <typename T> 
struct __dispatch_minmax<T, true> {
	static std::pair<T, T> apply() {
		return std::pair<T, T>(-1.0f, 1.0f);
	}
};

template <typename T> 
struct get_minmax {
	static std::pair<T, T> apply() {
		return __dispatch_minmax<T, boost::is_floating_point<T>::value >::apply(); 
	}
};

NS_MIA_END

#endif
