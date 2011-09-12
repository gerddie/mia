/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
private:

	void evaluate_curve_length() const;
	void evaluate_range() const;
	void evaluate_perfusion_peak() const;
	void evaluate_frequency() const; 

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
	mutable bool m_wt_valid;

	mutable pair<size_t, float>  m_first_peak;
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
	m_range_valid(false),
	m_perfusion_peak_valid(false),
	m_mean_freq_valid(false), 
	m_wt_valid(false), 
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


float CSlopeStatistics::get_positive_time_mean() const
{
	impl->get_positive_time_mean(); 
}


void CSlopeStatisticsImpl::evaluate_frequency() const
{
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

CSlopeStatistics::EEnergyCenterpos CSlopeStatistics::get_mean_energy_position() const
{
	return impl->get_mean_energy_position(); 
}

const vector<CSlopeStatistics::EEnergyCenterpos>& CSlopeStatistics::get_level_mean_energy_position() const
{
	return impl->get_level_mean_energy_position(); 
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
	vector<double> help(m_series.size());
	if (m_series[0] < mean)
		transform(m_series.begin(), m_series.end(), help.begin(),[mean](float x){return x - mean;});
	else
		transform(m_series.begin(), m_series.end(), help.begin(),[mean](float x){return mean - x;});

	auto filter = C1DSpacialKernelPluginHandler::instance().produce("gauss:w=2"); 
	help = filter->apply(help); 
	
	m_maximum_gradient_from_zero = 0.0; 
	for (int i = 1; i < help.size(); ++i) {
		float h = help[i] / i; 
		if (m_maximum_gradient_from_zero < h) {
			m_maximum_gradient_from_zero = h; 
			m_perfusion_peak.first = i; 
			m_perfusion_peak.second = help[i]; 
		}
	}
	cvinfo() << "Slope: " << get_index() 
		 << ": max(f(x)/x)@x= " 
		 <<  m_perfusion_peak.first 
		 << " = " 
		 << m_maximum_gradient_from_zero << "\n"; 

	int start_peak_area = m_perfusion_peak.first; 
	while (help[start_peak_area] > 0 && start_peak_area > 0) 
		--start_peak_area; 
	if (help[start_peak_area] < 0) 
		++start_peak_area; 

	cvinfo() << m_index <<" peak pos = " << m_perfusion_peak.first
		 << " start peak area = " << start_peak_area
		 << "\n"; 	
	float timmean = 0.0; 
	float sum = 0.0;  
	bool stop = false; 
	for (int i = start_peak_area; i < help.size(); ++i) {
		if (help[i]>0) {
			timmean += i * m_series[i]; 
			sum += help[i];
		}else
			break; 
	}
	if ( sum > 0.0) 
		m_energy_time_mean = timmean / sum; 

	m_perfusion_peak_valid = true; 
}

void CSlopeStatisticsImpl::evaluate_range() const
{
	pair<position, position> minmax = minmax_element(m_series.begin(), m_series.end());
	m_range = *minmax.second - *minmax.first;
	m_range_valid = true;

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

void CSlopeStatisticsImpl::evaluate_wt() const
{
	C1DWavelet wt(wt_daubechies_centered, 10);
	
	auto wt_transformed = wt.forward(m_series); 
	
	transform(wt_transformed.begin(), wt_transformed.end(), wt_transformed.begin(), 
		  [](double x) { return fabs(x);}); 
	
	int levels = log2(wt_transformed.size()); 
	int ncoeffs = 1; 
	m_wt_peak_coefficient = 0.0; 
	auto c = wt_transformed.begin() + 1; 
	float mean_level = 0;
	float sum_level_peaks = 0; 
	m_wt_energy = 0.0; 
	m_wt_level_coefficient_sums.resize(levels); 
	m_wt_level_mean_energy_pos.resize(levels); 


	for (int l = 0; l < levels; ++l, ncoeffs *= 2) {
		float peak_level_coeff = 0.0; 
		m_wt_level_coefficient_sums[l] = 0; 
		float wt_level_mean_pos = 0; 
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
		}
		mean_level += peak_level_coeff * l; 
		sum_level_peaks += peak_level_coeff; 
		if (!m_wt_level_coefficient_sums[l]) {
			CSlopeStatistics::ecp_none;
			continue; 
		}
		
		// evalaute the mean time position of the energy
		if (ncoeffs == 1)
			m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_center;
		else if (ncoeffs == 2) {
			cvdebug() << "ncoeffs = 2: weighted pos = " << wt_level_mean_pos  
				  << " sum = " << m_wt_level_coefficient_sums[l]
				  << "\n"; 
			wt_level_mean_pos /= m_wt_level_coefficient_sums[l];

			m_wt_level_mean_energy_pos[l] = (wt_level_mean_pos < 0.5) ? CSlopeStatistics::ecp_begin : 
				( wt_level_mean_pos == 0.5 ? CSlopeStatistics::ecp_center: CSlopeStatistics::ecp_end );
		}else {
			cvdebug() << "level = " << l 
				  << " wt_level_mean_pos = " << wt_level_mean_pos 
				  << " m_wt_level_coefficient_sums = " << m_wt_level_coefficient_sums[l] 
				  << " ncoeffs = " << ncoeffs 
				  << "\n"; 
			wt_level_mean_pos /= m_wt_level_coefficient_sums[l] * ncoeffs / 3.0;
			cvdebug() << "corrected = " << wt_level_mean_pos << "\n"; 
			if (wt_level_mean_pos <= 1.0 && ncoeffs > 1) {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_begin;
			}else if ((wt_level_mean_pos <= 2.0 && ncoeffs > 3) || ncoeffs == 1 ) {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_center; 
			} else {
				m_wt_level_mean_energy_pos[l] = CSlopeStatistics::ecp_end;
			}
		}
		
	}

	int at_begin = 0; 
	int at_center = 0; // the highest level is always centered 
	int at_end = 0; 
	
	for (int l = 2; l < levels; ++l) {
		switch (m_wt_level_mean_energy_pos[l]) {
		case CSlopeStatistics::ecp_begin: ++at_begin; break; 
		case CSlopeStatistics::ecp_end: ++at_end; break; 
		case CSlopeStatistics::ecp_center: ++at_center; break; 
		}	
	}
	
	if ( at_begin > at_center && at_begin > at_end ) 
		m_energy_pos = CSlopeStatistics::ecp_begin; 
	else if ( at_end > at_center && at_begin < at_end ) 
		m_energy_pos = CSlopeStatistics::ecp_end; 
	else 
		m_energy_pos = CSlopeStatistics::ecp_center;
	
	m_wt_mean_wt_level = mean_level / sum_level_peaks; 
	m_wt_valid = true; 
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
	return impl->get_maximum_gradient_from_zero(); 
}


NS_MIA_END
