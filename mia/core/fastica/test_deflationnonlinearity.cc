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

#include <mia/internal/plugintester.hh>
#include <mia/core/fastica/deflationnonlinearity.hh>

using namespace fastica_deflnonlin; 
using namespace mia; 
using namespace gsl; 
using namespace std; 

struct NonlinearityFixture {

	NonlinearityFixture(); 
	Matrix signal;
	DoubleVector w;

}; 

BOOST_FIXTURE_TEST_CASE( test_pow3_nonlinearity, NonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflPow3Plugin>("pow3");
	
	plugin->set_signal(&signal);
	plugin->apply(w); 
	
	BOOST_CHECK_CLOSE(w[0],  0.67748, 0.1); 
	BOOST_CHECK_CLOSE(w[1], -0.48505, 0.1); 
	BOOST_CHECK_CLOSE(w[2], -1.35082, 0.1); 
	BOOST_CHECK_CLOSE(w[3], -0.35110, 0.1); 
	
}

BOOST_FIXTURE_TEST_CASE( test_tanh_nonlinearity, NonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflTanhPlugin>("tanh:a=1.5");
	
	plugin->set_signal(&signal);
	plugin->apply(w); 
	 
	BOOST_CHECK_CLOSE(w[0],  0.26363, 0.1); 
	BOOST_CHECK_CLOSE(w[1], -0.22528, 0.1); 
	BOOST_CHECK_CLOSE(w[2], -0.57095, 0.1); 
	BOOST_CHECK_CLOSE(w[3], -0.17719, 0.1); 
	
}

BOOST_FIXTURE_TEST_CASE( test_gauss_nonlinearity, NonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflGaussPlugin>("gauss:a=1.1");
	
	plugin->set_signal(&signal);
	plugin->apply(w); 
	   
	BOOST_CHECK_CLOSE(w[0],  0.17860, 0.1); 
	BOOST_CHECK_CLOSE(w[1], -0.15242, 0.1); 
	BOOST_CHECK_CLOSE(w[2], -0.38649, 0.1); 
	BOOST_CHECK_CLOSE(w[3], -0.11984, 0.1); 
	
}


NonlinearityFixture::NonlinearityFixture()
{
	const double init_signal[] = {
		-0.150788,  0.185673,  0.253528, -0.192714,  0.309594,  0.384079,  0.117799,  0.200267, -0.212151, -0.257026,
		0.221388, -0.166861,  0.128731, -0.207766, -0.041186, -0.089463, -0.151564,  0.249002, -0.060958,  0.223747,
		0.170328, -0.071203, -0.435380,  0.093816, -0.206290, -0.408690, -0.212328, -0.211139,  0.499941, -0.015164,
		-0.240928,  0.052391,  0.053121,  0.306664, -0.062118,  0.114074,  0.246093, -0.238130, -0.226832,  0.048444
	};
	double init_w[] = { -0.22638, 0.16168, 0.45102, 0.11684}; 

	signal = Matrix(4, 10, init_signal); 
	w = DoubleVector(4, false); 
	std::copy(init_w, init_w + 4, w.begin()); 


}

