/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef __mia_internal_binarize_hh
#define __mia_internal_binarize_hh


#include <mia/core/filter.hh>

NS_MIA_BEGIN

template <class Image>	
class TBinarize: public TImageFilter<Image> {
	float m_min; 
	float m_max; 
public:
	typedef typename TImageFilter<Image>::result_type result_type; 
	
	TBinarize(float min, float max):
		m_min(min), 
		m_max(max)
	{
	}
	

	template <template  <typename> class Data, typename T>
	typename TBinarize<Image>::result_type operator () (const Data<T>& data) const ;
private: 
	typename TBinarize<Image>::result_type do_filter(const Image& image) const;
};

template <class Image>	
class TBinarizeImageFilterFactory: public TImageFilterPlugin<Image> {
public: 
	TBinarizeImageFilterFactory();
	virtual TImageFilter<Image> *do_create()const;
	virtual const std::string do_get_descr()const; 
private: 
	float m_min; 
	float m_max; 

};
	
NS_MIA_END

#endif
