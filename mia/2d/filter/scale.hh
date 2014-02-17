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

#include <memory>
#include <mia/core/filter.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/interpolator.hh>

NS_BEGIN(scale_2dimage_filter)


class CScale: public mia::C2DFilter {
public:
	CScale(const mia::C2DBounds& size, mia::PSplineKernel kernel);

	~CScale();

	template <typename  T>
	mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& data) const;

private:
	CScale::result_type do_filter(const mia::C2DImage& image) const;

	const mia::C2DBounds m_size;
	mia::PSplineKernel m_kernel;
};



class C2DScaleFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DScaleFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::C2DBounds m_s; 
	mia::PSplineKernel m_interp;
};

NS_END
