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

#include <mia/3d/filter.hh>
NS_BEGIN(gradnorm_3dimage_filter)

class CGradnorm: public mia::C3DFilter {
public:
	CGradnorm();

	template <typename  T>
	CGradnorm::result_type operator () (const mia::T3DImage<T>& data) const;

private:
	CGradnorm::result_type do_filter(const mia::C3DImage& image) const;
};


class C3DGradnormFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DGradnormFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
};

NS_END
