/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/core/slopestatistics.hh>
#include <mia/core/fft1d_r2c.hh>
#include <mia/core/msgstream.hh>
#include <cmath>
#include <iostream>
#include <numeric>
#include <boost/lambda/lambda.hpp>
#include <complex>

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <boost/algorithm/minmax_element.hpp>
#endif

NS_MIA_BEGIN

using namespace std;
using namespace boost;
using boost::lambda::_1;

struct CSlopeStatisticsImpl {
	CSlopeStatisticsImpl(const vector<float>& series);

	float get_curve_length() const;
	float get_range() const;
	std::pair<size_t, float>  get_first_peak() const;
	std::pair<size_t, float>  get_second_peak() const;
	std::pair<size_t, float>  get_perfusion_high_peak() const;
	float get_mean_frequency() const;
private:
	void evaluate_curve_length() const;
	void evaluate_range() const;
	void evaluate_perfusion_peak() const;

	vector<float> m_series;
	mutable bool m_curve_length_valid;
	mutable float m_curve_length;
	mutable bool m_range_valid;
	mutable bool m_perfusion_peak_valid;
	mutable float m_range;
	mutable float m_mean_freq;
	mutable bool m_mean_freq_valid;

	mutable std::pair<size_t, float>  m_first_peak;
	mutable std::pair<size_t, float>  m_second_peak;
	mutable std::pair<size_t, float>  m_perfusion_peak;
	typedef vector<float>::const_iterator position;

};

CSlopeStatistics::CSlopeStatistics(const std::vector<float>& series):
	impl(new CSlopeStatisticsImpl(series))
{
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

CSlopeStatisticsImpl::CSlopeStatisticsImpl(const vector<float>& series):
	m_series(series),
	m_curve_length_valid(false),
	m_range_valid(false),
	m_perfusion_peak_valid(false),
	m_mean_freq_valid(false)
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

std::pair<size_t, float>  CSlopeStatistics::get_first_peak() const
{
	return impl->get_first_peak();
}

std::pair<size_t, float>  CSlopeStatistics::get_second_peak() const
{
	return impl->get_second_peak();
}

std::pair<size_t, float>  CSlopeStatistics::get_perfusion_high_peak() const
{
	return impl->get_perfusion_high_peak();
}

std::pair<size_t, float>  CSlopeStatisticsImpl::get_first_peak() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_first_peak;
}

std::pair<size_t, float>  CSlopeStatisticsImpl::get_second_peak() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_second_peak;
}

std::pair<size_t, float>  CSlopeStatisticsImpl::get_perfusion_high_peak() const
{
	if (!m_perfusion_peak_valid)
		evaluate_perfusion_peak();
	return m_perfusion_peak;
}

float CSlopeStatisticsImpl::get_mean_frequency() const
{
	if (!m_mean_freq_valid) {
		m_mean_freq = 0.0;
		float sum_freq = 0.0;
		CFFT1D_R2C fft(m_series.size());
		vector<CFFT1D_R2C::Complex> freq = fft.forward(m_series);
		int k = 1;
		for (vector<CFFT1D_R2C::Complex>::const_iterator i =
			     freq.begin() + 1; i != freq.end(); ++i, ++k) {
			const float n = norm<float>(*i);
			float snorm = sqrt(n);
			m_mean_freq += k * snorm;
			sum_freq += snorm;
		}
		m_mean_freq /= sum_freq;
		m_mean_freq_valid = true;
	}
	return m_mean_freq;
}

float CSlopeStatisticsImpl::get_range() const
{
	if (!m_range_valid)
		evaluate_range();
	return m_range;
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

void CSlopeStatisticsImpl::evaluate_perfusion_peak() const
{
	float mean = accumulate(m_series.begin(), m_series.end(), 0.0) / m_series.size();

	vector<float> help(m_series.size());
	if (m_series[0] < mean)
		transform(m_series.begin(), m_series.end(), help.begin(), _1 - mean);
	else
		transform(m_series.begin(), m_series.end(), help.begin(), mean - _1);

	pair<position, position> minmax = minmax_element(help.begin(), help.end());
	position help_begin = help.begin();
	size_t peak_pos = distance(help_begin, minmax.second);
	m_perfusion_peak.first = peak_pos;
	m_perfusion_peak.second =  m_series[m_perfusion_peak.first];
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

NS_MIA_END
