/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_core_meanvar_hh
#define mia_core_meanvar_hh

#include <map>
#include <cmath>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   Evaluate the mean and the variance of a series of scalars 
   \tparam ForwardIterator 
   \param begin 
   \param end 
   \returns mean as .first and variance as .second of the std::pair. 
   \todo could be extended to vector types 
 */

template <typename ForwardIterator>
std::pair<double, double> mean_var(ForwardIterator begin, ForwardIterator end) 
{
	std::pair<double, double> result; 
	result.first  = 0.0; 
	result.second = 0.0; 
	size_t n = 0; 
	
	while (begin != end)  {
		const double help = *begin; 
		result.first += help; 
		result.second += help * help; 
		++n; 
		++begin; 
	}

	if (n > 0)
		result.first /= n; 

	if (n > 1) 
		result.second = sqrt((result.second - n * result.first * result.first) / (n - 1));
	else 
		result.second = 0.0; 
	return result; 
}

NS_MIA_END

#endif
