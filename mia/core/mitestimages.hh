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

#ifndef mi_test_images_h
#define mi_test_images_h

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup test 
   Simple structure for test images 
 */

struct SImageSize {
	/// image width 
	int width; 
	/// image height 
	int height; 
}; 

extern EXPORT_CORE const SImageSize mi_test_size; 
extern EXPORT_CORE const float moving_init_data[]; 
extern EXPORT_CORE const float reference_init_data[]; 

NS_MIA_END

#endif
