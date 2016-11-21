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
#include <limits>

using std::numeric_limits; 

namespace mia {

CConvergenceMeasure::CConvergenceMeasure(uint32_t size):
        m_size(size),
        m_current_sum(0.0)
{
        
}

void CConvergenceMeasure::push(double value)
{
        m_values.push(value);
        m_current_sum += value;

        if (m_values.size() > m_size) {
                m_current_sum -= m_values.front();
                m_values.pop();
        }
}

double CConvergenceMeasure::value() const
{
        return m_values.empty() ? numeric_limits<double>::max() : m_current_sum / fill(); 
}

uint32_t CConvergenceMeasure::fill() const
{
        return m_values.size(); 
}
            

bool CConvergenceMeasure::is_full_size() const
{
        return fill() >= m_size; 

}

}
