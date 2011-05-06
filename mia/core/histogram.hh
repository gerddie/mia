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

#ifndef mia_core_histogram_hh
#define mia_core_histogram_hh

#include <mia/core/defines.hh>

#include <cmath>
#include <cassert>
#include <vector>
#include <mia/core/defines.hh>
#include <mia/core/msgstream.hh>
#include <boost/type_traits.hpp>

/** \file histogram.hh This file defined some classes to handle simple histograms */ 

NS_MIA_BEGIN

/**
   \brief A class to normalize and quantizize input data to 
   a given histogram range with its given number of bins. 

   This class is used as a helpe class for simple histograms. 
   The class is responsible for scaling and quantizising the input values to 
   fit the histogram parameters. 
   \tparam the input data type to be fed into the instogram 
*/
template <typename T>
class THistogramFeeder {
public: 	
	/// typedef for generic programming 
	typedef T value_type; 

	/**
	   Initialize the histogram feeder for a histogram with values 
	   in [min,max] with the given number of bins. 
	   \param min 
	   \param max
	   \param bins 
	 */
	THistogramFeeder(T min, T max, size_t bins); 
	
	/// \returns the number of bins 
	size_t size() const; 

	/**
	   Evaluate the target bin of an input value  
	   \param x input value 
	   \returns the target bin index 
	   \remark the index is the nearest neighbor of the scaled input value 
	 */
	size_t index(T x) const; 

	/**
	   Evaluate the center value of a given bin in terms of the input data range 
	   \param k bin index 
	   \returns center value of bin 
	 */
	T value(size_t k) const; 
private: 
	T m_min;  
	T m_max; 
	size_t m_bins; 
	double m_step; 
	double m_inv_step; 
};

/**
   \brief specialization of the THistogramFeeder for unsigned byte input data 

   This specialization always uses the full range [0,256) of the input data 
   and 256 bins. 
   \remark is this really a good idea to specialize the histogram like this, 
   because so it is not possible to use a different number of bins for unsigned byte 
 */
template <>
class THistogramFeeder<unsigned char> {
public: 	
	/// typedef for generic programming 
	typedef unsigned char value_type; 

	/// Construct the feeder, the parameters are ignored 
	THistogramFeeder(unsigned char min, unsigned char max, size_t bins); 

	/// \returns 256 
	size_t size() const; 
	
	/// \returns x
	size_t index(unsigned char x) const; 

	/// \returns k 
	unsigned char value(size_t k) const; 
};

/// typedef for the unsigned byte histogram feeder specialization 
typedef THistogramFeeder<unsigned char> CUBHistogramFeeder; 

/**
   \brief a simple histogram that uses an instance of THistogramFeeder 
   as input converter

   This class implements a simple histogram that uses the nearest neighbor 
   approach implemeneted in THistogramFeeder to fill the histogram and provides 
   some funcionallity to work with the histogram. 
   \tparam the input feeder 
 */
template <typename Feeder>
class THistogram {
public: 

	/// STL iterator 
	typedef std::vector<size_t>::const_iterator const_iterator; 
	
	/// A type for the value-index pair \todo change to meaningful name 
	typedef std::pair<typename Feeder::value_type, size_t> value_type; 

	/**
	   Constructor to create the histogram with the given input feeder. 
	 */
	THistogram(Feeder f); 

	/**
	   Constructor to create a histogram by copying another histogram and 
	   cutting of part ot the upper tail. 
	   
	   \param org original histogram to copy from 
	   \param perc percentage of the bins to cut off 
	   \todo this should actually cut of a percentage of the data and not of the 
	   bins 
	 */
	THistogram(const THistogram<Feeder>& org, double perc); 
	
	/**
	   Add a value x to the histogram 
	   \param x 
	 */
	void push(typename Feeder::value_type x);

	/**
	   Add a value x to the histogram count times 
	   \param x 
	   \param count 
	 */
	void push(typename Feeder::value_type x, size_t count);

	/**
	   Add a range of data to the histogram 
	   \tparam Iterator forward iterator 
	   \param begin start of input range 
	   \param end end of input range (STL convention) 
	 */
	template <typename Iterator>
	void push_range(Iterator begin, Iterator end);

	/// \returns size of histogram 
	size_t size() const; 

	/// \returns start of histogram 
	const_iterator begin() const; 
	
	/// \returns end of histogram 
	const_iterator end() const; 
	
	/// \returns value of histogram bin at idx 
	size_t operator [] (size_t idx) const; 

	/** Return the count and input range value corresponding to 
	    the bin at idx 
	    \param idx
	    \returns <value,count> pair 
	*/ 
	const value_type at(size_t idx) const; 

	
	/// \returns median of the histogram 
	typename Feeder::value_type median() const; 

	/// \returns Median Average Distance  of the histogram 
	typename Feeder::value_type MAD() const; 

	/// \returns mean of the histogram 
	double average() const; 
	
