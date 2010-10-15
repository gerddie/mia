/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/2d/transform/affine.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct TranslateTransFixture {
	TranslateTransFixture():size(60, 80),
				rtrans(size)
		{
			rtrans.translate(1.0, 2.0);
		}
	C2DBounds size;
	C2DAffineTransformation rtrans;
};


BOOST_FIXTURE_TEST_CASE(basics_TranslateTransFixture, TranslateTransFixture)
{
	C2DFVector x(2, 4);
	BOOST_CHECK_EQUAL(rtrans.degrees_of_freedom(), 6);
	BOOST_CHECK_EQUAL(C2DFVector(3.0, 6.0),  rtrans(x));
}

BOOST_FIXTURE_TEST_CASE(max_TranslateTransFixture, TranslateTransFixture)
{
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), sqrtf(5.0), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_TranslateTransFixture, TranslateTransFixture)
{
	rtrans.set_identity();

	C2DAffineTransformation::const_iterator ti = rtrans.begin();

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


BOOST_AUTO_TEST_CASE(test_affine2d)
{
	C2DAffineTransformation t1(C2DBounds(10,20));

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 6);

	C2DFVector x0(1.0f, 2.0f);

	C2DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	t1.scale(std::log(2.0), std::log(3.0));

	C2DFVector y1 = t1(x0);
	BOOST_CHECK_EQUAL(y1, C2DFVector(2.0f, 6.0f));

	t1.translate(1.0f, 2.0f);
	BOOST_CHECK_EQUAL(t1(x0), C2DFVector(3.0f, 8.0f));


	t1.rotate(M_PI / 2.0);
	C2DFVector yr1 = t1(x0);
	BOOST_CHECK_CLOSE(yr1.x ,-8.0, 0.1f);
	BOOST_CHECK_CLOSE(yr1.y , 3.0, 0.1f);

	C2DAffineTransformation t2(C2DBounds(10,20));
	t2.rotate(M_PI / 2.0);
	C2DFVector yr = t2(x0);
	BOOST_CHECK_CLOSE(yr.x ,  -2.0f, 0.1f);
	BOOST_CHECK_CLOSE(yr.y ,   1.0f, 0.1f);

}


BOOST_AUTO_TEST_CASE( test_affine2d_iterator )
{
	C2DBounds size(10,20);

	C2DAffineTransformation t1(size);
	C2DAffineTransformation::const_iterator ti = t1.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}

	BOOST_CHECK(ti == t1.end());
}


struct RotateTransFixture {
	RotateTransFixture():
		size(60, 80),
		rtrans(size),
		rot_cos(cos(M_PI / 4.0)),
		rot_sin(sin(M_PI / 4.0))
	{
		rtrans.rotate(M_PI / 4.0);

	}
	C2DBounds size;
	C2DAffineTransformation rtrans;

	float rot_cos;
	float rot_sin;
};


BOOST_FIXTURE_TEST_CASE(basics_RotateTransFixture, RotateTransFixture)
{
	C2DFVector x(33, 40);
	C2DFVector r  = rtrans(x);

	BOOST_CHECK_CLOSE(r.x, (rot_cos * x.x - rot_sin* x.y), 0.1);
	BOOST_CHECK_CLOSE(r.y, (rot_cos * x.x + rot_sin* x.y), 0.1);
}

BOOST_FIXTURE_TEST_CASE(max_RotateTransFixture, RotateTransFixture)
{
	C2DFVector x(60, 80);
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), (x - rtrans(x)).norm(), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_RotateTransFixture, RotateTransFixture)
{
	rtrans.set_identity();

	C2DAffineTransformation::const_iterator ti = rtrans.begin();

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ti) {
			BOOST_CHECK_EQUAL(*ti, C2DFVector(x, y));
		}
}


BOOST_FIXTURE_TEST_CASE( test_affine_clone, TranslateTransFixture )
{
	P2DTransformation clone(rtrans.clone());

	C2DAffineTransformation& spclone = dynamic_cast<C2DAffineTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), rtrans.get_size());

	C2DTransformation::const_iterator ic = spclone.begin();
	C2DTransformation::const_iterator ec = spclone.end();

	C2DTransformation::const_iterator io = rtrans.begin();

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


struct AffineGrad2ParamFixtureAffine {
	AffineGrad2ParamFixtureAffine();


	C2DBounds size;
	C2DAffineTransformation trans;
};

BOOST_FIXTURE_TEST_CASE (test_grad2param_translation, AffineGrad2ParamFixtureAffine)
{
	C2DFVectorfield gradient(size);

	gradient(0,0) = C2DFVector(3.0, -4.0);
	gradient(1,0) = C2DFVector(1.0,  3.0);
	gradient(0,1) = C2DFVector(3.0, -2.0);
	gradient(1,1) = C2DFVector(1.0,  1.0);

	gsl::DoubleVector params = trans.get_parameters();


	trans.translate(gradient, params);

	BOOST_CHECK_CLOSE(params[0], 0.5, 0.1);
	BOOST_CHECK_CLOSE(params[1], 1.0, 0.1);
	BOOST_CHECK_CLOSE(params[2], 2.0, 0.1);

	BOOST_CHECK_CLOSE(params[3], 1.0, 0.1);
	BOOST_CHECK_CLOSE(params[4], -0.25, 0.1);
	BOOST_CHECK_CLOSE(params[5], -0.5, 0.1);
}

