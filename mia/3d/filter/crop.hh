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

#include <mia/3d/filter.hh>

NS_BEGIN(crop_3d_filter)

class C3DCrop: public mia::C3DFilter {
public:
	C3DCrop(const mia::C3DBounds& begin, const mia::C3DBounds& end);

	template <typename T>
	C3DCrop::result_type operator () (const mia::T3DImage<T>& data) const;
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	const mia::C3DBounds m_begin;
	const mia::C3DBounds m_end;
};

/* The factory class - this is what the application gets first. This factory class is used to
   create the actual filter object. It also provides some filter testing routines.
*/
class C3DCropImageFilterFactory: public mia::C3DFilterPlugin {
public:
	C3DCropImageFilterFactory();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::C3DBounds m_begin;
	mia::C3DBounds m_end;
};

NS_END