	/// \returns deviation of the histogram 
	double deviation() const; 

private: 
	Feeder m_feeder; 
	std::vector<size_t> m_histogram; 
	size_t m_n; 
}; 

// inline inplementation 
template <typename T> 
THistogramFeeder<T>::THistogramFeeder(T min, T max, size_t bins):
	m_min(min), 
	m_max(max), 
	m_bins(bins), 
	m_step(( double(max) - double(min) ) / double(bins - 1)), 
	m_inv_step(double(bins - 1) / (double(max) - double(min)))
{
}

template <typename T> 
size_t THistogramFeeder<T>::size() const
{
	return m_bins; 
}

template <typename T> 
inline size_t THistogramFeeder<T>::index(T x) const
{
	double val = floor(m_inv_step * (x - m_min) + 0.5); 
	if (val < 0) 
		return 0; 
	if (val < m_bins) 
		return val; 
	return m_bins - 1; 
}

template <typename T> 
T THistogramFeeder<T>::value(size_t k) const
{
	return k * m_step + m_min; 
}

inline THistogramFeeder<unsigned char>::THistogramFeeder(unsigned char /*min*/, unsigned char /*max*/, size_t /*bins*/)
{
}

inline size_t THistogramFeeder<unsigned char>::size() const
{
	return 256; 
}

inline 
size_t THistogramFeeder<unsigned char>::index(unsigned char x) const
{
	return x; 
}

inline 
unsigned char THistogramFeeder<unsigned char>::value(size_t k) const
{
	return k; 
}

template <typename Feeder>
THistogram<Feeder>::THistogram(Feeder f):
	m_feeder(f), 
	m_histogram(f.size()), 
	m_n(0)
{
}

template <typename Feeder>
THistogram<Feeder>::THistogram(const THistogram<Feeder>& org, double perc):
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
size_t THistogram<Feeder>::size() const 
{
	return m_histogram.size(); 
}

template <typename Feeder> 
void THistogram<Feeder>::push(typename Feeder::value_type x)
{
	++m_n; 
	++m_histogram[m_feeder.index(x)];
}

template <typename Feeder> 
template <typename Iterator>
void THistogram<Feeder>::push_range(Iterator begin, Iterator end)
{
	while (begin != end) 
		push(*begin++); 
}



template <typename Feeder> 
void THistogram<Feeder>::push(typename Feeder::value_type x, size_t count)
{
	m_n += count; 
	m_histogram[m_feeder.index(x)] += count;
}

template <typename Feeder> 
typename THistogram<Feeder>::const_iterator THistogram<Feeder>::begin() const
{
	return m_histogram.begin(); 
}
	
template <typename Feeder> 
typename THistogram<Feeder>::const_iterator THistogram<Feeder>::end() const
{
	return m_histogram.end(); 
}

template <typename Feeder> 
size_t THistogram<Feeder>::operator [] (size_t idx) const
{
	assert(idx < m_histogram.size()); 
	return m_histogram[idx]; 
}

template <typename Feeder> 
typename Feeder::value_type THistogram<Feeder>::median() const
{
	float n_2 = m_n / 2.0f;
	float sum = 0; 
	size_t k = 0; 
	while ( sum < n_2 ) 
		sum +=  m_histogram[k++]; 

	return m_feeder.value(k-1); 
}

template <typename Feeder> 
typename Feeder::value_type THistogram<Feeder>::MAD() const
{
	typedef typename Feeder::value_type T; 
	T m = median(); 
	
	THistogram<Feeder> help(m_feeder); 
	
	; 
	for (size_t k = 0; k < size(); ++k) {
		T v = m_feeder.value(k); 
		help.push(v > m ? v - m : m -v, m_histogram[k]); 
	}
	return help.median(); 
}


template <typename Feeder> 
const typename THistogram<Feeder>::value_type THistogram<Feeder>::at(size_t idx) const
{
	if (idx < m_histogram.size())
		return value_type(m_feeder.value(idx), m_histogram[idx]); 
	else 
		return value_type(m_feeder.value(idx), 0); 
}

template <typename Feeder> 
double THistogram<Feeder>::average() const
{
	if (m_n < 1) 
		return 0.0;
	double sum = 0.0;
	for (size_t i = 0; i < size(); ++i) {
		const typename THistogram<Feeder>::value_type value = at(i); 
		sum += value.first * value.second;
	}
	return sum / m_n; 
}

template <typename Feeder> 
double THistogram<Feeder>::deviation() const
{
	if (m_n < 2) 
		return 0.0;
	double sum  = 0.0;
	double sum2 = 0.0;
	for (size_t i = 0; i < size(); ++i) {
		const typename THistogram<Feeder>::value_type value = at(i); 
		sum  += value.first * value.second;
		sum2 += value.first * value.first * value.second;
	}
	return sqrt((sum2 - sum * sum / m_n) / (m_n - 1)); 
}


NS_MIA_END

#endif


