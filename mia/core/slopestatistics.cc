/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <gsl++/wavelet.hh>

#include <mia/core/spacial_kernel.hh>
#include <mia/core/slopestatistics.hh>
#include <mia/core/fft1d_r2c.hh>
#include <mia/core/msgstream.hh>
#include <cmath>
#include <iostream>
#include <numeric>
#include <complex>

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <boost/algorithm/minmax_element.hpp>
#endif

NS_MIA_BEGIN

using namespace gsl; 
using namespace std;
using namespace boost;


struct CSlopeStatisticsImpl {
	CSlopeStatisticsImpl(const vector<float>& series, int index);

	float get_curve_length() const;
	float get_range() const;
	pair<size_t, float>  get_first_peak() const;
	pair<size_t, float>  get_second_peak() const;
	pair<size_t, float>  get_perfusion_high_peak() const;
	float get_mean_frequency() const;
	float get_energy() const;

	float get_positive_time_mean() const; 

	float get_mean_frequency_level() const;
	pair<int, int> get_peak_level_and_time_index() const; 
	float get_peak_wavelet_coefficient() const; 
	float get_wavelet_energy() const; 
	const vector<float>& get_level_coefficient_sums() const; 
	const vector<CSlopeStatistics::EEnergyCenterpos>& get_level_mean_energy_position() const; 
	CSlopeStatistics::EEnergyCenterpos get_mean_energy_position() const; 
	float get_maximum_gradient_from_zero() const; 
	int get_index() const; 
	std::pair<size_t, float>  get_gradient_peak(int start_movement) const; 
	float get_level_change(size_t center) const; 
	int get_max_frequency_slot() const; 
private:

	void evaluate_curve_length() const;
	void evaluate_range() const;
	void evaluate_perfusion_peak() const;
	void evaluate_frequency() const; 
	void evaluate_gradient_peak(int start_movement) const;
	void evaluate_wt() const; 
	
	vector<float> m_series;
	
	mutable vector<double> m_series_lowpass;
	mutable bool m_curve_length_valid;
	mutable float m_curve_length;
	mutable bool m_range_valid;
	mutable bool m_perfusion_peak_valid;
	mutable float m_range;
	mutable float m_mean_freq;
	mutable float m_energy;
	mutable bool m_mean_freq_valid;
	mutable int m_start_movement;
	mutable bool m_gradient_peak_valid;
	mutable bool m_wt_valid;
	mutable int m_max_freq_slot; 


	mutable pair<size_t, float>  m_first_peak;
	mutable pair<size_t, float>  m_gradient_peak;
	mutable pair<size_t, float>  m_second_peak;
	mutable pair<size_t, float>  m_perfusion_peak;
	mutable pair<int, int>  m_wt_peak_level_and_index;
	mutable float m_wt_peak_coefficient;
	mutable float m_wt_mean_wt_level;
	mutable float m_wt_energy;
	mutable vector<float> m_wt_level_coefficient_sums; 
	mutable vector<CSlopeStatistics::EEnergyCenterpos> m_wt_level_mean_energy_pos; 
	mutable CSlopeStatistics::EEnergyCenterpos m_energy_pos; 
	mutable float m_energy_time_mean; 
	mutable float m_maximum_gradient_from_zero; 
	int m_index; 

	typedef vector<float>::const_iterator position;

};

CSlopeStatistics::CSlopeStatistics(const vector<float>& series, int index):
	impl(new CSlopeStatisticsImpl(series, index))
{
}

int CSlopeStatistics::get_index() const
{
	return impl->get_index(); 
}

int CSlopeStatisticsImpl::get_index() const
{
	return m_index; 
}

CSlopeStatistics::~CSlopeStatistics()
{
	delete impl;
}

float CSlopeStatistics::get_curve_length() const
{
	return impl->get_curve_length();
}

float CSlopeStatistics::get_mean_frequency() const
{
	return impl->get_mean_frequency();
}

float CSlopeStatistics::get_energy() const
{
	return impl->get_energy();
}


