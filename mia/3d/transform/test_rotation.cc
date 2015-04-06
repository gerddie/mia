/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <cmath>
#include <numeric>
#include <mia/internal/autotest.hh>

#include <mia/3d/transform/rotation.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C3DInterpolatorFactory ipf; 
}; 


BOOST_FIXTURE_TEST_CASE(test_rotation3d, ipfFixture)
{
	C3DRotationTransformation t1(C3DBounds(10,20,30), C3DFVector::_0, ipf); 

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 3u);

	C3DFVector x0(1.0f, 2.0f, -1.0f);

	C3DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	const float pi_half = M_PI / 2.0; 

	{
		C3DRotationTransformation t_rot_xy(t1); 
		t_rot_xy.rotate(pi_half, 0.0, 0.0);
		C3DFVector yrx = t_rot_xy(x0);
		BOOST_CHECK_CLOSE(yrx.x, -2.0, 0.1f);
		BOOST_CHECK_CLOSE(yrx.y,  1.0, 0.1f);
		BOOST_CHECK_CLOSE(yrx.z, -1.0, 0.1f);
	}

	{
		C3DRotationTransformation t_rot_xz(t1); 
		t_rot_xz.rotate(0.0, pi_half,  0.0);
		C3DFVector yry = t_rot_xz(x0);
		BOOST_CHECK_CLOSE(yry.x , 1.0, 0.1f);
		BOOST_CHECK_CLOSE(yry.y , 2.0, 0.1f);
		BOOST_CHECK_CLOSE(yry.z , 1.0, 0.1f);
	}
	{
		C3DRotationTransformation t_rot_yz(t1); 
		t_rot_yz.rotate(0.0,  0.0, pi_half);
		C3DFVector yrz = t_rot_yz(x0);
		BOOST_CHECK_CLOSE(yrz.x , 1.0, 0.1f);
		BOOST_CHECK_CLOSE(yrz.y , 1.0, 0.1f);
		BOOST_CHECK_CLOSE(yrz.z , 2.0, 0.1f);
	}


	C3DRotationTransformation t2(C3DBounds(10,20,30), C3DFVector::_0, ipf);
	t2.rotate(M_PI / 2.0, 0, 0);
	C3DFVector yr = t2(x0);
	BOOST_CHECK_CLOSE(yr.x ,  -2.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.y ,   1.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.z ,  -1.0f, 0.1f);
}

BOOST_FIXTURE_TEST_CASE( test_rotation3d_iterator, ipfFixture)
{
	C3DBounds size(10,20,5);

	C3DRotationTransformation t1(size, C3DFVector::_0, ipf);
	C3DRotationTransformation::const_iterator ti = t1.begin();

	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}

	BOOST_CHECK(ti == t1.end());
}

BOOST_FIXTURE_TEST_CASE( test_translate_gradient, ipfFixture)
{
	C3DBounds size(2,2,2); 
	C3DFVectorfield field(size); 

	field(0,0,0) = C3DFVector(1,1,1); 
	field(0,1,0) = C3DFVector(2,1,1); 
	field(1,0,0) = C3DFVector(2,0,0); 
	field(1,1,0) = C3DFVector(2,0,2);

	field(0,0,1) = C3DFVector(1,3,1); 
	field(0,1,1) = C3DFVector(2,4,2); 
	field(1,0,1) = C3DFVector(2,3,2); 
	field(1,1,1) = C3DFVector(2,6,3);
	
	
	C3DRotationTransformation t1(size, C3DFVector::_0, ipf);

	
	

}

BOOST_FIXTURE_TEST_CASE( test_rotation3d_ranged_iterator, ipfFixture)
{
	C3DBounds size(10,20,30);
	C3DBounds delta(1,2,3); 

	C3DRotationTransformation t1(size, C3DFVector::_0, ipf);
	auto ti = t1.begin_range(delta, size - delta);

	for (size_t z = delta.z; z < size.z - delta.z; ++z)
		for (size_t y = delta.y; y < size.y - delta.y; ++y)
			for (size_t x = delta.x; x < size.x - delta.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}

	BOOST_CHECK(ti == t1.end_range(delta, size - delta));
}

BOOST_FIXTURE_TEST_CASE( test_rotation3d_rotxcentered_basic, ipfFixture)
{
	C3DBounds size(61, 81, 41);
	C3DRotationTransformation rcrot(size, 
				    C3DFVector(M_PI * 0.5, 0.0, 0.0),
				    C3DFVector(0.5,0.5,0.5), ipf);


	BOOST_CHECK_CLOSE(rcrot.get_max_transform(), 20.f * sqrtf(10.0f), 0.1);

	C3DFVector x(20,40,30); 
	
	auto y = rcrot(x); 
	
	BOOST_CHECK_CLOSE(y.x, 20.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.y, 30.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.z, 20.0f, 0.1); 
}

BOOST_FIXTURE_TEST_CASE( test_rotation3d_rotzcentered_basic, ipfFixture)
{

	C3DBounds size(61, 81, 41); 
	C3DRotationTransformation rcrot(size, 
				     C3DFVector(0.0, 0.0, M_PI * 0.5),
				     C3DFVector(0.5,0.5,0.5), ipf); 
		
	BOOST_CHECK_CLOSE(rcrot.get_max_transform(), 10.f * sqrtf(50.0f), 0.1);

	C3DFVector x(20,40,30);
	
	auto y = rcrot(x);
	
	BOOST_CHECK_CLOSE(y.x, 30.0f, 0.1);
	BOOST_CHECK_CLOSE(y.y, 30.0f, 0.1);
	BOOST_CHECK_CLOSE(y.z, 30.0f, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_rotation3d_rotxcentered_translate_field, ipfFixture)
{
	C3DBounds size(31, 21, 21);
	C3DRotationTransformation rcrot(size, 
				     C3DFVector(0.02, 0.0, 0.0),
				     C3DFVector(0.5,0.5,0.5), ipf);

	C3DFVectorfield field(size); 
	auto ifield = field.begin_range(C3DBounds::_0, size); 
	for (auto ir = rcrot.begin(); ir != rcrot.end(); ++ir, ++ifield) {
		*ifield = *ir - C3DFVector(ifield.pos()); 
	}
	CDoubleVector grad(rcrot.degrees_of_freedom());

	rcrot.translate(field, grad); 
	
	// 32340  = 2 sum_x,y[-10,10]x[-10,10] ( x^2 + y^2 ) 
	BOOST_CHECK_CLOSE(grad[0], sin(0.02) * 31 * 32340, 0.1 ); 
	BOOST_CHECK_SMALL(grad[1], 1e-2); 
	BOOST_CHECK_SMALL(grad[2], 1e-2); 
}

BOOST_FIXTURE_TEST_CASE( test_rotation3d_rotycentered_basic, ipfFixture)
{
	C3DBounds size(61, 81, 41);
	C3DRotationTransformation rcrot(size, 
				    C3DFVector(0.0, M_PI * 0.5, 0.0),
				    C3DFVector(0.5,0.5,0.5), ipf);


	BOOST_CHECK_CLOSE(rcrot.get_max_transform(), 10.f * sqrtf(26.0f), 0.1);

	C3DFVector x(20,40,30); 
	
	auto y = rcrot(x); 
	
	BOOST_CHECK_CLOSE(y.x, 20.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.y, 40.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.z, 10.0f, 0.1); 
	
	
}

