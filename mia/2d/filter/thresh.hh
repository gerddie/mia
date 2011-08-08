/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/shape.hh>


NS_BEGIN(thresh_2dimage_filter)

class C2DThreshNImageFilter: public mia::C2DFilter {
public:
	C2DThreshNImageFilter(mia::P2DShape shape, double thresh);

	template <typename T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& result)const;

private:

	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	mia::P2DShape m_shape;
	double m_thresh;
};

class C2DThreshNImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DThreshNImageFilterFactory();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr() const;
	std::string m_shape;
	double m_thresh;
};

NS_END
