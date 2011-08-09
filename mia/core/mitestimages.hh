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
extern EXPORT_CORE const int moving_init_data[]; 
extern EXPORT_CORE const int reverence_init_data[]; 

NS_MIA_END

#endif
