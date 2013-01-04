/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/filter.hh>
#include <mia/core/labelmap.hh>

NS_BEGIN(labelmap_2dimage_filter)
      
class C2DLabelMapFilter: public mia::C2DFilter {
	std::vector<size_t>  m_map;
public:
	C2DLabelMapFilter(const mia::CLabelMap& lmap); 
	
	template <class Data2D>
	typename C2DLabelMapFilter::result_type operator () (const Data2D& data) const ;

private: 
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

};

class C2DLabelMapFilterPlugin: public mia::C2DFilterPlugin {
public: 
	C2DLabelMapFilterPlugin();
	virtual mia::C2DFilter *do_create()const; 
	virtual const std::string do_get_descr()const; 

private: 
	std::string m_map; 
};

NS_END
