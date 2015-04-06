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

#ifndef mia_2d_imagetest_hh
#define mia_2d_imagetest_hh

#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup test 
   \brief test if two images are equal and report the results by using BOOST_TEST methods. 

   Test if two images are ove equal size, equal pixel type and if their 
   pixel values are equal if the pixel type is integral, or close 
   if the pixel type is floating point. 
   The backend uses the BOOST unit test library to report sucess or failture. 

   @param A test image 
   @param expect the expected image 
   
 */
void EXPORT_2D test_image_equal(const C2DImage& A, const C2DImage& expect);

NS_MIA_END

#endif
