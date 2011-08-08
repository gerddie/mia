/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#include <limits>
#include <cmath>
#include <mia/core/seriesstats.hh>

NS_MIA_BEGIN

using std::numeric_limits; 


FIntensityStatsAccumulator::FIntensityStatsAccumulator():
	m_stats_valid(false)
{
	m_stats.sum = 0; 
	m_stats.sumsq = 0;
	m_stats.mean  = 0;
	m_stats.variation = 0; 
	m_stats.min = numeric_limits<double>::max(); 
	m_stats.max =-numeric_limits<double>::max(); 
	m_stats.n = 0; 
}
		
const SIntensityStats& FIntensityStatsAccumulator::get_result() const
{
	if (!m_stats_valid) {
		if (m_stats.n > 0) 
			m_stats.mean = m_stats.sum / m_stats.n; 
		if (m_stats.n > 1) 
			m_stats.variation = sqrt(( m_stats.sumsq - m_stats.sum * m_stats.mean) / (m_stats.n - 1));
		m_stats_valid = true; 
	}
	return m_stats; 
}

NS_MIA_END