CSlopeStatisticsImpl::CSlopeStatisticsImpl(const vector<float>& series, int index):
	m_series(series),
	m_curve_length_valid(false),
	m_curve_length(0.0), 
	m_range_valid(false),
	m_perfusion_peak_valid(false),
	m_range(0.0), 
	m_mean_freq(0.0), 
	m_energy(0.0), 
	m_mean_freq_valid(false), 
	m_start_movement(-1), 
	m_gradient_peak_valid(false), 
	m_wt_valid(false), 
	m_max_freq_slot(0), 
	m_wt_peak_coefficient(0),
	m_wt_mean_wt_level(0),
	m_wt_energy(0),
	m_energy_pos(CSlopeStatistics::ecp_none), 
	m_energy_time_mean(0), 
	m_maximum_gradient_from_zero(0), 
	m_index(index)
{
}

float CSlopeStatisticsImpl::get_curve_length() const
{
	if (!m_curve_length_valid)
		evaluate_curve_length();
	return m_curve_length;
}

float CSlopeStatistics::get_range() const
{
	return impl->get_range();
}

pair<size_t, float>  CSlopeStatistics::get_first_peak() const
{
	return impl->get_first_peak();
}

pair<size_t, float>  CSlopeStatistics::get_second_peak() const
{
	return impl->get_second_peak();
}

pair<size_t, float>  CSlopeStatistics::get_perfusion_high_peak() const
{
	return impl->get_perfusion_high_peak();
}


float CSlopeStatistics::get_mean_frequency_level() const
{
	return impl->get_mean_frequency_level(); 
}

float CSlopeStatistics::get_wavelet_energy() const
{
	return impl->get_wavelet_energy(); 
}

float CSlopeStatistics::get_peak_wavelet_coefficient() const
{
	return impl->get_peak_wavelet_coefficient(); 
}

const vector<float>& CSlopeStatistics::get_level_coefficient_sums() const
{
	return impl->get_level_coefficient_sums(); 
}

pair<int, int> CSlopeStatistics::get_peak_level_and_time_index() const
{
	return impl->get_peak_level_and_time_index(); 
}

float CSlopeStatistics::get_maximum_gradient_from_zero() const
{
	FUNCTION_NOT_TESTED; 
	return impl->get_maximum_gradient_from_zero(); 
}

float CSlopeStatistics::get_positive_time_mean() const
{
	FUNCTION_NOT_TESTED; 
	return impl->get_positive_time_mean(); 
}

CSlopeStatistics::EEnergyCenterpos CSlopeStatistics::get_mean_energy_position() const
{
	return impl->get_mean_energy_position(); 
}

const vector<CSlopeStatistics::EEnergyCenterpos>& CSlopeStatistics::get_level_mean_energy_position() const
{
	return impl->get_level_mean_energy_position(); 
}




pair<size_t, float>  CSlopeStatisticsImpl::get_first_peak() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_first_peak;
}

pair<size_t, float>  CSlopeStatisticsImpl::get_second_peak() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_second_peak;
}

pair<size_t, float>  CSlopeStatisticsImpl::get_perfusion_high_peak() const
{
	if (!m_perfusion_peak_valid)
		evaluate_perfusion_peak();
	return m_perfusion_peak;
}
float CSlopeStatisticsImpl::get_positive_time_mean() const
{
	if (!m_perfusion_peak_valid)
		evaluate_perfusion_peak();
	return m_energy_time_mean;
}


int CSlopeStatistics::get_max_frequency_slot() const
{
	assert(impl); 
	return impl->get_max_frequency_slot(); 
}

int CSlopeStatisticsImpl::get_max_frequency_slot() const
{
	if (!m_mean_freq_valid)
		evaluate_frequency(); 
	return m_max_freq_slot; 
}

