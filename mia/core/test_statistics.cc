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


#include <mia/internal/autotest.hh>
#include <mia/core/statistics.hh>

NS_MIA_BEGIN
class CSTDVector {
public: 
	EPixelType get_pixel_type() const {
		return m_pt; 
	}
	virtual size_t size() const = 0; 
protected: 
	CSTDVector(EPixelType pt): m_pt(pt) {}
private: 
	EPixelType m_pt; 
}; 

template <typename T> 
class TSTDVector: public CSTDVector {
	
public: 
	TSTDVector(size_t size):
		CSTDVector((EPixelType)pixel_type<T>::value), 
		m_data(size){}; 
	
	size_t size() const {
		return m_data.size(); 
	} 
	typename std::vector<T>::const_iterator begin() const {
		return m_data.begin(); 
	}
	typename std::vector<T>::const_iterator end() const {
		return m_data.end(); 
	}

	typename std::vector<T>::iterator begin() {
		return m_data.begin(); 
	}
	typename std::vector<T>::iterator end() {
		return m_data.end(); 
	}

private: 
	std::vector<T> m_data; 
}; 

template <>
struct Binder<CSTDVector> {
	typedef __bind_all<TSTDVector> Derived;
};
NS_MIA_END

NS_MIA_USE; 


BOOST_AUTO_TEST_CASE( test_mean_var )
{
	const float test_data[5] = {
		1.0, 0.0, -1.0, 2.0, 3.0
	};
	TSTDVector<double> data(5); 
	copy(test_data, test_data + 5, data.begin()); 

	CSTDVector& help = data; 
	auto mean_var = mia::filter(FMeanVariance(), help); 
	BOOST_CHECK_CLOSE(mean_var.first, 1.0, 0.1);
	BOOST_CHECK_CLOSE(mean_var.second, 1.5811, 0.1);
}


BOOST_AUTO_TEST_CASE( test_median_MAD_1 )
{
	const float test_data[5] = {
		1.0, 0.0, -1.0, 2.0, 3.0
	};
	TSTDVector<double> data(5); 
	copy(test_data, test_data + 5, data.begin()); 

	CSTDVector& help = data; 
	auto median_MAD = mia::filter(FMedianMAD(), help); 
	BOOST_CHECK_EQUAL(median_MAD.first, 1.0);
	BOOST_CHECK_EQUAL(median_MAD.second, 1.0);
}

