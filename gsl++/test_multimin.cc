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

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <vector>

#include <gsl++/multimin.hh>

using namespace gsl; 


class TestCFDFProblem : public CFDFMinimizer::Problem {
public:
	TestCFDFProblem(); 
private: 
	virtual double  do_f(const Vector& x); 
	virtual void    do_df(const Vector&  x, Vector&  g); 
	virtual double  do_fdf(const Vector&  x, Vector&  g); 
	std::vector<double> m_p; 
}; 

BOOST_AUTO_TEST_CASE(test_cfdf_multmin ) 
{
	CFDFMinimizer minimizer(CFDFMinimizer::PProblem(new TestCFDFProblem), gsl_multimin_fdfminimizer_conjugate_fr );
	
	Vector x(2, false); 
	const size_t i0 = 0; 
	const size_t i1 = 1; 

	x[i0] = 5.0; 
	x[i1] = 7.0; 
	
	BOOST_REQUIRE(minimizer.run(x)== GSL_SUCCESS); 
	BOOST_CHECK_CLOSE(x[i0], 1.0, 0.1); 
	BOOST_CHECK_CLOSE(x[i1], 2.0, 0.1); 
}

TestCFDFProblem::TestCFDFProblem():
	CFDFMinimizer::Problem(2), 
	m_p(5)
{
	double p_init[5] = {1.0, 2.0, 10.0, 20.0, 30.0 }; 
	copy(p_init, p_init+5, m_p.begin()); 
}
	

double  TestCFDFProblem::do_f(const Vector&  v)
{

	const size_t i0 = 0; 
	const size_t i1 = 1; 

	const double x = v[i0];
	const double y = v[i1];
	
	return m_p[2] * (x - m_p[0]) * (x - m_p[0]) +
		m_p[3] * (y - m_p[1]) * (y - m_p[1]) + m_p[4]; 
	
}

void    TestCFDFProblem::do_df(const Vector&  v, Vector&  g)
{
	const size_t i0 = 0; 
	const size_t i1 = 1; 

	const double x = v[i0];
	const double y = v[i1];
      
	g[i0] = 2.0 * m_p[2] * (x - m_p[0]);
	g[i1] = 2.0 * m_p[3] * (y - m_p[1]);
}

double  TestCFDFProblem::do_fdf(const Vector&  x, Vector&  g)
{
	do_df(x,g); 
	return do_f(x); 
}


class TestCFProblem : public CFMinimizer::Problem {
public:
	TestCFProblem(); 
private: 
	virtual double  do_f(const Vector& x); 
	std::vector<double> m_p; 
}; 


BOOST_AUTO_TEST_CASE(test_cf_multmin ) 
{
	CFMinimizer minimizer(CFMinimizer::PProblem(new TestCFProblem), gsl_multimin_fminimizer_nmsimplex );
	
	const size_t i0 = 0; 
	const size_t i1 = 1; 

	Vector x(2, false); 
	x[i0] = 5.0; 
	x[i1] = 7.0; 
	
	BOOST_REQUIRE(minimizer.run(x)== GSL_SUCCESS); 
	BOOST_CHECK_CLOSE(x[i0], 1.0, 1); 
	BOOST_CHECK_CLOSE(x[i1], 2.0, 1); 
}


TestCFProblem::TestCFProblem():
	CFMinimizer::Problem(2), 
	m_p(5)
{
	double p_init[5] = {1.0, 2.0, 10.0, 20.0, 30.0 }; 
	copy(p_init, p_init+5, m_p.begin()); 
}

double  TestCFProblem::do_f(const Vector&  v)
{

	const size_t i0 = 0; 
	const size_t i1 = 1; 

	const double x = v[i0];
	const double y = v[i1];
	
	return m_p[2] * (x - m_p[0]) * (x - m_p[0]) +
		m_p[3] * (y - m_p[1]) * (y - m_p[1]) + m_p[4]; 
	
}
