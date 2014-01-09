/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/3d/fullcost/divcurl.hh>
#include <mia/3d/transformmock.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;
PrepareTestPluginPath plugin_path_init; 

BOOST_AUTO_TEST_CASE( test_divcurl_cost ) 
{
	C3DDivCurlFullCost  div(4.0, 6.0, 1.0); 

	C3DBounds size(1, 2, 1); 
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	div.set_size(size); 

	CDoubleVector gradient(t.degrees_of_freedom()); 
	BOOST_CHECK_EQUAL(div.evaluate(t, gradient), 5.0); 
	BOOST_CHECK_EQUAL(gradient[0], -2.0); 
	BOOST_CHECK_EQUAL(gradient[1], -3.0); 
	BOOST_CHECK_EQUAL(gradient[2], -1.0);
}

BOOST_AUTO_TEST_CASE( test_div_cost ) 
{
	C3DDivCurlFullCost  div(4.0, 0.0, 0.5); 

	C3DBounds size(1, 2, 1); 
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	div.set_size(size); 

	CDoubleVector gradient(t.degrees_of_freedom()); 
	BOOST_CHECK_EQUAL(div.evaluate(t, gradient), 1.0); 
	BOOST_CHECK_EQUAL(gradient[0], -1.0); 
	BOOST_CHECK_EQUAL(gradient[1],  0.0); 
	BOOST_CHECK_EQUAL(gradient[2],  1.0);
}

BOOST_AUTO_TEST_CASE( test_curl_cost ) 
{
	C3DDivCurlFullCost  div(0.0, 3.0, 2.0); 

	C3DBounds size(1,2,1); 
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	div.set_size(size); 

	CDoubleVector gradient(t.degrees_of_freedom()); 
	BOOST_CHECK_EQUAL(div.evaluate(t, gradient), 3.0);
	BOOST_CHECK_EQUAL(gradient[0], 0.0);
	BOOST_CHECK_EQUAL(gradient[1], -3.0);
	BOOST_CHECK_EQUAL(gradient[2], -3.0);
}

BOOST_AUTO_TEST_CASE( test_curl_cost_notrans ) 
{
	C3DDivCurlFullCost  div(0.0, 4.0, 2.0); 

	C3DBounds size(1,2,1); 
	div.set_size(size); 

	BOOST_CHECK_EQUAL(div.cost_value(), 0.0); 
}
