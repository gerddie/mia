/* -*- mia-c++  -*-
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
		_M_min(numeric_limits<T>::min()), 
		_M_max(numeric_limits<T>::max()){
		if (fmin > _M_min)
			_M_min = static_cast<T>(fmin < numeric_limits<T>::max() ? fmin : numeric_limits<T>::max()); 
		
		if (fmax < _M_max)
			_M_max = static_cast<T>(fmax > numeric_limits<T>::min() ? fmax : numeric_limits<T>::min()); 
	}
	
	bool operator ()(T x)const {
		return !(x > _M_max || x < _M_min);
	}
private: 
	T _M_min; 
	T _M_max; 
}; 

template <typename T>
struct FBinarize<T, false> {
	FBinarize(float fmin, float fmax): 
		_M_min(fmin), 
		_M_max(fmax){
	}
	
	bool operator ()(T x)const {
		return !(x > _M_max || x < _M_min);  
	}
private: 
	T _M_min; 
	T _M_max; 
}; 

template <class Image>
template <template  <typename> class Data, typename T>
typename TBinarize<Image>::result_type TBinarize<Image>::operator () (const Data<T>& data) const
{
	const bool is_integral = ::boost::is_integral<T>::value; 
	
	Data<bool> *result = new Data<bool>(data.get_size()); 
	if (!result) {
		stringstream err; 
		err << "binarize: unable to allocate image of size " << data.get_size().x << "x" << data.get_size().y; 
		throw runtime_error(err.str()); 
	}
	
	transform(data.begin(), data.end(), result->begin(), 
		  FBinarize<T, is_integral>(_M_min, _M_max)); 
	
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
	_M_min(0), 
	_M_max(std::numeric_limits<float>::max())
{
	this->add_parameter("min", new CFloatParameter(_M_min, 0, numeric_limits<float>::max(), 
						 false, "minimum of the band")); 
	this->add_parameter("max", new CFloatParameter(_M_max, 0, numeric_limits<float>::max(), 
						 false, "maximum of the band")); 

}

template <class Image> 
typename TImageFilterPlugin<Image>::ProductPtr TBinarizeImageFilterFactory<Image>::do_create()const
{
	return typename TImageFilterPlugin<Image>::ProductPtr(new TBinarize<Image>(_M_min, _M_max)); 
}

template <class Image> 
const std::string TBinarizeImageFilterFactory<Image>::do_get_descr()const
{
	return "image binarize filter"; 
}

NS_MIA_END
