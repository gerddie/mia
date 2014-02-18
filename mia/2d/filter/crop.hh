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

#include <mia/2d/filter.hh>
#include <stdexcept>

NS_BEGIN(crop_2dimage_filter)




class C2DCrop: public mia::C2DFilter {
public:
	typedef mia::T2DVector<int> C2DSize;
	C2DCrop(const C2DSize & start, const C2DSize& size):
		m_start(start),
		m_size(size)
	{
	}

	template <class Data2D>
	typename C2DCrop::result_type operator () (const Data2D& data) const;

private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;
	C2DSize m_start;
	C2DSize m_size;

};

class C2DCropImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DCropImageFilterFactory();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	C2DCrop::C2DSize m_start;
	C2DCrop::C2DSize m_end;
};

NS_END
