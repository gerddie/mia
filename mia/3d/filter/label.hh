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

#ifndef mia_3d_filter_label_hh
#define mia_3d_filter_label_hh

#include <mia/3d/filter.hh>
#include <mia/3d/shape.hh>


NS_BEGIN(label_3dimage_filter)

class CLabel: public mia::C3DFilter
{
public:
       CLabel(mia::P3DShape m_mask);

private:
       void grow_region(const mia::C3DBounds& loc, const mia::C3DBitImage& input,
                        mia::C3DUSImage& result, unsigned short label)const;
       CLabel::result_type do_filter(const mia::C3DImage& image) const;
       mia::P3DShape m_mask;
};

class C3DLabelFilterPlugin: public mia::C3DFilterPlugin
{
public:
       C3DLabelFilterPlugin();
private:
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string do_get_descr()const;
       mia::P3DShape m_mask;
};

NS_END

#endif
