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

#ifndef mia_3d_filter_median_hh
#define mia_3d_filter_median_hh

#include <mia/3d/filter.hh>

NS_BEGIN(median_3dimage_filter)

class C3DMedianFilter: public mia::C3DFilter
{
       int m_width;
public:
       C3DMedianFilter(int hwidth);

       template <class T>
       mia::P3DImage operator () (const mia::T3DImage<T>& data) const ;
private:
       virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
};


class C3DSaltAndPepperFilter: public mia::C3DFilter
{
       int m_width;
       float m_thresh;
public:
       C3DSaltAndPepperFilter(int hwidth, float thresh);

       template <class T>
       mia::P3DImage operator () (const mia::T3DImage<T>& data) const ;

private:

       virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
};

class C3DMedianFilterFactory: public mia::C3DFilterPlugin
{
public:
       C3DMedianFilterFactory();
private:
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string  do_get_descr() const;
       int m_hw;
};



class C3DSaltAndPepperFilterFactory: public mia::C3DFilterPlugin
{
public:
       C3DSaltAndPepperFilterFactory();
private:
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string  do_get_descr() const;
       int m_hw;
       float m_thresh;
};


NS_END

#endif
