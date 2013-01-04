/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 David Paster, Gert Wollny
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

#ifndef mia_2d_trackpoint_hh
#define mia_2d_trackpoint_hh

#include <mia/2d/transform.hh>
#include <mia/template/trackpoint.hh>

NS_MIA_BEGIN



/**
   \brief Class to track pixel movement based on a transformation

   This class holds the information to track a single pixel in 2D space 
   over time and with a given transformation.  
*/

typedef TTrackPoint<C2DTransformation>  C2DTrackPoint; 


NS_MIA_END

#endif 
