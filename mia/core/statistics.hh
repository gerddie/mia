/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_core_statistics_hh
#define mia_core_statistics_hh

#include <cmath>
#include <mia/core/filter.hh>


NS_MIA_BEGIN

/**
   \ingroup misc
   \brief Functor to be called by mia::filter to evaluate mean and variance of 
   a series of data. 

*/
struct  FMeanVariance: public TFilter< std::pair<double, double> >  {

	/** result type of this functor 
	    - first = mean
	    - second = variance 
	*/
	typedef TFilter< std::pair<double, double> >::result_type result_type; 
	
	/**
	   \tparam T container type T holding the data to be analyzed. Must provide
	   the methods begin() and end() returning a forward_iterator; 
	   \param data the data to be anlyzed
	   \returns a std::pair first=mean, second=variance
	 */

	template <typename T> 
	result_type operator()( const T& data) const; 
}; 

/**
   \ingroup misc
   \brief Functor to be called by mia::filter to evaluate median and median average distance (MAD) of 
   a series of data. 
*/
struct  FMedianMAD: public TFilter< std::pair<double, double> >  {

	/** result type of this functor 
	    - first = median 
	    - second = MAD 
	*/
	typedef TFilter< std::pair<double, double> >::result_type result_type; 
	
	/**
	   \tparam T container type T holding the data to be analyzed. Must provide
	   the methods begin() and end() returning a forward_iterator; 
	   \param data the data to be anlyzed
	   \returns a std::pair first=median, second=MAD
	 */
	template <typename T> 
	result_type operator()( const T& data) const; 
private: 
	double median(std::vector<double>& buf)const; 
}; 



template <typename T> 
FMeanVariance::result_type FMeanVariance::operator()( const T& data) const
{
	double sum = 0.0; 
	double sum2  = 0.0; 
	double n = data.size(); 
	for (auto i = data.begin(); i != data.end(); ++i) {
		sum += *i; 
		sum2 += *i * *i; 
	}

	FMeanVariance::result_type result = {0.0, 0.0}; 
	
	if (n > 0) {
		result.first = 	sum / n;
		if (n > 1) 
			result.second = sqrt((sum2 - result.first * sum) / (n - 1));
	}
	return result; 
}

template <typename T> 
FMedianMAD::result_type FMedianMAD::operator()( const T& data) const
{
	std::vector<double> buffer(data.size()); 
	copy(data.begin(), data.end(), buffer.begin()); 

	FMedianMAD::result_type result; 
	result.first = median(buffer); 

	transform(buffer.begin(), buffer.end(), buffer.begin(), 
		  [&result](double x) {return fabs(x - result.first);});
	result.second = median(buffer); 
	return result; 
}

double FMedianMAD::median(std::vector<double>& buf)const
{
	if (buf.empty()) 
		return 0.0; 

	if (buf.size() & 1) {
		auto i = buf.begin() + (buf.size() - 1) / 2;
		std::nth_element(buf.begin(), i, buf.end());
		return *i;
	}else{
		auto i1 = buf.begin() + buf.size() / 2 - 1;
		auto i2 = buf.begin() + buf.size() / 2;
		std::nth_element(buf.begin(), i1, buf.end());
		std::nth_element(buf.begin(), i2, buf.end());
		return (*i1 + *i2) / 2.0;
	}
}

NS_MIA_END
#endif
