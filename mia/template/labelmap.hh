/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_template_labelmap_hh
#define mia_template_labelmap_hh

#include <mia/core/filter.hh>
#include <mia/core/labelmap.hh>
#include <map>

NS_MIA_BEGIN

template <class Image>       
class TLabelMapFilter: public TDataFilter<Image> {
	std::map<size_t, size_t>  m_map;
public:

	typedef typename TDataFilter<Image>::result_type result_type; 
	
	TLabelMapFilter(const CLabelMap& lmap); 
	
	template <class TImage>
	typename TLabelMapFilter<Image>::result_type operator () (const TImage& data) const ;

private: 
	virtual typename TLabelMapFilter<Image>::result_type do_filter(const Image& image) const;

};

template <class Image> 
class TLabelMapFilterPlugin: public TDataFilterPlugin<Image> {
public: 
	TLabelMapFilterPlugin();
	virtual TDataFilter<Image> *do_create()const; 
	virtual const std::string do_get_descr()const; 

private: 
	std::string m_map; 
};

NS_END

#endif 