void CSlopeStatisticsImpl::evaluate_frequency() const
{
	float max_slot_energy = 0.0; 
	m_max_freq_slot = 0; 
	m_mean_freq = 0.0;
	m_energy = 0.0;
	CFFT1D_R2C fft(m_series.size());
	vector<CFFT1D_R2C::Complex> freq = fft.forward(m_series);
	int k = 1;
	for (vector<CFFT1D_R2C::Complex>::const_iterator i =
		     freq.begin() + 1; i != freq.end(); ++i, ++k) {
		const float n = norm<float>(*i);
		float snorm = sqrt(n);
		m_mean_freq += k * snorm;
		m_energy += snorm;
		if (max_slot_energy < snorm) {
			max_slot_energy = snorm; 
			m_max_freq_slot = k; 
		}
	}
	m_mean_freq /= m_energy;
	m_mean_freq_valid = true;
	
}

float CSlopeStatisticsImpl::get_energy() const
{
	if (!m_mean_freq_valid)
		evaluate_frequency(); 
	return m_energy;
}

float CSlopeStatisticsImpl::get_mean_frequency() const
{
	if (!m_mean_freq_valid)
		evaluate_frequency(); 
	return m_mean_freq;
}

float CSlopeStatisticsImpl::get_range() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_range;
}


const vector<float>& CSlopeStatisticsImpl::get_level_coefficient_sums() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_level_coefficient_sums; 
}

const vector<CSlopeStatistics::EEnergyCenterpos>& CSlopeStatisticsImpl::get_level_mean_energy_position() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_level_mean_energy_pos; 
}

CSlopeStatistics::EEnergyCenterpos CSlopeStatisticsImpl::get_mean_energy_position() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_energy_pos; 
}


void CSlopeStatisticsImpl::evaluate_curve_length() const
{
	m_curve_length = 0.0f;
	for (size_t i = 0; i < m_series.size() - 1; ++i) {
		const float delta = m_series[i] - m_series[i+1];
		m_curve_length +=  sqrt(delta * delta + 1.0f);
	}
	m_curve_length_valid = true;
}

float CSlopeStatisticsImpl::get_maximum_gradient_from_zero() const
{
	if (!m_perfusion_peak_valid) 
		evaluate_perfusion_peak(); 
	return m_maximum_gradient_from_zero; 
}

void CSlopeStatisticsImpl::evaluate_perfusion_peak() const
{
	float mean = accumulate(m_series.begin(), m_series.end(), 0.0) / m_series.size();
	cvdebug() << "=== Slope[" << get_index() << "] evaluate perfusion peak\n"; 
	cvdebug() << "  mean = " << mean << "\n"; 
	vector<double> help(m_series.size());
	if (m_series[0] < mean)
		transform(m_series.begin(), m_series.end(), help.begin(),[mean](float x){return x - mean;});
	else
		transform(m_series.begin(), m_series.end(), help.begin(),[mean](float x){return mean - x;});

	
	m_maximum_gradient_from_zero = 0.0; 
	for (size_t i = 1; i < help.size(); ++i) {
		float h = help[i] / i; 
		if (m_maximum_gradient_from_zero < h) {
			m_maximum_gradient_from_zero = h; 
			m_perfusion_peak.first = i; 
			m_perfusion_peak.second = help[i]; 
		}
	}
	// now find true local maximum 
	for(size_t i = m_perfusion_peak.first + 1; i < help.size(); ++i) {
		cvdebug() << "  test " << i << " " << help[i]<< " vs " << m_perfusion_peak.second << "\n"; 
		if (help[i] > m_perfusion_peak.second) {
			m_perfusion_peak.first = i; 
			m_perfusion_peak.second = help[i]; 
		}else if (help[i] < 0.0) 
			break; 
	}

	m_perfusion_peak.second = m_series[m_perfusion_peak.first]; 
	cvdebug() << "   max(f(x)/x)@x= " 
		  <<  m_perfusion_peak.first 
		  << " = " 
		  << m_maximum_gradient_from_zero << "\n"; 
	
	int start_peak_area = m_perfusion_peak.first; 
	while (help[start_peak_area] > 0 && start_peak_area > 0) 
		--start_peak_area; 
	if (help[start_peak_area] < 0) 
		++start_peak_area; 

	cvdebug() << "   peak pos = " << m_perfusion_peak.first
		  << " start peak area = " << start_peak_area
		  << "\n"; 
	
	float timmean = 0.0; 
	float sum = 0.0;  
	for (size_t i = start_peak_area; i < help.size(); ++i) {
		if (help[i]>0) {
			timmean += i * m_series[i]; 
			sum += help[i];
		}else
			break; 
	}
	if ( sum > 0.0) 
		m_energy_time_mean = timmean / sum; 

	m_perfusion_peak_valid = true; 

	cvdebug() << "======================\n"; 
}

