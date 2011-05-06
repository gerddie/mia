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


#include <cmath>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/cstplan.hh>

NS_MIA_USE
using namespace std; 
using namespace boost; 

class C1DPlan: public TCSTPlan<vector<float> > {
public: 
	C1DPlan( fftwf_r2r_kind forward, std::vector<int> size); 
private: 
	virtual void do_execute(float *buffer) const;
}; 


C1DPlan::C1DPlan(fftwf_r2r_kind forward, std::vector<int> size): 
	TCSTPlan<vector<float> >(forward, size)
{
}

void C1DPlan::do_execute(float *buffer)const 
{
	BOOST_CHECK(buffer != NULL); 
}


BOOST_AUTO_TEST_CASE( test_cstplan_odd )
{
	const size_t s = 10; 
	std::vector<int> size(1, s);

	vector<float> data(s); 
	vector<float> out_data(s); 

	for(size_t i = 0; i < s; ++i)
		data[i] = sqrt(i + 1.0); 
	
	
	C1DPlan plan00(FFTW_RODFT00,size); 
	plan00.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
	
	C1DPlan plan10(FFTW_RODFT10, size); 
	plan10.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan01(FFTW_RODFT01, size); 
	plan01.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan11(FFTW_RODFT11, size); 
	plan11.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
}



BOOST_AUTO_TEST_CASE( test_cstplan_odd_2d )
{
	const size_t sx = 10; 
	const size_t sy = 20; 
	const size_t s = sy * sx; 
	std::vector<int> size(2);
	size[0] = sx; 
	size[1] = sy; 

	vector<float> data(s); 
	vector<float> out_data(s); 

	for(size_t i = 0; i < s; ++i)
		data[i] = sqrt(i + 1.0); 
	
	
	C1DPlan plan00(FFTW_RODFT00, size); 
	plan00.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
	
	C1DPlan plan10(FFTW_RODFT10, size); 
	plan10.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan01(FFTW_RODFT01, size); 
	plan01.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan11(FFTW_RODFT11, size); 
	plan11.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
}



BOOST_AUTO_TEST_CASE( test_cstplan_even )
{
	const size_t s = 10; 
	std::vector<int> size(1, s);

	vector<float> data(s); 
	vector<float> out_data(s); 

	for(size_t i = 0; i < s; ++i)
		data[i] = sqrt(i + 1.0); 
	

	C1DPlan plan00(FFTW_REDFT00,size); 
	plan00.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
	
	C1DPlan plan10(FFTW_REDFT10, size); 
	plan10.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan01(FFTW_REDFT01, size); 
	plan01.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 

	C1DPlan plan11(FFTW_REDFT11, size); 
	plan11.execute(data, out_data); 

	for(size_t i = 0; i < s; ++i)
		BOOST_CHECK_CLOSE(data[i], out_data[i], 0.01); 
}



