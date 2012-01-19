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

#include <cmath>
#include <numeric>
#include <mia/internal/autotest.hh>

#include <mia/3d/transform/rigid.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;


CSplineKernelTestPath kernel_test_path; 

struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C3DInterpolatorFactory ipf; 
}; 


struct TranslateTransFixture : public ipfFixture {
	TranslateTransFixture():size(60, 80, 40),
				rtrans(size, ipf)
		{
			rtrans.translate(1.0, 2.0, 3.0);
		}
	C3DBounds size;
	C3DRigidTransformation rtrans;
};


BOOST_FIXTURE_TEST_CASE(basics_TranslateTransFixture, TranslateTransFixture)
{
	C3DFVector x(-2, -4, 2);
	BOOST_CHECK_EQUAL(rtrans.degrees_of_freedom(), 6u);
	BOOST_CHECK_EQUAL(C3DFVector(-1.0, -2.0, 5.0),  rtrans(x));
}

BOOST_FIXTURE_TEST_CASE(max_TranslateTransFixture, TranslateTransFixture)
{
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), sqrtf(14.0), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_TranslateTransFixture, TranslateTransFixture)
{
	rtrans.set_identity();

	C3DRigidTransformation::const_iterator ti = rtrans.begin();

	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}
}

BOOST_FIXTURE_TEST_CASE(derivative_TranslateTransFixture, TranslateTransFixture)
{
	C3DFMatrix d = 	rtrans.derivative_at(10,10,10);
	BOOST_CHECK_EQUAL(d.x.x, 1.0f);
	BOOST_CHECK_EQUAL(d.x.y, 0.0f);
	BOOST_CHECK_EQUAL(d.x.z, 0.0f);
	BOOST_CHECK_EQUAL(d.y.x, 0.0f);
	BOOST_CHECK_EQUAL(d.y.y, 1.0f);
	BOOST_CHECK_EQUAL(d.y.z, 0.0f);
	BOOST_CHECK_EQUAL(d.z.x, 0.0f);
	BOOST_CHECK_EQUAL(d.z.y, 0.0f);
	BOOST_CHECK_EQUAL(d.z.z, 1.0f);
}


BOOST_FIXTURE_TEST_CASE(test_rigid3d, ipfFixture)
{
	C3DRigidTransformation t1(C3DBounds(10,20,30), ipf); 

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 6u);

	C3DFVector x0(1.0f, 2.0f, -1.0f);

	C3DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	t1.translate(1.0f, 2.0f, 2.0);
	BOOST_CHECK_EQUAL(t1(x0), C3DFVector(2.0f, 4.0f, 1.0));

	const float pi_half = M_PI / 2.0; 

	{
		C3DRigidTransformation t_rot_xy(t1); 
		t_rot_xy.rotate(pi_half, 0.0, 0.0);
		C3DFVector yrx = t_rot_xy(x0);
		BOOST_CHECK_CLOSE(yrx.x ,-4.0, 0.1f);
		BOOST_CHECK_CLOSE(yrx.y , 2.0, 0.1f);
		BOOST_CHECK_CLOSE(yrx.z , 1.0, 0.1f);
	}

	{
		C3DRigidTransformation t_rot_xz(t1); 
		t_rot_xz.rotate(0.0, pi_half,  0.0);
		C3DFVector yry = t_rot_xz(x0);
		BOOST_CHECK_CLOSE(yry.x ,-1.0, 0.1f);
		BOOST_CHECK_CLOSE(yry.y , 4.0, 0.1f);
		BOOST_CHECK_CLOSE(yry.z , 2.0, 0.1f);
	}
	{
		C3DRigidTransformation t_rot_yz(t1); 
		t_rot_yz.rotate(0.0,  0.0, pi_half);
		C3DFVector yrz = t_rot_yz(x0);
		BOOST_CHECK_CLOSE(yrz.x , 2.0, 0.1f);
		BOOST_CHECK_CLOSE(yrz.y ,-1.0, 0.1f);
		BOOST_CHECK_CLOSE(yrz.z , 4.0, 0.1f);
	}


	C3DRigidTransformation t2(C3DBounds(10,20,30), ipf);
	t2.rotate(M_PI / 2.0, 0, 0);
	C3DFVector yr = t2(x0);
	BOOST_CHECK_CLOSE(yr.x ,  -2.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.y ,   1.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.z ,  -1.0f, 0.1f);
}