void CSlopeStatisticsImpl::evaluate_range() const
{
	pair<position, position> minmax = minmax_element(m_series.begin(), m_series.end());
	m_range = *minmax.second - *minmax.first;
	m_range_valid = true;

	cvinfo() << "Slope[" << get_index() << "] Range: [" 
		 << *minmax.first  << ", " << *minmax.second << "] = " << m_range << "\n"; 

	size_t delta1 = distance(m_series.begin(), minmax.first);
	size_t delta2 = distance(m_series.begin(), minmax.second);
	m_first_peak.first = delta1 < delta2 ? delta1 : delta2;
	m_first_peak.second = m_series[m_first_peak.first];
	m_second_peak.first = delta1 < delta2 ? delta2 : delta1;
	m_second_peak.second = m_series[m_second_peak.first];
}

float CSlopeStatisticsImpl::get_wavelet_energy() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_energy; 
}

float CSlopeStatisticsImpl::get_mean_frequency_level() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_mean_wt_level; 
}

float CSlopeStatisticsImpl::get_peak_wavelet_coefficient() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_peak_coefficient; 
}


pair<int,int> CSlopeStatisticsImpl::get_peak_level_and_time_index() const
{
	if (!m_wt_valid) 
		evaluate_wt(); 
	return m_wt_peak_level_and_index; 

}

float CSlopeStatistics::get_level_change(size_t center) const
{
	return impl->get_level_change(center); 
}

float CSlopeStatisticsImpl::get_level_change(size_t center) const
{
	if (center >= m_series.size() - 1) {
		return -(accumulate(m_series.begin(), m_series.begin(), 0.0f) / m_series.size()); 
	}
	if  (center <= 0) {
		return (accumulate(m_series.begin(), m_series.begin(), 0.0f) / m_series.size()); 
	}
	
	float before = accumulate(m_series.begin(), m_series.begin() + center, 0.0f) / center; 
	float after = accumulate(m_series.begin() + center + 1, m_series.end(), 0.0f) / (m_series.size() - center - 1);
	return after - before; 
}						

