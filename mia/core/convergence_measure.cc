/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Genoa 1999-2016 Gert Wollny
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

#include <mia/core/convergence_measure.hh>
#include <mia/core/msgstream.hh>
#include <limits>
#include <numeric>
#include <cmath>

using std::numeric_limits; 

namespace mia {

CConvergenceMeasure::CConvergenceMeasure(uint32_t size):
        m_size(size)
{
        
}

void CConvergenceMeasure::push(double value)
{
        m_v.push_back(value);

        if (m_v.size() > m_size) {
                m_v.pop_front();
        }
}

double CConvergenceMeasure::value() const
{
	double retval = numeric_limits<double>::max(); 
	if (!m_v.empty())
		retval = std::accumulate(m_v.begin(), m_v.end(), 0.0)/m_v.size();
	return retval; 
}

double CConvergenceMeasure::rate() const
{
	double retval = 0.0;
	if (m_v.size() > 1) {
		double sum_v = 0.0;
		double sum_xv = 0.0;
		double sum_x2 = 0.0; 
		for (std::size_t i = 0, k=1; i < m_v.size(); ++i, ++k) {
			sum_v  += m_v[i];
			sum_xv += k * m_v[i];
			sum_x2 += k*k; 
		}
		double x_mean = (m_v.size() + 1.0) / 2.0;
		retval = ((sum_xv - sum_v  *  x_mean) / m_v.size()) /
			(sum_x2 / m_v.size()  - x_mean * x_mean); 
	}
	return retval; 
}

uint32_t CConvergenceMeasure::fill() const
{
        return m_v.size(); 
}
            

bool CConvergenceMeasure::is_full_size() const
{
        return fill() >= m_size; 

}

}
