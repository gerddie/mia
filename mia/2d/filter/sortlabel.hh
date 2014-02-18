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

NS_BEGIN(sort_label_2dimage_filter)

class CSortLabel: public mia::C2DFilter {
public:
	template <typename T> 
	mia::P2DImage operator() (const mia::T2DImage<T>& image) const; 
private:
	CSortLabel::result_type do_filter(const mia::C2DImage& image) const;
};

class CSortLabelFilterPlugin: public mia::C2DFilterPlugin {
public:
	CSortLabelFilterPlugin();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
};

NS_END
