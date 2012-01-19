/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cmath>

#include <mia/core/fullstats.hh>

NS_MIA_BEGIN
using namespace std;

void CFullStats::evaluate(Vector& tmp)
{
	const size_t n = tmp.size();

	if (n < 2)
		throw invalid_argument("CFullStats: require at least two values");


	m_mean /= n;
	m_sigma = sqrt((m_sigma - m_mean * m_mean  * n) / (n - 1));

	if (n & 1) {
		Vector::iterator i = tmp.begin() + (n - 1) / 2;
		std::nth_element(tmp.begin(), i, tmp.end());
		m_median = *i;
	}else {
		Vector::iterator i1 = tmp.begin() + n / 2 - 1;
		Vector::iterator i2 = tmp.begin() + n / 2;
		std::nth_element(tmp.begin(), i1, tmp.end());
		std::nth_element(tmp.begin(), i2, tmp.end());
		m_median = (*i1 + *i2) / 2.0;
	}
}

double CFullStats::mean()const
{
	return m_mean;
}

double CFullStats::sigma()const
{
	return m_sigma;
}

double CFullStats::median()const
{
	return m_median;
}

double CFullStats::max()const
{
	return m_max;
}

double CFullStats::min()const
{
	return m_min;
}


void CFullStats::print(std::ostream& os) const
{
	os << m_mean << " "
	   << m_sigma << " "
	   << m_median << " "
	   << m_min << " "
	   << m_max;
}


NS_MIA_END
