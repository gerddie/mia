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

#include <cassert>
#include <stdexcept>
#include <mia/internal/similarity_profile.hh>

NS_MIA_BEGIN

template <int dim>
TSimilarityProfile<dim>::TSimilarityProfile(PFullCost cost, 
					    const ImageSeries& images, 
					    size_t _reference):
	m_reference(_reference),
	m_peak_freq(-1),
	m_peak_freq_valid(false)
{
	// +-2 makes sure that the implementation of get_periodic_subset works
	assert(m_reference < images.size() - 2 && m_reference >= 2); 
	

	auto ref = images[m_reference]; 
	save_image("ref.@", ref); 
	
	for(auto i = images.begin(); i != images.end(); ++i) {
		save_image("src.@", *i);
		cost->reinit();
		cost->set_size((*i)->get_size()); 
		m_cost_values.push_back( cost->cost_value()); 
	}
}

template <int dim>
TSimilarityProfile<dim>::TSimilarityProfile(const TSimilarityProfile<dim>& other):
	m_reference(other.m_reference), 
	m_cost_values(other.m_cost_values)
{
	m_peak_freq = other.m_peak_freq;
	m_peak_freq_valid = other.m_peak_freq_valid; 
}

template <int dim>
TSimilarityProfile<dim>& TSimilarityProfile<dim>::operator = (const TSimilarityProfile<dim>& other)
{
	if (this != &other) {
		m_reference = other.m_reference; 
		m_cost_values = other.m_cost_values; 
		m_peak_freq = other.m_peak_freq;
		m_peak_freq_valid = other.m_peak_freq_valid; 
	}
	return *this; 
}

template <int dim>
float TSimilarityProfile<dim>::get_peak_frequency() const
{
	if (!m_peak_freq_valid) {
		CFFT1D_R2C fft(m_cost_values.size());
		cvdebug() << "costs:" << m_cost_values << "\n"; 
		auto freq = fft.forward(m_cost_values);
		for (auto i = freq.begin() + 1; i != freq.end(); ++i) {
			const float n = norm<float>(*i);
			float snorm = sqrt(n);
			if (snorm > m_peak_freq)
				m_peak_freq = snorm; 
		}
		m_peak_freq_valid = true; 
	}
	return m_peak_freq;
}

template <int dim>
vector<size_t> TSimilarityProfile<dim>::get_periodic_subset() const
{
	std::vector<size_t> result; 
	
	result.push_back(m_reference); 
	size_t i = m_reference - 1; 
	cvinfo()  << "Similarity profile["<< m_reference <<"]:" 
		  << m_cost_values << "\n"; 

	while (i > 2) {
		if (m_cost_values[i] < m_cost_values[i + 1] 
		    && m_cost_values[i] < m_cost_values[i + 2]
		    && m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2])  {
			result.push_back(i); 
			i -= 3; 
		}else
			--i; 
	}
	result.push_back(0); 
		
	i = m_reference + 1; 
	while (i < m_cost_values.size() - 2) {
		if (m_cost_values[i] < m_cost_values[i + 1] 
		    && m_cost_values[i] < m_cost_values[i + 2]
		    && m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2]) {
			result.push_back(i); 
			i += 3; 
		}
		else
			++i; 
	}
	// append butlast if better then last 
	// a the the end of the series the changes in intesnity should 
	// not be so big 
	while (i < m_cost_values.size() - 1) {
		if (m_cost_values[i] < m_cost_values[i + 1] 
		    && m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2]) {
			result.push_back(i); 
			i += 3; 
		}
		else
			++i; 
	}
	// not yet past the end, therefore, we may want t o add the last image
	while (i < m_cost_values.size()) {
		if (m_cost_values[i] < m_cost_values[i - 1]
		    && m_cost_values[i] < m_cost_values[i - 2]) {
			result.push_back(i); 
			i += 3; 
		}
		else
			++i; 
	}
	
	sort(result.begin(), result.end()); 
	return result; 
}
	
NS_MIA_END
