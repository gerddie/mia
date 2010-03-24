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

#ifndef __mia_internal_invert_hh
#define __mia_internal_invert_hh

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN

template <class Image> 
class TInvert: public TImageFilter<Image> {
public: 	
	typedef typename TImageFilter<Image>::result_type result_type; 
	TInvert(); 

	template <class  T>
	typename TInvert::result_type operator () (const T& data) const;
private: 
	typename TInvert::result_type do_filter(const Image& image) const;
};

template <class Image> 
class TInvertFilterPlugin: public TImageFilterPlugin<Image>  {
public: 
	TInvertFilterPlugin();
	virtual typename TImageFilterPlugin<Image>::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const; 
};

NS_MIA_END


#endif