BOOST_FIXTURE_TEST_CASE (test_grad2param_translation2, AffineGrad2ParamFixtureAffine)
{
	C2DFVectorfield gradient(size);

	copy(trans.begin(), trans.end(), gradient.begin()); 

	gsl::DoubleVector params = trans.get_parameters();

	trans.translate(gradient, params);

	gsl::DoubleVector org_params = trans.get_parameters();
	BOOST_CHECK_CLOSE(params[0], org_params[0], 0.1);
	BOOST_CHECK_CLOSE(params[1], org_params[1], 0.1);
	BOOST_CHECK_CLOSE(params[2], org_params[2], 0.1);

	BOOST_CHECK_CLOSE(params[3], org_params[3], 0.1);
	BOOST_CHECK_CLOSE(params[4], org_params[4], 0.1);
	BOOST_CHECK_CLOSE(params[5], org_params[5], 0.1);
}

BOOST_FIXTURE_TEST_CASE (test_add, AffineGrad2ParamFixtureAffine)
{
	C2DFVector test = trans(C2DFVector(2,1));
	cvinfo() << test << "\n";
	test = trans(test);
	cvinfo() << test << "\n";
	trans.add(trans);

	C2DFVector probe = trans(C2DFVector(2,1));

	cvinfo() << probe << "\n";
	BOOST_CHECK_CLOSE(probe.x, test.x, 0.1);
	BOOST_CHECK_CLOSE(probe.y, test.y, 0.1);
}

BOOST_FIXTURE_TEST_CASE (test_upscale, AffineGrad2ParamFixtureAffine)
{
	C2DBounds x(4,4);
	P2DTransformation ups = trans.upscale(x);
	const C2DAffineTransformation& a = dynamic_cast<const C2DAffineTransformation&>(*ups);
	BOOST_CHECK_EQUAL(a.get_size(), x);

	auto params = a.get_parameters();


	// test the remaining parameters

}

AffineGrad2ParamFixtureAffine::AffineGrad2ParamFixtureAffine():
	size(2,2),
	trans(size)
{
	trans.translate(-1, -3);
	trans.rotate(0.0);
	trans.scale(0.69314718, -0.69314718);
}


BOOST_AUTO_TEST_CASE (test_invers)
{
	C2DBounds size(10,20); 
	C2DAffineTransformation trans(size); 

	auto params = trans.get_parameters(); 
//	BOOST_REQUIRE(params.size()== 6); 

	params[0] = 1.0; 
	params[1] = 2.0; 
	params[2] = 3.0; 

	params[3] = 1.0; 
	params[4] = 1.0; 
	params[5] = 4.0; 
	
	trans.set_parameters(params); 
	
	unique_ptr<C2DTransformation> inverse( trans.invert()); 
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	params = inverse->get_parameters(); 
	BOOST_REQUIRE(params.size()== 6); 

	BOOST_CHECK_CLOSE(params[0], -1.0, 0.1); 
	BOOST_CHECK_CLOSE(params[1],  2.0, 0.1); 
	BOOST_CHECK_CLOSE(params[2], -5.0, 0.1); 
	
	BOOST_CHECK_CLOSE(params[3],  1.0, 0.1); 
	BOOST_CHECK_CLOSE(params[4], -1.0, 0.1); 
	BOOST_CHECK_CLOSE(params[5],  1.0, 0.1); 
	
}

BOOST_AUTO_TEST_CASE (test_invers2)
{
	C2DBounds size(10,20); 
	C2DAffineTransformation trans(size); 

	auto params = trans.get_parameters(); 
//	BOOST_REQUIRE(params.size()== 6); 

	params[0] = 1.0; 
	params[1] = 2.0; 
	params[2] = 3.0; 

	params[3] = 3.0; 
	params[4] = 2.0; 
	params[5] = 2.0; 
	
	trans.set_parameters(params); 
	
	unique_ptr<C2DTransformation> inverse( trans.invert()); 
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	params = inverse->get_parameters(); 
	BOOST_REQUIRE(params.size()== 6); 

	BOOST_CHECK_CLOSE(params[0], -0.5, 0.1); 
	BOOST_CHECK_CLOSE(params[1],  0.5, 0.1); 
	BOOST_CHECK_CLOSE(params[2],  0.5, 0.1); 
	
	BOOST_CHECK_CLOSE(params[3],  0.75, 0.1); 
	BOOST_CHECK_CLOSE(params[4], -0.25, 0.1); 
	BOOST_CHECK_CLOSE(params[5], -1.75, 0.1); 
	
}
