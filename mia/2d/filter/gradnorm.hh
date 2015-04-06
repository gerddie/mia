/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
NS_BEGIN(gradnorm_2dimage_filter)

class CGradnorm: public mia::C2DFilter {
public:
	CGradnorm(bool normalize);

	template <typename  T>
	CGradnorm::result_type operator () (const mia::T2DImage<T>& data) const;

private:
	CGradnorm::result_type do_filter(const mia::C2DImage& image) const;
	bool m_normalize; 
};


class C2DGradnormFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DGradnormFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	bool m_normalize; 
};

NS_END
