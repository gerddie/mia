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

#include <mia/core/dictmap.hh>
#include <mia/3d/filter.hh>


// start a namespace to avoid naming conflicts during runtime

NS_BEGIN(reorient_3dimage_filter)

// the name of the plugin - this is, how it will be called from the command line


/*Some filter specific parameters are defined here:
  the binarize filter converts an input image to a binary image. A pixel becomes 1
  if corresponding pixel in the the input image is in the range [min,max], 0 otherwise
*/


/* This is the work-horse class of the filter plugin. It is initialised with above parameters
   and provides a templated operator () to be able to handle all pixel types.
 */
class C3DReorient: public mia::C3DFilter
{
public:
       enum EOrientations {xyz,
                           yzx, // permutate axes
                           zxy, // permutate axes
                           flip_yz,
                           flip_xy,
                           flip_xz,
                           rotate_x90,
                           rotate_x180,
                           rotate_x270,
                           rotate_y90,
                           rotate_y180,
                           rotate_y270,
                           rotate_z90,
                           rotate_z180,
                           rotate_z270,
                           ooo
                          };

       C3DReorient(EOrientations strategy);

       template <typename T>
       C3DReorient::result_type operator () (const mia::T3DImage<T>& data) const;
private:
       virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
       mia::E3DImageOrientation get_new_orientation(EOrientations strategy, mia::E3DImageOrientation old_orientation)const;
       EOrientations m_strategy;
};



/* The factory class - this is what the application gets first. This factory class is used to
   create the actual filter object. It also provides some filter testing routines.
*/
class C3DReorientImageFilterFactory: public mia::C3DFilterPlugin
{
public:
       C3DReorientImageFilterFactory();
       virtual mia::C3DFilter *do_create()const;
       virtual const std::string do_get_descr()const;

private:
       static const mia::TDictMap<C3DReorient::EOrientations>::Table table[];
       static const mia::TDictMap<C3DReorient::EOrientations> map;
       C3DReorient::EOrientations m_orientation;
};

NS_END
