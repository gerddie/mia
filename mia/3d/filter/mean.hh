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

#ifndef mia_3d_filter_mean_hh
#define mia_3d_filter_mean_hh

#include <mia/3d/filter.hh>

NS_BEGIN(mean_3dimage_filter)

class C3DMeanFilter: public mia::C3DFilter {
public:
	C3DMeanFilter(int hwidth);

	template <class T>
	mia::T3DImage<T> *apply(const mia::T3DImage<T>& data) const ;


	template <class T>
	mia::P3DImage operator () (const mia::T3DImage<T>& data) const ;
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	int m_hwidth;
};



class C3DMeanFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DMeanFilterPlugin();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string  do_get_descr() const;
	int m_hw;
};


class C3DVarianceFilter: public mia::C3DFilter {
public:
	C3DVarianceFilter(int hwidth);

	template <class T>
	mia::P3DImage operator () (const mia::T3DImage<T>& data) const;

	mia::P3DImage operator () (const mia::C3DBitImage& data) const;

private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	int m_hwidth;
        C3DMeanFilter m_mean; 
};



class C3DVarianceFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DVarianceFilterPlugin();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string  do_get_descr() const;
	int m_hw;
};




NS_END

#endif
