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

struct DeflationNonlinearityFixture {

	DeflationNonlinearityFixture(); 
	Matrix signal;
	DoubleVector w;

}; 

BOOST_FIXTURE_TEST_CASE( test_pow3_nonlinearity, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflPow3Plugin>("pow3");
	
	plugin->set_signal(&signal);
	plugin->apply(w); 
	
	BOOST_CHECK_CLOSE(w[0],  0.67748, 0.01); 
	BOOST_CHECK_CLOSE(w[1], -0.48505, 0.01); 
	BOOST_CHECK_CLOSE(w[2], -1.35082, 0.01); 
	BOOST_CHECK_CLOSE(w[3], -0.35110, 0.01); 
	
}

BOOST_FIXTURE_TEST_CASE( test_pow3_nonlinearity_stabelized, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflPow3Plugin>("pow3");
	
	plugin->set_signal(&signal);
	plugin->set_mu(0.1); 
	plugin->apply(w); 
	 
	BOOST_CHECK_CLOSE(w[0], -0.22633, 0.01); 
	BOOST_CHECK_CLOSE(w[1], 0.16169, 0.01); 
	BOOST_CHECK_CLOSE(w[2], 0.45097, 0.01); 
	BOOST_CHECK_CLOSE(w[3], 0.11686, 0.01); 
	
}

BOOST_FIXTURE_TEST_CASE( test_tanh_nonlinearity, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflTanhPlugin>("tanh:a=1.5");
		
	plugin->set_signal(&signal);
	plugin->apply(w); 
	 
	BOOST_CHECK_CLOSE(w[0],  0.26363, 0.01); 
	BOOST_CHECK_CLOSE(w[1], -0.22528, 0.01); 
	BOOST_CHECK_CLOSE(w[2], -0.57095, 0.01); 
	BOOST_CHECK_CLOSE(w[3], -0.17719, 0.01); 
	
}

BOOST_FIXTURE_TEST_CASE( test_tanh_nonlinearity_stabelized, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflTanhPlugin>("tanh:a=1.5");
		
	plugin->set_signal(&signal);
	plugin->set_mu(0.1); 
	plugin->apply(w); 

	BOOST_CHECK_CLOSE(w[0], -0.22299, 0.01); 
	BOOST_CHECK_CLOSE(w[1], 0.16196, 0.01); 
	BOOST_CHECK_CLOSE(w[2], 0.44761, 0.01); 
	BOOST_CHECK_CLOSE(w[3], 0.11809, 0.01); 
	
}


BOOST_FIXTURE_TEST_CASE( test_gauss_nonlinearity, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflGaussPlugin>("gauss:a=1.1");
	
	plugin->set_signal(&signal);
	plugin->apply(w); 
	   
	BOOST_CHECK_CLOSE(w[0],  0.17860, 0.01); 
	BOOST_CHECK_CLOSE(w[1], -0.15242, 0.01); 
	BOOST_CHECK_CLOSE(w[2], -0.38649, 0.01); 
	BOOST_CHECK_CLOSE(w[3], -0.11984, 0.01); 
	
}
BOOST_FIXTURE_TEST_CASE( test_gauss_nonlinearity_stabilized, DeflationNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflGaussPlugin>("gauss:a=1.1");
	
	plugin->set_signal(&signal);
	plugin->set_mu(0.1); 
	plugin->apply(w); 
	 
	BOOST_CHECK_CLOSE(w[0],  -0.22302, 0.01); 
	BOOST_CHECK_CLOSE(w[1],  0.16196, 0.01); 
	BOOST_CHECK_CLOSE(w[2],  0.44763, 0.01); 
	BOOST_CHECK_CLOSE(w[3],  0.11809, 0.01); 

}

const double init_signal[] = {
	-0.150788,  0.185673,  0.253528, -0.192714,  0.309594,  0.384079,  0.117799,  0.200267, -0.212151, -0.257026,
	0.221388, -0.166861,  0.128731, -0.207766, -0.041186, -0.089463, -0.151564,  0.249002, -0.060958,  0.223747,
	0.170328, -0.071203, -0.435380,  0.093816, -0.206290, -0.408690, -0.212328, -0.211139,  0.499941, -0.015164,
	-0.240928,  0.052391,  0.053121,  0.306664, -0.062118,  0.114074,  0.246093, -0.238130, -0.226832,  0.048444
};

DeflationNonlinearityFixture::DeflationNonlinearityFixture()
{
	double init_w[] = { -0.22638, 0.16168, 0.45102, 0.11684}; 

	signal = Matrix(4, 10, init_signal); 
	w = DoubleVector(4, false); 
	std::copy(init_w, init_w + 4, w.begin()); 


}

struct SymmetryNonlinearityFixture {

	SymmetryNonlinearityFixture(); 
	Matrix signal;
	Matrix W;

}; 


BOOST_FIXTURE_TEST_CASE( test_pow_nonlinearity_stabilized_Symm, SymmetryNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflPow3Plugin>("pow3");
	
	plugin->set_signal(&signal);
	plugin->apply(W); 
	 
	BOOST_CHECK_CLOSE(W(0,0), 1.24114, 0.01); 
	BOOST_CHECK_CLOSE(W(0,1), -2.34633, 0.01); 
	BOOST_CHECK_CLOSE(W(1,0), -0.89385, 0.01); 
	BOOST_CHECK_CLOSE(W(1,1), -1.22523, 0.01); 
	BOOST_CHECK_CLOSE(W(2,0), -2.47949, 0.01); 
	BOOST_CHECK_CLOSE(W(2,1), -0.39022, 0.01); 
	BOOST_CHECK_CLOSE(W(3,0), -0.64952, 0.01); 
	BOOST_CHECK_CLOSE(W(3,1), -1.35443, 0.01); 

}


BOOST_FIXTURE_TEST_CASE( test_tanh_nonlinearity_Symm, SymmetryNonlinearityFixture ) 
{
	auto plugin = BOOST_TEST_create_from_plugin<CFastICADeflTanhPlugin>("tanh:a=1.5");
	
	plugin->set_signal(&signal);
	plugin->apply(W); 
	    
	BOOST_CHECK_CLOSE(W(0,0), 0.42007, 0.01); 
	BOOST_CHECK_CLOSE(W(0,1), -1.05476, 0.01); 
	BOOST_CHECK_CLOSE(W(1,0),  -0.36337, 0.01); 
	BOOST_CHECK_CLOSE(W(1,1), -0.58123, 0.01); 
	BOOST_CHECK_CLOSE(W(2,0), -0.91679, 0.01); 
	BOOST_CHECK_CLOSE(W(2,1), -0.24641, 0.01); 
	BOOST_CHECK_CLOSE(W(3,0),  -0.28688, 0.01); 
	BOOST_CHECK_CLOSE(W(3,1), -0.62782, 0.01); 

}



SymmetryNonlinearityFixture::SymmetryNonlinearityFixture()
{
	double init_W[] = { -0.41718, 0.78261, 
			     0.29795, 0.40841,    
			     0.83116, 0.12941,   
			     0.21532, 0.45165}; 

	signal = Matrix(4, 10, init_signal); 
	W = Matrix(4, 2, init_W);
}