BOOST_FIXTURE_TEST_CASE( test_rigid3d_iterator, ipfFixture)
{
	C3DBounds size(10,20,5);

	C3DRigidTransformation t1(size, ipf);
	C3DRigidTransformation::const_iterator ti = t1.begin();

	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}

	BOOST_CHECK(ti == t1.end());
}


BOOST_FIXTURE_TEST_CASE( test_rigid3d_ranged_iterator, ipfFixture)
{
	C3DBounds size(10,20,30);
	C3DBounds delta(1,2,3); 

	C3DRigidTransformation t1(size, ipf);
	auto ti = t1.begin_range(delta, size - delta);

	for (size_t z = delta.z; z < size.z - delta.z; ++z)
		for (size_t y = delta.y; y < size.y - delta.y; ++y)
			for (size_t x = delta.x; x < size.x - delta.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}

	BOOST_CHECK(ti == t1.end_range(delta, size - delta));
}


#if 0 
struct RotateTransFixture {
	RotateTransFixture():
		size(60, 80, 30),
		rtrans(size),
		rot_cos(cos(M_PI / 4.0)),
		rot_sin(sin(M_PI / 4.0))
	{
		rtrans.rotate(M_PI / 4.0);

	}
	C3DBounds size;
	C3DRigidTransformation rtrans;

	float rot_cos;
	float rot_sin;
};



BOOST_FIXTURE_TEST_CASE(basics_RotateTransFixture, RotateTransFixture)
{
	C3DFVector x(33, 40);
	C3DFVector r  = rtrans(x);

	BOOST_CHECK_CLOSE(r.x, (rot_cos * x.x - rot_sin* x.y), 0.1);
	BOOST_CHECK_CLOSE(r.y, (rot_cos * x.x + rot_sin* x.y), 0.1);
}

BOOST_FIXTURE_TEST_CASE(max_RotateTransFixture, RotateTransFixture)
{
	C3DFVector x(60, 80);
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), (x - rtrans(x)).norm(), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_RotateTransFixture, RotateTransFixture)
{
	rtrans.set_identity();

	C3DRigidTransformation::const_iterator ti = rtrans.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y));
		}
}

BOOST_FIXTURE_TEST_CASE( test_rigid_clone, TranslateTransFixture )
{
	P3DTransformation clone(rtrans.clone());

	C3DRigidTransformation& spclone = dynamic_cast<C3DRigidTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), rtrans.get_size());

	C3DRigidTransformation::const_iterator ic = spclone.begin();
	C3DRigidTransformation::const_iterator ec = spclone.end();

	C3DRigidTransformation::const_iterator io = rtrans.begin();

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
	C3DFMatrix d = 	rtrans.derivative_at(10,10);
	BOOST_CHECK_CLOSE(d.x.x, rot_cos, 0.1);
	BOOST_CHECK_CLOSE(d.x.y, -rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.x, rot_sin, 0.1);
	BOOST_CHECK_CLOSE(d.y.y, rot_cos, 0.1);
}

struct RigidGrad2ParamFixtureRigid {
	RigidGrad2ParamFixtureRigid();


	C3DBounds size;
	C3DRigidTransformation trans;
};

BOOST_FIXTURE_TEST_CASE (test_upscale, RigidGrad2ParamFixtureRigid)
{
	C3DBounds x(4,4);
	P3DTransformation ups = trans.upscale(x);
	const C3DRigidTransformation& a = dynamic_cast<const C3DRigidTransformation&>(*ups);
	BOOST_CHECK_EQUAL(a.get_size(), x);

	auto params = a.get_parameters();


	// test the remaining parameters

}

RigidGrad2ParamFixtureRigid::RigidGrad2ParamFixtureRigid():
	size(80,80),
	trans(size)
{
	trans.translate(-1, -3);
//	trans.rotate(0.0);
}



BOOST_AUTO_TEST_CASE (test_inverse_rigid)
{
	C3DBounds size(10,2);
	C3DRigidTransformation trans(size);
	auto a = trans.get_parameters();
	a[0] = -1; 
	a[1] = -3;
	a[2] = 1.0; 
	trans.set_parameters(a);
	
	unique_ptr<C3DTransformation> inverse( trans.invert()); 
	
	auto b = inverse->get_parameters();
	const double ca = cos(1.0); 
 	const double sa = sin(1.0); 
	
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	BOOST_CHECK_EQUAL(b.size(), 3u);
	BOOST_CHECK_CLOSE(b[0], ca * 1 + sa * 3, 0.1);
	BOOST_CHECK_CLOSE(b[1],-sa * 1 + ca * 3, 0.1);
	BOOST_CHECK_EQUAL(b[2],-1.0);
	
}
#endif
