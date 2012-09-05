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

#include <boost/algorithm/minmax_element.hpp>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

template <class Image> 
TInvert<Image>::TInvert()
{
}

template <class Image> 
template <class Data>
typename TInvert<Image>::result_type TInvert<Image>::operator () (const Data& data) const
{
	TRACE("TInvert::operator()"); 
	// maintain range 

	auto  ib = data.begin(); 
	auto  ie = data.end(); 

	auto src_minmax = ::boost::minmax_element(ib, ie); 
	
	Data *result = new Data(data.get_size(), data); 

	std::transform(ib, ie, result->begin(), [src_minmax](typename Data::value_type x){
			return *src_minmax.second - x + *src_minmax.first;}); 
	
	return typename TInvert::result_type(result);
}

template <class Image> 
typename TInvert<Image>::result_type TInvert<Image>::do_filter(const Image& image) const
{
	return mia::filter(*this, image); 
}


template <class Image> 
TInvertFilterPlugin<Image>::TInvertFilterPlugin():
	TImageFilterPlugin<Image>("invert")
{
}

template <class Image> 
TImageFilter<Image> *TInvertFilterPlugin<Image>::do_create()const
{
	return new TInvert<Image>(); 
}

template <class Image> 
const std::string TInvertFilterPlugin<Image>::do_get_descr()const
{
	return "intensity invert filter"; 
}

NS_MIA_END
