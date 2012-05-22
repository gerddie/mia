/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/2d/transformmock.hh>

#include <mia/internal/plugintester.hh>
#include <mia/2d/splinepenalty/divcurl.hh>

using namespace divcurl_splinepenalty; 
NS_MIA_USE
namespace bfs=::boost::filesystem;

CSplineKernelTestPath kernel_test_path; 

BOOST_AUTO_TEST_CASE( test_divcurl_cost ) 
{
	C2DDivcurlSplinePenalty  penalty(4.0, 6.0, 1.0); 

	C2DBounds size(1,2); 
	C2DFVector range(2,3); 
	C2DFVectorfield coef(size); 
	coef(0,0) = C2DFVector(1,2); 
	coef(0,1) = C2DFVector(3,4);
	
	auto kernel = produce_spline_kernel("bspline:d=3"); 
	
	penalty.initialize(size, range, kernel); 
	CDoubleVector gradient(6, true); 
	
	BOOST_CHECK_EQUAL(penalty.value(coef), 5.0); 
	BOOST_CHECK_EQUAL(gradient[0], -2.0); 
	BOOST_CHECK_EQUAL(gradient[1], -3.0); 
	BOOST_CHECK_EQUAL(gradient[2], -2.0); 
	BOOST_CHECK_EQUAL(gradient[3], -3.0); 

}
