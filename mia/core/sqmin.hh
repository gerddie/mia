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


#ifndef mia_core_sqmin_hh
#define mia_core_sqmin_hh

#include <mia/core/defines.hh>


NS_MIA_BEGIN

/**
   \ingroup helpers 
   given three points (0.0, y0), (1.0, y,1), (x, y2) evaluate
   \f$ r = arg(min (ax^2 + bx + c)) \forall r \in [min(0.0, x), max(1.0, x)] \f$
 */
EXPORT_CORE double  min_ax2_bx_c(double x2, double y0, double y1, double y2);

NS_MIA_END


#endif
