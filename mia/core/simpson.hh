/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   Simpson integration of a 1D-function 
   \tparam F a functor that defines the double operator ()(double x). 
   \param from begin of integration interval 
   \param to end of integration interval 
   \param intervals number of intervals to use for integration 
   \param function funtion to integrate 
   \returns value of integral 
 */
template <class F>
double simpson(double from, double to, size_t intervals, const F& function)
{
	double sum = 0.0; 
	double dx = (to - from) / intervals; 
	
	sum = 0.5 * (function(from) + function(to)); 
	double x = from + dx; 
	for (size_t ix = 1; ix < intervals; ++ix, x+=dx) 
		sum += function(x); 

	x = from + .5 * dx; 
	for (size_t ix = 0; ix < intervals; ++ix, x+=dx) 
		sum += 2 * function(x); 
	
	return sum * dx / 3.0; 
}

NS_MIA_END
