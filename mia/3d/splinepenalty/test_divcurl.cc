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

#include <mia/3d/transformmock.hh>

#include <mia/internal/plugintester.hh>
#include <mia/3d/splinepenalty/divcurl.hh>
#include <mia/3d/interpolator.hh>

using namespace divcurl_splinepenalty; 
NS_MIA_USE
namespace bfs=::boost::filesystem;

/*
  The proper evaluation of the penalty and its gradient 
  is done in test_ppmatrix.cc. Here we only test the proper 
  initialization and transfer of data. 
*/

BOOST_AUTO_TEST_CASE( test_divcurl_cost_w2_d1_c1 ) 
{
	C3DBounds size(10,10,10); 
	C3DFVector range(32,32,32); 
	auto kernel = produce_spline_kernel("bspline:d=3"); 
	
	auto plugin = BOOST_TEST_create_from_plugin<C3DDivcurlSplinePenaltyPlugin>("divcurl:weight=2.0"); 
	
	plugin->initialize(size, range, kernel);

	C3DPPDivcurlMatrix test_matrix(size, range, *kernel, 1.0, 1.0); 
	
	C3DFVectorfield coefficients(size); 
	
	BOOST_CHECK_EQUAL(plugin->value(coefficients), 0.0); 
	
	coefficients(5,5,5) = C3DFVector(2,3,4); 

	BOOST_CHECK_EQUAL(plugin->value(coefficients), 2 * (test_matrix * coefficients));
	
	CDoubleVector plug_gradient(size.product() * 3); 
	CDoubleVector test_gradient(size.product() * 3); 

	BOOST_CHECK_EQUAL(plugin->value_and_gradient(coefficients, plug_gradient), 
			  2 * test_matrix.evaluate(coefficients,test_gradient)); 

	for (auto ipg = plug_gradient.begin(), itg = test_gradient.begin(); 
	     ipg != plug_gradient.end(); ++ipg, ++itg)
		BOOST_CHECK_CLOSE(*ipg, -2.0 * *itg, 0.1); 

	
}