void CSlopeStatisticsImpl::evaluate_wt() const
{
	C1DWavelet wt(wt_daubechies_centered, 10);
	
	auto wt_transformed = wt.forward(m_series); 
	
	transform(wt_transformed.begin(), wt_transformed.end(), wt_transformed.begin(), 
		  [](double x) { return fabs(x);}); 
	
	int levels = log2(wt_transformed.size()); 
	if (levels < 2) 
		throw create_exception<invalid_argument>("CSlopeStatistics::evaluate_wt: Series size", wt_transformed.size(), 
							 "too short for a sensible evaluation"); 

	int ncoeffs = 1; 
	m_wt_peak_coefficient = 0.0; 
	auto c = wt_transformed.begin() + 1; 
	float mean_level = 0;
	float sum_level_peaks = 0; 
	m_wt_energy = 0.0; 
	m_wt_level_coefficient_sums.resize(levels); 
	m_wt_level_mean_energy_pos.resize(levels); 


	float wt_pos_sum = 0.0; 

	for (int l = 0; l < levels; ++l, ncoeffs *= 2) {
		float peak_level_coeff = 0.0; 
		m_wt_level_coefficient_sums[l] = 0; 
		float wt_level_mean_pos = 0; 
		float wt_level_mean = 0; 
		float wt_level_sigma = 0; 
		for (int i = 0; i < ncoeffs; ++i, ++c) {
			if ( m_wt_peak_coefficient < *c) {
				m_wt_peak_coefficient = *c; 
				m_wt_peak_level_and_index.first = l; 
				m_wt_peak_level_and_index.second = i;
			}
			if (peak_level_coeff < *c)
				peak_level_coeff = *c; 
			m_wt_energy += *c; 
			m_wt_level_coefficient_sums[l] += *c; 
			wt_level_mean_pos += *c * i;

			float fi = 3.0 * float(i) / float(ncoeffs - 1.0); 
			wt_level_mean += *c * fi;
			wt_level_sigma += *c * fi * fi;
		}
		mean_level += peak_level_coeff * l; 
		sum_level_peaks += peak_level_coeff; 
		if (!m_wt_level_coefficient_sums[l]) {
			m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_none;
			continue; 
		}
		
		if (ncoeffs > 1)
			wt_pos_sum += wt_level_mean / m_wt_level_coefficient_sums[l]; 

		// evalaute the mean time position of the energy
		if (ncoeffs == 1)
			m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_none;
		else if (ncoeffs == 2) {
			cvdebug() << "ncoeffs = 2: weighted pos = " << wt_level_mean_pos  
				  << " sum = " << m_wt_level_coefficient_sums[l]
				  << "\n"; 
			wt_level_mean_pos /= m_wt_level_coefficient_sums[l] / 3.0;

			m_wt_level_mean_energy_pos[l] = (wt_level_mean_pos < 1.0) ? CSlopeStatistics::ecp_begin : 
				( wt_level_mean_pos < 2.0 ? CSlopeStatistics::ecp_center: CSlopeStatistics::ecp_end );

			
		}else {
			cvdebug() << "level = " << l 
				  << " wt_level_mean_pos = " << wt_level_mean_pos 
				  << " m_wt_level_coefficient_sums = " << m_wt_level_coefficient_sums[l] 
				  << " ncoeffs = " << ncoeffs 
				  << "\n"; 
			wt_level_mean_pos /= m_wt_level_coefficient_sums[l] * ncoeffs / 3.0;
			cvdebug() << "corrected = " << wt_level_mean_pos << "\n"; 


			if (wt_level_mean_pos <= 1.0) {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_begin;
			}else if ((wt_level_mean_pos <= 2.0 && ncoeffs > 3) || ncoeffs == 1 ) {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_center; 
			} else {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_end;
			}
		}
		
	}

	wt_pos_sum /= levels - 1; 

	cvinfo() << "wt_pos_sum= " << wt_pos_sum << "\n"; 

	m_energy_pos = wt_pos_sum <= 1.0 ? CSlopeStatistics::ecp_begin : 
		(wt_pos_sum <= 2.0 ? CSlopeStatistics::ecp_center : CSlopeStatistics::ecp_end); 

	cvinfo() << "slope energy peak:" << m_energy_pos << "\n"; 
	m_wt_mean_wt_level = mean_level / sum_level_peaks; 
	m_wt_valid = true; 
}


std::pair<size_t, float>  CSlopeStatistics::get_gradient_peak(int start_movement) const
{
	return impl->get_gradient_peak(start_movement); 
}

std::pair<size_t, float>  CSlopeStatisticsImpl::get_gradient_peak(int start_movement) const
{
	if (!m_gradient_peak_valid || m_start_movement != start_movement) 
		evaluate_gradient_peak(start_movement); 
	return m_gradient_peak; 
	
}

void CSlopeStatisticsImpl::evaluate_gradient_peak(int start_movement) const
{
	m_start_movement = start_movement; 
	m_gradient_peak.first = m_series.size(); 
	m_gradient_peak.second = 0 ;

	const double f1 = 1.0/3840.0; 
	const double f2 = 1416.0/ 23040.0; 
	const double f3 = 17340.0/ 46080.0; 
	
	size_t end = start_movement > 3 ? start_movement - 3 : m_series.size() - 3; 
	
	for (size_t i = 3; i < end; ++i) {
		const double gradient = f1 * (m_series[i + 3] - m_series[i - 3]) + 
			f2 * (m_series[i + 2] - m_series[i - 2]) + 
			f3 * (m_series[i + 1] - m_series[i - 1]); 
		if (m_gradient_peak.second < gradient) {
			m_gradient_peak.second =  gradient; 
			m_gradient_peak.first = i; 
		}
	}
	m_gradient_peak_valid = true; 
}

NS_MIA_END
