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


#include <mia/2d/correlation_weight.hh>
//#include <mia/2d/pseudo_ground_thruth.hh>



struct PseudoGroundTruthFixture {
	PseudoGroundTruthFixture(); 

	C2DBounds size; 
	size_t N;
	vector<P2DImage> input_series; 
}; 

BOOST_FIXTURE_TEST_CASE( test_correlation, PseudoGroundTruthFixture ) 
{
	CCorrelationEvaluator ce(0.3); 
	auto result = ce(input_series); 
	
	BOOST_CHECK_EQUAL(result.horizontal.get_size().x, 2); 
	BOOST_CHECK_EQUAL(result.horizontal.get_size().y, 3); 
	BOOST_CHECK_EQUAL(result.vertical.get_size().x, 3); 
	BOOST_CHECK_EQUAL(result.vertical.get_size().y, 2); 


	BOOST_CHECK_CLOSE(result.horizontal(0,1), 1.0, 0.1); 
	BOOST_CHECK_CLOSE(result.horizontal(0,2)+1, 1.0, 0.1); 
	
	// negative 
	BOOST_CHECK_CLOSE(result.horizontal(1,1), 0.94388, 0.1); 
	
	// negative 	
	BOOST_CHECK_CLOSE(result.vertical(1,0) +1 ,  1,0.1); 

	// positive but below threshold
	BOOST_CHECK_CLOSE(result.vertical(1,1)+1,   1.0,0.1); 


	BOOST_CHECK_CLOSE(result.vertical(0,1),   0.88659,0.1); 

}; 

#if 0
BOOST_FIXTURE_TEST_CASE( test_pseudo_ground_truth, PseudoGroundTruthFixture ) 
{
	const double alpha = 1.0; 
	const double beta = 1.0; 
	const double rho_th = 1.0; 
	
	CGroundThruthEvaluator gte(alpha, beta, rho_th); 
	vector<P2DImage> pgt = gte(work_images); 
	
}
#endif

PseudoGroundTruthFixture::PseudoGroundTruthFixture():
	size(3,3),
	N(4)
{
	const float init[4][3*3] = {
		{ 0, 4, 0,  1, 1, 2,  3, 1, 0}, 
		{ 0, 3, 0,  2, 2, 3,  3, 1, 0}, 
		{ 0, 2, 0,  3, 3, 4,  4, 2, 0}, 
		{ 0, 1, 0,  4, 4, 4,  7, 1, 0}
	}; 
	for (size_t i = 0; i < N; ++i) 
		input_series.push_back(P2DImage(new C2DFImage(size, init[i]))); 

	
}
