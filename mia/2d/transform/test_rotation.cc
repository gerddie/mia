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

#include <cmath>
#include <numeric>
#include <mia/internal/autotest.hh>
#include <mia/2d/transform/rotation.hh>

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
	C2DInterpolatorFactory ipf; 
}; 

BOOST_FIXTURE_TEST_CASE(test_rotation2d, ipfFixture)
{
	C2DRotationTransformation t1(C2DBounds(10,20), C2DFVector::_0, ipf);

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 1u);

	C2DFVector x0(1.0f, 2.0f);

	C2DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	C2DRotationTransformation t2(C2DBounds(10,20), C2DFVector::_0, ipf);
	t2.rotate(M_PI / 2.0);
	C2DFVector yr = t2(x0);
	BOOST_CHECK_CLOSE(yr.x ,  -2.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.y ,   1.0f, 0.1f);
}

BOOST_FIXTURE_TEST_CASE( test_rotation2d_iterator, ipfFixture )
{
	C2DBounds size(10,20);

	C2DRotationTransformation t1(size, C2DFVector::_0, ipf);
	C2DRotationTransformation::const_iterator ti = t1.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}

	BOOST_CHECK(ti == t1.end());
}


struct RotateTransFixture : public ipfFixture {
	RotateTransFixture():
		size(60, 80),
		rtrans(size, C2DFVector::_0, ipf),
		rot_cos(cos(M_PI / 3.0)),
		rot_sin(sin(M_PI / 3.0))
	{
		rtrans.rotate(M_PI / 3.0);

	}
	C2DBounds size;
	C2DRotationTransformation rtrans;

	float rot_cos;
	float rot_sin;
};


BOOST_FIXTURE_TEST_CASE(basics_RotateTransFixture, RotateTransFixture)
{
	C2DFVector x(33, 40);
	C2DFVector r  = rtrans(x);

	BOOST_CHECK_CLOSE(r.x, (rot_cos * x.x - rot_sin* x.y), 0.1);
	BOOST_CHECK_CLOSE(r.y, (rot_sin * x.x + rot_cos* x.y), 0.1);
}

BOOST_FIXTURE_TEST_CASE(max_RotateTransFixture, RotateTransFixture)
{
	C2DFVector x(59, 79);
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), (x - rtrans(x)).norm(), 0.1);
}


BOOST_FIXTURE_TEST_CASE(test_transform_derivative_at_gridpoint, RotateTransFixture)
{
	auto jac = rtrans.derivative_at(3, 4); 
	BOOST_CHECK_CLOSE(jac.x.x,  rot_cos, 0.1); 
	BOOST_CHECK_CLOSE(jac.y.y,  rot_cos, 0.1); 
	BOOST_CHECK_CLOSE(jac.x.y, -rot_sin, 0.1); 
	BOOST_CHECK_CLOSE(jac.y.x,  rot_sin, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_transform_derivative, RotateTransFixture)
{
	auto jac = rtrans.derivative_at(C2DFVector(3.1, 4.2));
	BOOST_CHECK_CLOSE(jac.x.x,  rot_cos, 0.1); 
	BOOST_CHECK_CLOSE(jac.y.y,  rot_cos, 0.1); 
	BOOST_CHECK_CLOSE(jac.x.y, -rot_sin, 0.1); 
	BOOST_CHECK_CLOSE(jac.y.x,  rot_sin, 0.1);
}


BOOST_FIXTURE_TEST_CASE(set_identity_RotateTransFixture, RotateTransFixture)
{
	rtrans.set_identity();

	C2DRotationTransformation::const_iterator ti = rtrans.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}
}

BOOST_FIXTURE_TEST_CASE(derivative_RotateTransFixture, RotateTransFixture)
{
	C2DFMatrix d = 	rtrans.derivative_at(10,10);
	BOOST_CHECK_CLOSE(d.x.x, rot_cos, 0.1);
	BOOST_CHECK_CLOSE(d.x.y, -rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.x, rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.y, rot_cos, 0.1);
}

BOOST_FIXTURE_TEST_CASE (test_upscale, ipfFixture)
{
	C2DBounds size(2,2);
	C2DRotationTransformation trans(size, 0.21, C2DFVector::_0, ipf); 
	C2DBounds x(4,4);
	P2DTransformation ups = trans.upscale(x);
	const C2DRotationTransformation& a = dynamic_cast<const C2DRotationTransformation&>(*ups);
	BOOST_CHECK_EQUAL(a.get_size(), x);

	auto params = a.get_parameters();

	BOOST_CHECK_EQUAL(params.size(), 1u); 
	// test the remaining parameters
	BOOST_CHECK_EQUAL(params[0], 0.21); 
}

BOOST_FIXTURE_TEST_CASE (test_inverse_rotation, ipfFixture)
{
	C2DBounds size(10,2);
	C2DRotationTransformation trans(size, C2DFVector::_0, ipf);
	auto a = trans.get_parameters();
	a[0] = -1; 
	trans.set_parameters(a);
	
	unique_ptr<C2DTransformation> inverse( trans.invert()); 
	
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	auto b = inverse->get_parameters(); 
	BOOST_CHECK_EQUAL(b.size(), 1u);
	BOOST_CHECK_EQUAL(b[0],1.0);

}

struct RotationCenteredFixture : public ipfFixture{
	RotationCenteredFixture();
	C2DBounds size;
	C2DRotationTransformation trans;
};

BOOST_FIXTURE_TEST_CASE (test_centered_rotation_rotation, RotationCenteredFixture)
{
	BOOST_CHECK_CLOSE(trans.get_max_transform(), 10.f * sqrtf(26.0f), 0.1);
		
	C2DFVector x(10,40); 
	
	auto y = trans(x); 
	
	BOOST_CHECK_CLOSE(y.x, 10.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.y, 20.0f, 0.1); 

}

RotationCenteredFixture::RotationCenteredFixture():
	size(41,61),
	trans(size, M_PI / 2.0, C2DFVector(0.5, 0.5), ipf)
{
}
	
