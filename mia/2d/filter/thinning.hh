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
#include <mia/2d/morphshape.hh>

NS_BEGIN(thinning_2dimage_filter) 

class C2DThinningImageFilter: public mia::C2DFilter {
public:
	C2DThinningImageFilter(int max_iterations);

	template <typename T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& result)const;

private:

	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	std::vector<mia::C2DMorphShape> m_shape;
	int m_max_iterations; 
};

class C2DThinningFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DThinningFilterFactory();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	int m_max_iterations; 
};


NS_END


