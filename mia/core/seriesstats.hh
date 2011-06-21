/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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


#ifndef mia_2d_seriesstats_hh
#define mia_2d_seriesstats_hh


#include <mia/core/filter.hh>

NS_MIA_BEGIN

struct SIntensityStats {
	double sum; 
	double sumsq; 
	double mean; 
	double variation; 
	double min; 
	double max; 
	size_t n; 
}; 

class FIntensityStatsAccumulator : public TFilter<void> {
public: 
	FIntensityStatsAccumulator(); 
	
	template <typename Container> 
	void operator () ( const Container& data); 
	
	void finalize(); 

	const SIntensityStats& get_result() const; 
private: 
	SIntensityStats m_stats; 
};  

template <typename Container> 
void FIntensityStatsAccumulator::operator () ( const Container& data)
{
	m_stats.n += data.size(); 
	for (auto i = data.begin(); i != data.end(); ++i) {
		m_stats.sum += *i; 
		m_stats.sumsq += *i * *i; 
		if (m_stats.min > *i) 
			m_stats.min = *i; 
		if (m_stats.max < *i) 
			m_stats.max = *i; 
	}
}

NS_MIA_END

#endif
