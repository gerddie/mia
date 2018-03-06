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

#ifndef __mia_3d_filter_mask_hh
#define __mia_3d_filter_mask_hh

#include <mia/core/delayedparameter.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>


NS_BEGIN(mask_3dimage_filter)

class C3DMask: public mia::C3DFilter
{
public:
       C3DMask(const mia::C3DImageDataKey& mask_image);

       template <typename T>
       C3DMask::result_type operator () (const mia::T3DImage<T>& data) const;
private:
       virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;

       mia::C3DImageDataKey m_image_key;
};

/* The factory class - this is what the application gets first. This factory class is used to
   create the actual filter object. It also provides some filter testing routines.
*/
class C3DMaskImageFilterFactory: public mia::C3DFilterPlugin
{
public:
       C3DMaskImageFilterFactory();
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string do_get_descr()const;
private:
       std::string m_mask_filename;
};

NS_END
#endif
