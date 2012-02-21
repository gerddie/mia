/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/2d/2dfilter.hh>

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
