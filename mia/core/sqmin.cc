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

#include <cassert>
#include <cmath>
#include <mia/core/sqmin.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN

// float x0 = 0.0, float x1 = 1.0,
EXPORT_CORE  double min_ax2_bx_c(double x, double y0, double y1, double y2)
{
	assert (x != 0.0 && x != 1.0);
	const double d10 = y1 - y0;
	const double d20 = y2 - y0;
	const double x2 = x * x;
	const double b = (d10 * x2 - d20) / (x2 - x);
	const double a = 2.0 * (b - d10);

	if ( ( ((x > 1.0 ) ? x : 1.0)  * fabs(a) )  <   fabs(b) ) {
		if ( b > 0 )
			return 0.0;
		else
			return (x > 1.0 ) ? x : 1.0;
	}




	if (a * b < 0.0)
		return 0.0;

	return b /  a;
}

NS_MIA_END
