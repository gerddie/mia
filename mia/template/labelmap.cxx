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

#include <stdexcept>
#include <istream>
#include <fstream>
#include <mia/template/labelmap.hh>

NS_MIA_BEGIN;

template <class Image>
TLabelMapFilter<Image>::TLabelMapFilter(const CLabelMap& lmap)
{

	for (auto i = lmap.begin(), e = lmap.end(); 
	     i != e; ++i)
		m_map[i->first] = i->second; 
}


template <class Image>
typename TLabelMapFilter<Image>::result_type TLabelMapFilter<Image>::do_filter(const Image& image) const
{
	return mia::filter(*this, image); 
}

template <typename Image, bool is_integer> 
struct dispatch_label_map {
	typedef typename Image::value_type value_type; 
	typedef std::map<size_t, size_t> LabelMap; 

	static 
	typename TLabelMapFilter<Image>::result_type 
	apply(const Image& /*image*/, const LabelMap& /*lmap*/){
		throw std::invalid_argument("Labelmap: Floating pount values are nor supported");
	}
};

template <typename Image> 
struct dispatch_label_map<Image, true> {
	typedef  typename Image::value_type value_type; 
	typedef std::map<size_t, size_t> LabelMap; 
	typedef typename TLabelMapFilter<Image>::result_type result_type; 

	static result_type apply(const Image& image, const LabelMap& lmap){
		Image *result = new Image(image.get_size(), image); 
		transform(image.begin(), image.end(), result->begin(), 
			  [&lmap](value_type x){ 
				  auto iy = lmap.find(x); 
				  return iy != lmap.end() ? static_cast<value_type>(iy->second) : x;
			  });
		return result_type(result); 
	}
};

template <class Image>
template <class TImage>
typename TLabelMapFilter<Image>::result_type TLabelMapFilter<Image>::operator () (const TImage& data) const
{
	return dispatch_label_map<TImage, std::numeric_limits<typename TImage::value_type>::is_integer>::apply(data, m_map); 
	
}

template <class Image>
TLabelMapFilterPlugin<Image>::TLabelMapFilterPlugin():
	TDataFilterPlugin<Image>("labelmap")
{
	this->add_parameter("map", new CStringParameter(m_map, CCmdOptionFlags::required_input, "Label mapping file")) ;
}


template <class Image>
TDataFilter<Image> *TLabelMapFilterPlugin<Image>::do_create() const
{
	std::ifstream is(m_map.c_str()); 

	if (!is.good())
		throw create_exception<std::runtime_error>("C2DLabelMapFilterPlugin::load_label_map:"
					    "Unable to open file '", m_map, "'"); 
	
	CLabelMap map(is);
	return new TLabelMapFilter<Image>(map); 
}

template <class Image>
const std::string TLabelMapFilterPlugin<Image>::do_get_descr()const
{
	return "Image filter to remap label id's. Only applicable to "
		"images with integer valued intensities/labels."; 
}


NS_END
