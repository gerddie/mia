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

#include <limits>
#include <stdexcept>
#include <boost/type_traits.hpp>


NS_MIA_BEGIN
	
template <typename T, bool is_integral>
struct FBinarize {
	FBinarize(float fmin, float fmax): 
		m_min(std::numeric_limits<T>::min()), 
		m_max(std::numeric_limits<T>::max()){
		if (fmin > m_min)
			m_min = static_cast<T>(fmin < std::numeric_limits<T>::max() ? fmin : std::numeric_limits<T>::max()); 
		
		if (fmax < m_max)
			m_max = static_cast<T>(fmax > std::numeric_limits<T>::min() ? fmax : std::numeric_limits<T>::min()); 
	}
	
	bool operator ()(T x)const {
		return !(x > m_max || x < m_min);
	}
private: 
	T m_min; 
	T m_max; 
}; 

template <typename T>
struct FBinarize<T, false> {
	FBinarize(float fmin, float fmax): 
		m_min(fmin), 
		m_max(fmax){
	}
	
	bool operator ()(T x)const {
		return !(x > m_max || x < m_min);  
	}
private: 
	T m_min; 
	T m_max; 
}; 

template <class Image>
template <template  <typename> class Data, typename T>
typename TBinarize<Image>::result_type TBinarize<Image>::operator () (const Data<T>& data) const
{
	const bool is_integral = ::boost::is_integral<T>::value; 
	
	Data<bool> *result = new Data<bool>(data.get_size(), data); 
	if (!result)
		throw create_exception<std::runtime_error>("binarize: unable to allocate image of size ", data.get_size()); 
	
	std::transform(data.begin(), data.end(), result->begin(), 
		  FBinarize<T, is_integral>(m_min, m_max)); 
	
	return result_type(result); 
}


/* The actual filter dispacth function calls the filter by selecting the right pixel type through wrap_filter */
template <class Image>
typename TBinarize<Image>::result_type TBinarize<Image>::do_filter(const Image& image) const
{
	return mia::filter(*this, image); 
}


template <class Image>
TBinarizeImageFilterFactory<Image>::TBinarizeImageFilterFactory():
	TImageFilterPlugin<Image>("binarize"), 
	m_min(0), 
	m_max(std::numeric_limits<float>::max())
{
	this->add_parameter("min", new CFloatParameter(m_min, 0, std::numeric_limits<float>::max(), 
						 false, "minimum of accepted range")); 
	this->add_parameter("max", new CFloatParameter(m_max, 0, std::numeric_limits<float>::max(), 
						 false, "maximum of accepted range")); 

}

template <class Image> 
TImageFilter<Image> *TBinarizeImageFilterFactory<Image>::do_create()const
{
	return new TBinarize<Image>(m_min, m_max); 
}

template <class Image> 
const std::string TBinarizeImageFilterFactory<Image>::do_get_descr()const
{
	return "image binarize filter"; 
}

NS_MIA_END
