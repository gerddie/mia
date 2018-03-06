/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

NS_BEGIN(thinning_3dimage_filter)

class C3DThinning: public mia::C3DFilter
{
public:
       C3DThinning();

private:
       mia::C3DFilter::result_type do_filter(const mia::C3DImage& image) const;
};


class C3DThinningFilterPlugin: public mia::C3DFilterPlugin
{
public:
       C3DThinningFilterPlugin();
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string do_get_descr()const;
};

NS_END


