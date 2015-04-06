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

#ifndef mia_3d_filter_lvdownscale_hh
#define mia_3d_filter_lvdownscale_hh

#include <mia/3d/filter.hh>

NS_BEGIN(lvdownscale_3dimage_filter)

class C3DLVDownscale: public mia::C3DFilter {
public:
	C3DLVDownscale(const mia::C3DBounds& block_size);

	template <typename  T>
	mia::C3DFilter::result_type operator () (const mia::T3DImage<T>& data) const;

private:
	mia::C3DFilter::result_type do_filter(const mia::C3DImage& image) const;

	const mia::C3DBounds m_block_size;


};

class C3DLVDownscaleFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DLVDownscaleFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::C3DBounds m_b;
};

NS_END

#endif
