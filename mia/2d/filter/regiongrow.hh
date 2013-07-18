/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/filter.hh>
#include <mia/2d/shape.hh>
#include <mia/2d/imageio.hh>

NS_BEGIN(regiongrow_2d_filter) 

class C2DRegiongrowFilter: public mia::C2DFilter {
public:
	C2DRegiongrowFilter(const mia::C2DImageDataKey& mask_image, mia::P2DShape m_neighborhood);

	template <typename  T>
	C2DRegiongrowFilter::result_type operator () (const mia::T2DImage<T>& data) const;

private:
	C2DRegiongrowFilter::result_type do_filter(const mia::C2DImage& image) const;

	typedef mia::C2DShape::value_type MPosition; 
	typedef mia::C2DBounds Position; 
	
	std::vector<MPosition> m_neighborhood; 
	mia::C2DImageDataKey m_seed_image_key; 

};

class C2DRegiongrowFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DRegiongrowFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::P2DShape m_neighborhood;
	std::string m_seed_image;
};


NS_END
