/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                    Gert Wollny <gw.fossdev @ gmail.com>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/2d/groundtruthproblem.hh>

using namespace mia; 
using gsl::DoubleVector; 

const size_t N = 4; 
const C2DBounds slice_size(3,3); 
const size_t psize = 4 * 9; 


struct GroundTruthAccess: public GroundTruthProblem {
	GroundTruthAccess(const DoubleVector& left_side,
			   const  CCorrelationEvaluator::result_type& corr); 
	void check_spacial_gradient(const double *result); 
	void check_time_derivative(const double *result); 
private: 
	void check_vector_equal(const vector<double>& result, const double *test); 
}; 


BOOST_AUTO_TEST_CASE( test_time_gradient ) 
{
	float hinit[6] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.0}; 
	float vinit[6] = {0.0, 1.0, 1.0, 0.5, 0.5, 1.0}; 
	float xinit[psize] = {
		0, 4, 0,  1, 1, 2,  3, 1, 1, 
		0, 3, 0,  2, 2, 3,  3, 1, 1, 
		0, 2, 0,  3, 3, 4,  4, 2, 1, 
		0, 1, 0,  4, 4, 4,  7, 1, 1
	}; 

	double time_derivative[psize] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,-1,-1, 0, 
		0, 0, 0, 0, 0, 1,-2, 2, 0
	}; 

	double space_derivative[psize] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,-1,-1, 0, 
		0, 0, 0, 0, 0, 1,-2, 2, 0
	}; 
	

	CCorrelationEvaluator::result_type corr; 
	corr.horizontal = C2DFImage(C2DBounds(2,3), hinit); 
	corr.vertical = C2DFImage(C2DBounds(3,2), vinit); 
	
	DoubleVector left_side(N*slice_size.x * slice_size.y); 

	DoubleVector x(psize); 
	copy(xinit, xinit + psize, x.begin()); 
	
	GroundTruthAccess gta(left_side, corr); 
	
	DoubleVector g(psize); 
	gta.df(x, &gta, g);
	gta.check_time_derivative(time_derivative); 
}



void GroundTruthAccess::check_vector_equal(const vector<double>& result, const double *test)
{
	BOOST_REQUIRE(psize == result.size()); 
	for(auto sg = result.begin(), t = test; sg !=result.end(); ++t, ++ sg)
	    BOOST_CHECK_CLOSE(*sg + 0.31, *t + 0.31, 0.1); 
}

GroundTruthAccess::GroundTruthAccess(const DoubleVector& left_side,
		   const  CCorrelationEvaluator::result_type& corr):
	GroundTruthProblem(1.0, 1.0, slice_size, N, left_side, corr)
{
}

void GroundTruthAccess::check_spacial_gradient(const double *test)
{
	check_vector_equal(get_spacial_gradient(), test); 
}

void GroundTruthAccess::check_time_derivative(const double *test)
{
	check_vector_equal(get_time_derivative(), test); 
}



