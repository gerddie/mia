/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Biomedical Image Technologies, Universidad Politecnica de Madrid
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

#ifndef mia_3d_filter_median_hh
#define mia_3d_filter_median_hh

#include <mia/3d/3dfilter.hh>

NS_BEGIN(median_3dimage_filter)

class C3DMedianFilter: public mia::C3DFilter {
	int m_width;
public:
	C3DMedianFilter(int hwidth);

	template <class T>
	mia::P3DImage operator () (const mia::T3DImage<T>& data) const ;
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
};


class C3DSaltAndPepperFilter: public mia::C3DFilter {
	int m_width;
	float m_thresh;
public:
	C3DSaltAndPepperFilter(int hwidth, float thresh);

	template <class T>
	mia::P3DImage operator () (const mia::T3DImage<T>& data) const ;

private:

	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
};

class C3DMedianFilterFactory: public mia::C3DFilterPlugin {
public:
	C3DMedianFilterFactory();
private:
	virtual mia::C3DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string  do_get_descr() const;
	virtual bool do_test() const;
	int m_hw;
};



class C3DSaltAndPepperFilterFactory: public mia::C3DFilterPlugin {
public:
	C3DSaltAndPepperFilterFactory();
private:
	virtual mia::C3DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string  do_get_descr() const;
	virtual bool do_test() const;
	int m_hw;
	float m_thresh;
};


NS_END

#endif
