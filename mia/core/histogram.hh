/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifndef mia_core_histogram_hh
#define mia_core_histogram_hh

#ifndef VSTREAM_DOMAIN
#define VSTREAM_DOMAIN "CORE-HISTOGRAM"
#endif

#include <mia/core/defines.hh>

#include <cmath>
#include <cassert>
#include <vector>
#include <mia/core/defines.hh>
#include <mia/core/msgstream.hh>
#include <boost/type_traits.hpp>

NS_MIA_BEGIN

template <typename T>
class CHistogramFeeder {
public: 	
	typedef T value_type; 
	CHistogramFeeder(T min, T max, size_t bins); 
	size_t size() const; 
	size_t index(T x) const; 
	T value(size_t k) const; 
private: 
	T m_min;  
	T m_max; 
	size_t m_bins; 
	double m_step; 
	double m_inv_step; 
};

template <>
class CHistogramFeeder<unsigned char> {
public: 	
	typedef unsigned char value_type; 
	CHistogramFeeder(unsigned char min, unsigned char max, size_t bins); 
	size_t size() const; 
	size_t index(unsigned char x) const; 
	unsigned char value(size_t k) const; 
};


typedef CHistogramFeeder<unsigned char> CUBHistogramFeeder; 

template <typename Feeder>
class CHistogram {
public: 
	typedef std::vector<size_t>::const_iterator const_iterator; 
	typedef std::pair<typename Feeder::value_type, size_t> value_type; 

	CHistogram(Feeder f); 

	CHistogram(const CHistogram<Feeder>& org, double perc); 
	
	void push(typename Feeder::value_type x);
	
	void push(typename Feeder::value_type x, size_t count);

	template <typename Iterator>
	void push_range(Iterator begin, Iterator end);
	
	size_t size() const; 
	
	const_iterator begin() const; 
	
	const_iterator end() const; 
	
	size_t operator [] (size_t idx) const; 

	const value_type at(size_t idx) const; 

	typename Feeder::value_type median() const; 
	typename Feeder::value_type MAD() const; 
	double average() const; 
	double deviation() const; 

	double average(double cut_percentage) const; 

private: 
	Feeder m_feeder; 
	std::vector<size_t> m_histogram; 
	size_t m_n; 
}; 

// inline inplementation 
template <typename T> 
CHistogramFeeder<T>::CHistogramFeeder(T min, T max, size_t bins):
	m_min(min), 
	m_max(max), 
	m_bins(bins), 
	m_step(( double(max) - double(min) ) / double(bins - 1)), 
	m_inv_step(double(bins - 1) / (double(max) - double(min)))
{
}

template <typename T> 
size_t CHistogramFeeder<T>::size() const
{
	return m_bins; 
}

template <typename T> 
inline size_t CHistogramFeeder<T>::index(T x) const
{
	double val = floor(m_inv_step * (x - m_min) + 0.5); 
	if (val < 0) 
		return 0; 
	if (val < m_bins) 
		return val; 
	return m_bins - 1; 
}

template <typename T> 
T CHistogramFeeder<T>::value(size_t k) const
{
	return k * m_step + m_min; 
}

inline CHistogramFeeder<unsigned char>::CHistogramFeeder(unsigned char /*min*/, unsigned char /*max*/, size_t /*bins*/)
{
}

inline size_t CHistogramFeeder<unsigned char>::size() const
{
	return 256; 
}

inline 
size_t CHistogramFeeder<unsigned char>::index(unsigned char x) const
{
	return x; 
}

inline 
unsigned char CHistogramFeeder<unsigned char>::value(size_t k) const
{
	return k; 
}

template <typename Feeder>
CHistogram<Feeder>::CHistogram(Feeder f):
	m_feeder(f), 
	m_histogram(f.size()), 
	m_n(0)
{
}

template <typename Feeder>
CHistogram<Feeder>::CHistogram(const CHistogram<Feeder>& org, double perc):
	m_feeder(org.m_feeder), 
	m_histogram(m_feeder.size()), 
	m_n(0)
{
	size_t n = (size_t)(org.m_n * (1.0 - perc)); 
		
	size_t i = 0;
	while (n > m_n && i < m_histogram.size()) {
		m_n += org.m_histogram[i]; 
		m_histogram[i] = org.m_histogram[i]; 
		++i; 
	}
}


template <typename Feeder> 
size_t CHistogram<Feeder>::size() const 
{
	return m_histogram.size(); 
}

template <typename Feeder> 
void CHistogram<Feeder>::push(typename Feeder::value_type x)
{
	++m_n; 
	++m_histogram[m_feeder.index(x)];
}

template <typename Feeder> 
template <typename Iterator>
void CHistogram<Feeder>::push_range(Iterator begin, Iterator end)
{
	while (begin != end) 
		push(*begin++); 
}



template <typename Feeder> 
void CHistogram<Feeder>::push(typename Feeder::value_type x, size_t count)
{
	m_n += count; 
	m_histogram[m_feeder.index(x)] += count;
}

template <typename Feeder> 
typename CHistogram<Feeder>::const_iterator CHistogram<Feeder>::begin() const
{
	return m_histogram.begin(); 
}
	
template <typename Feeder> 
typename CHistogram<Feeder>::const_iterator CHistogram<Feeder>::end() const
{
	return m_histogram.end(); 
}

template <typename Feeder> 
size_t CHistogram<Feeder>::operator [] (size_t idx) const
{
	assert(idx < m_histogram.size()); 
	return m_histogram[idx]; 
}

template <typename Feeder> 
typename Feeder::value_type CHistogram<Feeder>::median() const
{
	float n_2 = m_n / 2.0f;
	float sum = 0; 
	size_t k = 0; 
	while ( sum < n_2 ) 
		sum +=  m_histogram[k++]; 

	return m_feeder.value(k-1); 
}

template <typename Feeder> 
typename Feeder::value_type CHistogram<Feeder>::MAD() const
{
	typedef typename Feeder::value_type T; 
	T m = median(); 
	
	CHistogram<Feeder> help(m_feeder); 
	
	; 
	for (size_t k = 0; k < size(); ++k) {
		T v = m_feeder.value(k); 
		help.push(v > m ? v - m : m -v, m_histogram[k]); 
	}
	return help.median(); 
}


template <typename Feeder> 
const typename CHistogram<Feeder>::value_type CHistogram<Feeder>::at(size_t idx) const
{
	if (idx < m_histogram.size())
		return value_type(m_feeder.value(idx), m_histogram[idx]); 
	else 
		return value_type(m_feeder.value(idx), 0); 
}

template <typename Feeder> 
double CHistogram<Feeder>::average() const
{
	if (m_n < 1) 
		return 0.0;
	double sum = 0.0;
	for (size_t i = 0; i < size(); ++i) {
		const typename CHistogram<Feeder>::value_type value = at(i); 
		sum += value.first * value.second;
	}
	return sum / m_n; 
}

template <typename Feeder> 
double CHistogram<Feeder>::deviation() const
{
	if (m_n < 2) 
		return 0.0;
	double sum  = 0.0;
	double sum2 = 0.0;
	for (size_t i = 0; i < size(); ++i) {
		const typename CHistogram<Feeder>::value_type value = at(i); 
		sum  += value.first * value.second;
		sum2 += value.first * value.first * value.second;
	}
	return sqrt((sum2 - sum * sum / m_n) / (m_n - 1)); 
}


NS_MIA_END

#endif


