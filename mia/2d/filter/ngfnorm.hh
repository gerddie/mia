/* -*- mia-c++  -*-
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

#include <mia/core/filter.hh>
#include <mia/2d/2dfilter.hh>

NS_BEGIN(ngfnorm_2dimage_filter)

class CNgfnorm: public mia::C2DFilter {
public:
	CNgfnorm();

	template <typename  T>
	mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& data) const;

private:
	mia::C2DFilter::result_type do_filter(const mia::C2DImage& image) const;
};


class C2DNgfnormFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DNgfnormFilterPlugin();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
};

NS_END
