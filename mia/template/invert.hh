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

#ifndef __mia_internal_invert_hh
#define __mia_internal_invert_hh

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN

///  @cond DOC_PLUGINS 
template <class Image> 
class TInvert: public TDataFilter<Image> {
public: 	
	typedef typename TDataFilter<Image>::result_type result_type; 
	TInvert(); 

	template <class  T>
	typename TInvert::result_type operator () (const T& data) const;
private: 
	typename TInvert::result_type do_filter(const Image& image) const;
};

template <class Image> 
class TInvertFilterPlugin: public TDataFilterPlugin<Image>  {
public: 
	TInvertFilterPlugin();
	virtual TDataFilter<Image> *do_create()const;
	virtual const std::string do_get_descr()const; 
};

///  @endcond 
NS_MIA_END


#endif
