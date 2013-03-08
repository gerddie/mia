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

#include <cmath>
#include <numeric>
#include <mia/internal/autotest.hh>

#include <mia/2d/transform/rigid.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

CSplineKernelTestPath kernel_test_path; 

struct TranslateTransFixture {
	TranslateTransFixture():size(60, 80),
				rtrans(size, C2DFVector::_0, C2DInterpolatorFactory("bspline:d=3", "mirror"))
		{
			rtrans.translate(1.0, 2.0);
		}
	C2DBounds size;
	C2DRigidTransformation rtrans;
};


BOOST_FIXTURE_TEST_CASE(basics_TranslateTransFixture, TranslateTransFixture)
{
	C2DFVector x(-2, -4);
	BOOST_CHECK_EQUAL(rtrans.degrees_of_freedom(), 3u);
	BOOST_CHECK_EQUAL(C2DFVector(-1.0, -2.0),  rtrans(x));
}

BOOST_FIXTURE_TEST_CASE(max_TranslateTransFixture, TranslateTransFixture)
{
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), sqrtf(5.0), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_TranslateTransFixture, TranslateTransFixture)
{
	rtrans.set_identity();

	C2DRigidTransformation::const_iterator ti = rtrans.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}
}

BOOST_FIXTURE_TEST_CASE(derivative_TranslateTransFixture, TranslateTransFixture)
{
	C2DFMatrix d = 	rtrans.derivative_at(10,10);
	BOOST_CHECK_EQUAL(d.x.x, 1.0f);
	BOOST_CHECK_EQUAL(d.x.y, 0.0f);
	BOOST_CHECK_EQUAL(d.y.x, 0.0f);
	BOOST_CHECK_EQUAL(d.y.y, 1.0f);
}


struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C2DInterpolatorFactory ipf; 
}; 

BOOST_FIXTURE_TEST_CASE(test_rigid2d, ipfFixture)
{
	C2DRigidTransformation t1(C2DBounds(10,20), C2DFVector::_0, ipf);

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 3u);

	C2DFVector x0(1.0f, 2.0f);

	C2DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	t1.translate(1.0f, 2.0f);
	BOOST_CHECK_EQUAL(t1(x0), C2DFVector(2.0f, 4.0f));

	t1.rotate(M_PI / 2.0);
	C2DFVector yr1 = t1(x0);
	BOOST_CHECK_CLOSE(yr1.x ,-4.0, 0.1f);
	BOOST_CHECK_CLOSE(yr1.y , 2.0, 0.1f);

	C2DRigidTransformation t2(C2DBounds(10,20), C2DFVector::_0, ipf);
	t2.rotate(M_PI / 2.0);
	C2DFVector yr = t2(x0);
	BOOST_CHECK_CLOSE(yr.x ,  -2.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.y ,   1.0f, 0.1f);
}

BOOST_FIXTURE_TEST_CASE( test_rigid2d_iterator, ipfFixture )
{
	C2DBounds size(10,20);

	C2DRigidTransformation t1(size, C2DFVector::_0, ipf);
	C2DRigidTransformation::const_iterator ti = t1.begin();

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
	C2DRigidTransformation rtrans;

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

	C2DRigidTransformation::const_iterator ti = rtrans.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}
}

BOOST_FIXTURE_TEST_CASE( test_rigid_clone, TranslateTransFixture )
{
	P2DTransformation clone(rtrans.clone());

	C2DRigidTransformation& spclone = dynamic_cast<C2DRigidTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), rtrans.get_size());

	C2DRigidTransformation::const_iterator ic = spclone.begin();
	C2DRigidTransformation::const_iterator ec = spclone.end();

	C2DRigidTransformation::const_iterator io = rtrans.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_curl, RotateTransFixture )
{
	BOOST_CHECK_CLOSE(rtrans.grad_curl()+ 1.0, 1.0, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_divergence, RotateTransFixture )
{
	BOOST_CHECK_CLOSE(rtrans.grad_divergence() + 1.0, 1.0, 0.1);
}

BOOST_FIXTURE_TEST_CASE(derivative_RotateTransFixture, RotateTransFixture)
{
	C2DFMatrix d = 	rtrans.derivative_at(10,10);
	BOOST_CHECK_CLOSE(d.x.x, rot_cos, 0.1);
	BOOST_CHECK_CLOSE(d.x.y, -rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.x, rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.y, rot_cos, 0.1);
}

struct RigidGrad2ParamFixtureRigid : public ipfFixture{
	RigidGrad2ParamFixtureRigid();


	C2DBounds size;
	C2DRigidTransformation trans;
};

BOOST_FIXTURE_TEST_CASE (test_upscale, RigidGrad2ParamFixtureRigid)
{
	C2DBounds x(4,4);
	P2DTransformation ups = trans.upscale(x);
	const C2DRigidTransformation& a = dynamic_cast<const C2DRigidTransformation&>(*ups);
	BOOST_CHECK_EQUAL(a.get_size(), x);

	auto params = a.get_parameters();


	// test the remaining parameters

}

RigidGrad2ParamFixtureRigid::RigidGrad2ParamFixtureRigid():
	size(80,80),
	trans(size, C2DFVector::_0, ipf)
{
	trans.translate(-1, -3);
//	trans.rotate(0.0);
}



BOOST_FIXTURE_TEST_CASE (test_inverse_rigid, ipfFixture)
{
	C2DBounds size(10,2);
	C2DRigidTransformation trans(size, C2DFVector::_0, ipf);
	auto a = trans.get_parameters();
	a[0] = -1; 
	a[1] = -3;
	a[2] = 1.0; 
	trans.set_parameters(a);
	
	unique_ptr<C2DTransformation> inverse( trans.invert()); 
	
	auto b = inverse->get_parameters();
	const double ca = cos(1.0); 
 	const double sa = sin(1.0); 
	
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	BOOST_CHECK_EQUAL(b.size(), 3u);
	BOOST_CHECK_CLOSE(b[0], ca * 1 + sa * 3, 0.1);
	BOOST_CHECK_CLOSE(b[1],-sa * 1 + ca * 3, 0.1);
	BOOST_CHECK_EQUAL(b[2],-1.0);

}

struct RigidCenteredFixture : public ipfFixture{
	RigidCenteredFixture();
	C2DBounds size;
	C2DRigidTransformation trans;
};

BOOST_FIXTURE_TEST_CASE (test_centered_rotation_rigid, RigidCenteredFixture)
{
	BOOST_CHECK_CLOSE(trans.get_max_transform(), 10.f * sqrtf(26.0f), 0.1);
		
	C2DFVector x(10,40); 
	
	auto y = trans(x); 
	
	BOOST_CHECK_CLOSE(y.x, 10.0f, 0.1); 
	BOOST_CHECK_CLOSE(y.y, 20.0f, 0.1); 

}

RigidCenteredFixture::RigidCenteredFixture():
	size(41,61),
	trans(size, C2DFVector::_0, M_PI / 2.0, C2DFVector(0.5, 0.5), ipf)
{
}
	
