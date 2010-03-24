/* -*- mia-c++  -*-
 * Copyright (c) 2007-2009 Gert Wollny 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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



NS_MIA_BEGIN

template <class Image> 
TBandPass<Image>::TBandPass(float min, float max):
	_M_min(min), 
	_M_max(max)
{
}


template <class Image> 
template <class Data>
typename TBandPass<Image>::result_type TBandPass<Image>::operator () (const Data& data) const
{
	TRACE("TBandPass::operator()"); 

	typename Data::const_iterator ib = data.begin(); 
	typename Data::const_iterator ie = data.end(); 
	Data *result = new Data(data.get_size()); 
	typename Data::iterator r = result->begin(); 
	fill(result->begin(), result->end(), 0); 
	
	while (ib != ie) {
		if (! (*ib < _M_min) && ! (*ib > _M_max))
			*r = *ib; 
		++r; 
		++ib; 
	}

	return typename TBandPass::result_type(result);
}

template <class Image> 
typename TBandPass<Image>::result_type TBandPass<Image>::do_filter(const Image& image) const
{
	return mia::filter(*this, image); 
}


template <class Image> 
TBandPassFilterPlugin<Image>::TBandPassFilterPlugin():
	TImageFilterPlugin<Image>("bandpass"), 
	_M_min(0), 
	_M_max(numeric_limits<float>::max())
{
	this->add_parameter("min", new CFloatParameter(_M_min, 0, numeric_limits<float>::max(), false, "minimum of the band")); 
	this->add_parameter("max", new CFloatParameter(_M_max, 0, numeric_limits<float>::max(), false, "maximum of the band")); 
}

template <class Image> 
typename TImageFilterPlugin<Image>::ProductPtr TBandPassFilterPlugin<Image>::do_create()const
{
	return typename TImageFilterPlugin<Image>::ProductPtr(new TBandPass<Image>(_M_min, _M_max)); 
}

template <class Image> 
const std::string TBandPassFilterPlugin<Image>::do_get_descr()const
{
	return "intensity bandpass filter"; 
}

NS_MIA_END
