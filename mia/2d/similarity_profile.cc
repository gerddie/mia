/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009-2010
 *
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

#include <cassert>
#include <stdexcept>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/similarity_profile.hh>

NS_MIA_BEGIN

using namespace std; 

C2DSimilarityProfile::C2DSimilarityProfile(P2DFullCost cost, 
					   const C2DImageSeries& images, 
					   size_t _skip, size_t _reference):
	m_skip(_skip),
	m_reference(_reference),
	m_peak_freq(-1),
	m_peak_freq_valid(false)
{
	// +-2 makes sure that the implementation of get_periodic_subset works
	assert(m_reference < images.size() - 2 && m_reference >= m_skip + 2); 
	
	if (m_skip >= images.size()) {
		THROW(invalid_argument, "C2DSimilarityProfile: Trying to skip "
		      << m_skip << " images of a set of only " << images.size() << " images"); 
	}
	
	P2DImage ref = images[m_reference]; 
	save_image2d("ref.@", ref); 
	
	for(auto i = images.begin() + m_skip; i != images.end(); ++i) {
		save_image2d("src.@", *i);
		cost->reinit(); 
		m_cost_values.push_back( cost->cost_value()); 
	}
}


float C2DSimilarityProfile::get_peak_frequency() const
{
	{
		boost::mutex::scoped_lock lock(_M_peak_freq_mutex);
		if (!m_peak_freq_valid) {
			CFFT1D_R2C fft(m_cost_values.size());
			vector<CFFT1D_R2C::Complex> freq = fft.forward(m_cost_values);
			for (auto i = freq.begin() + 1; i != freq.end(); ++i) {
				const float n = norm<float>(*i);
				float snorm = sqrt(n);
				if (snorm > m_peak_freq)
					m_peak_freq = snorm; 
			}
			m_peak_freq_valid = true; 
		}
	}
	return m_peak_freq;
}

vector<size_t> C2DSimilarityProfile::get_periodic_subset() const
{
	vector<size_t> result; 
	
	result.push_back(m_reference); 
	size_t i = m_reference - m_skip; 
	
	while (i > 2) {
		if (m_cost_values[i] < m_cost_values[i + 1] 
		    && m_cost_values[i] < m_cost_values[i + 2]
		    && m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2]) {
			result.push_back(i + m_skip); 
			i -= 3; 
		}else
			--i; 
	}
	result.push_back(m_skip); 
		
	i = m_reference - m_skip; 
	while (i < m_cost_values.size() - 2) {
		if (m_cost_values[i] < m_cost_values[i + 1] 
		    && m_cost_values[i] < m_cost_values[i + 2]
		    && m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2]) {
			result.push_back(i + m_skip); 
			i += 3; 
		}
		else
			++i; 
	}
	result.push_back(m_cost_values.size() + m_skip - 1); 
	sort(result.begin(), result.end()); 
	return result; 
}
	
NS_MIA_END
