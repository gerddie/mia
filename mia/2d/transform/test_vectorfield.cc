/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/core/spacial_kernel.hh>
#include <mia/2d/transform/vectorfield.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;



struct GridTransformFixture {
	GridTransformFixture():
		size(256, 128),
		r(size.x - 1,size.y - 1),
		field(size), 
		scale(2 * M_PI / r.x, 2 * M_PI / r.y)
	{
		C2DGridTransformation::field_iterator i = field.field_begin();
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				*i = C2DFVector( fx(x, y), fy(x, y));
			}
	}
	C2DBounds size;
	C2DBounds r;
	C2DGridTransformation field;
protected:
	float fx(float x, float y);
	float fy(float x, float y);

	float dfx_x(float x, float y);
	float dfy_x(float x, float y);
	float dfx_y(float x, float y);
	float dfy_y(float x, float y);

	float dfx_xx(float x, float y);
	float dfx_xy(float x, float y);
	float dfx_yx(float x, float y);
	float dfx_yy(float x, float y);
	float dfy_xx(float x, float y);
	float dfy_xy(float x, float y);
	float dfy_yx(float x, float y);
	float dfy_yy(float x, float y);

	C2DFVector scale; 
};


BOOST_FIXTURE_TEST_CASE(test_gridtransform_basic_props, GridTransformFixture)
{
	BOOST_CHECK_EQUAL(field.degrees_of_freedom(), size.x * size.y * 2);

	C2DFVector testx(20.2, 21.9);
	C2DFVector result = field.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y), 1);
	BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y), 1);

	result = field(testx);

	BOOST_CHECK_CLOSE(result.x, testx.x - fx(testx.x, testx.y), 0.1);
	BOOST_CHECK_CLOSE(result.y, testx.y - fy(testx.x, testx.y), 0.1);

}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative, GridTransformFixture)
{


	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x) {
			C2DFMatrix dv =  field.derivative_at(x, y);
			if ( x > 0 && x < size.x- 1) {
				BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x, y), 1);
				BOOST_CHECK_CLOSE(dv.x.y, -dfy_x(x, y), 1);
			}else {
				BOOST_CHECK_EQUAL(dv.x.x, 1.0f);
				BOOST_CHECK_EQUAL(dv.x.y, 0);
			}
			if ( y > 0 && y < size.y - 1) {
				BOOST_CHECK_CLOSE(dv.y.x, -dfx_y(x, y), 1);
				BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x, y), 1);
			}else {
				BOOST_CHECK_EQUAL(dv.y.x, 0);
				BOOST_CHECK_EQUAL(dv.y.y, 1.0f);
			}
		}

}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_set_identity, GridTransformFixture)
{
	field.set_identity();
	C2DGridTransformation::const_iterator i = field.begin();
	for(size_t y = 0; y < size.y; ++y)
		for(size_t x = 0; x < size.x; ++x, ++i) {
			BOOST_CHECK_EQUAL( *i, C2DFVector(x,y));
		}

}


BOOST_AUTO_TEST_CASE( test_grid2d_iterator )
{
	C2DBounds size(5,10);
	C2DGridTransformation gt(size);

	C2DGridTransformation::field_iterator i = gt.field_begin();
	C2DFVector value(0.12, 0.32);

	while (i != gt.field_end()) {
		*i = value;
		value.x += 0.12f;
		value.y += 0.32f;
		++i;
	}

	C2DGridTransformation::const_iterator k = gt.begin();

	float n = 1.0f;
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++k, n += 1.0f) {
			BOOST_CHECK_CLOSE( (*k).x , (float)x - n * 0.12f, 0.1f );
			BOOST_CHECK_CLOSE( (*k).y , (float)y - n * 0.32f, 0.1f );

		}
}

BOOST_AUTO_TEST_CASE( test_gridtransform_add )
{
	C2DFVector init_a[9] = {
		C2DFVector(2,3), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(-1,-1), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0)
	};

	C2DFVector init_b[9] = {
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(1,1), C2DFVector(0.5,0),
		C2DFVector(0,0), C2DFVector(0,0.5), C2DFVector(4,5)
	};


	C2DBounds size(3,3);
	C2DGridTransformation a(size);
	std::copy(init_a, init_a + 9, a.field_begin());

	C2DGridTransformation b(size);
	std::copy(init_b, init_b + 9, b.field_begin());

	C2DGridTransformation c = a + b;

	BOOST_CHECK_EQUAL( c.apply(C2DFVector(1,1)), C2DFVector(3,4));
	BOOST_CHECK_EQUAL( c.apply(C2DFVector(2,1)), C2DFVector(0.0,-0.5));
	BOOST_CHECK_EQUAL( c.apply(C2DFVector(1,2)), C2DFVector(-0.5,0.0));

}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_max, GridTransformFixture )
{
	float fx0 = fx(0.75 * size.x,size.y / 2);
	float fy0 = fy(0.75 * size.x,size.y / 2);

	BOOST_CHECK_CLOSE(sqrt(fx0*fx0 + fy0*fy0), field.get_max_transform(),1);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_pertuberate, GridTransformFixture )
{
	C2DFVectorfield v(size);
	C2DFVector vv(10.0, 20.0);
	fill(v.begin(), v.end(), vv);

	float gamma = field.pertuberate(v);

	C2DFVector lmg(184, 107);
	C2DFVector mg(vv.x - vv.x * dfx_x(lmg.x,lmg.y) - vv.y * dfx_y(lmg.x,lmg.y),
		      vv.y - vv.x * dfy_x(lmg.x,lmg.y) - vv.y * dfy_y(lmg.x,lmg.y));
	BOOST_CHECK_CLOSE(gamma, mg.norm(), 0.1);

	for (size_t y = 1; y < size.y - 1; ++y)
		for (size_t x = 1; x < size.x - 1; ++x) {
			const C2DFVector& iv = v(x,y);
			BOOST_CHECK_CLOSE(iv.x, vv.x - vv.x * dfx_x(x,y) - vv.y * dfx_y(x,y), 1);
			BOOST_CHECK_CLOSE(iv.y, vv.y - vv.x * dfy_x(x,y) - vv.y * dfy_y(x,y), 1);
		}
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_jacobian, GridTransformFixture )
{
	C2DFVectorfield v(size);
	fill(v.begin(), v.end(), C2DFVector(10.0,20.0));

	C2DFVector lmg(64, 32);
	float j = field.get_jacobian(v, 1.0);
	C2DFMatrix J = field.derivative_at(lmg.x, lmg.y);

	BOOST_CHECK_CLOSE(j, J.x.x * J.y.y - J.x.y * J.y.x, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_curl, GridTransformFixture )
{
	double curl = 0.0; 
	const double n = (size.y - 2 ) * (size.x - 2); 
	for (size_t y = 1; y < size.y-1; ++y)
		for (size_t x = 1; x < size.x-1; ++x) {
			const float lcurl = dfx_y(x,y) - dfy_x(x,y); 
			curl += lcurl * lcurl; 
		}

	BOOST_CHECK_CLOSE(field.curl(), curl / n, 2.0);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_divergence, GridTransformFixture )
{
	double div = 0.0; 
	const double n = (size.y - 2 ) * (size.x - 2); 
	for (size_t y = 1; y < size.y-1; ++y)
		for (size_t x = 1; x < size.x-1; ++x) {
			const float dfxx = dfx_x(x,y); 
			const float dfyy = dfy_y(x,y); 
			div += dfxx * dfxx + dfyy * dfyy; 
		}

	BOOST_CHECK_CLOSE(field.divergence(), div / n , 2.0);
}


BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_grad_curl, GridTransformFixture )
{
	double gradcurl = 0.0; 
	double n = (size.y - 2 ) * (size.x - 2); 
	for (size_t y = 1; y < size.y-1; ++y)
		for (size_t x = 1; x < size.x-1; ++x) {
			const double gdfx_xy = dfx_xy(x,y); 
			const double gdfx_yy = dfx_yy(x,y);
			const double gdfy_xx = dfy_xx(x,y); 
			const double gdfy_xy = dfy_xy(x,y);
			gradcurl += gdfx_xy * gdfx_xy + gdfx_yy * gdfx_yy + 
				gdfy_xx * gdfy_xx + gdfy_xy * gdfy_xy - 
				2.0 * ( gdfx_xy * gdfy_xx + 
					gdfx_yy * gdfy_xy ); 

		}

	BOOST_CHECK_CLOSE(field.grad_curl(), gradcurl / n, 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_grad_divergence, GridTransformFixture )
{
	double graddiv = 0.0; 
	double n = (size.y - 2 ) * (size.x - 2); 
	for (size_t y = 1; y < size.y-1; ++y)
		for (size_t x = 1; x < size.x-1; ++x) {
			const double gdfxx_x = dfx_xx(x,y); 
			const double gdfxx_y = dfx_xy(x,y);
			const double gdfyy_x = dfy_yx(x,y); 
			const double gdfyy_y = dfy_yy(x,y);

			graddiv += 
				gdfxx_x * gdfxx_x + gdfxx_y * gdfxx_y + 
				gdfyy_x * gdfyy_x + gdfyy_y * gdfyy_y 
				+ 2.0 * ( gdfxx_x * gdfyy_x + gdfxx_y * gdfyy_y)
				; 
		}

	BOOST_CHECK_CLOSE(field.grad_divergence(), graddiv / n , 1);
}


BOOST_FIXTURE_TEST_CASE( test_grid_clone, GridTransformFixture )
{
	P2DTransformation clone(field.clone());

	C2DGridTransformation& spclone = dynamic_cast<C2DGridTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), field.get_size());

	C2DGridTransformation::const_iterator ic = spclone.begin();
	C2DGridTransformation::const_iterator ec = spclone.end();

	C2DGridTransformation::const_iterator io = field.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}


float GridTransformFixture::fx(float x, float y)
{

	x *= scale.x;
	y *= scale.y;
	return 	(1.0 + sinf(x - M_PI / 2.0)) * (1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::fy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return (1.0 - cosf(2 * x)) * (1.0 - cosf(y));
}


float GridTransformFixture::dfx_x(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return scale.x * cosf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::dfx_y(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 4.0 * M_PI / r.y * (1.0 + sinf(x - M_PI / 2.0)) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfy_x(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 4 * M_PI / r.x * sinf(2 * x) * (1.0 - cosf(y));
}

float GridTransformFixture::dfy_y(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return scale.y * (1.0 - cosf(2 * x)) * sinf(y);
}


float GridTransformFixture::dfx_xx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return - scale.x * scale.x * sinf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}
	
float GridTransformFixture::dfx_xy(float x, float y)

{
	x *= scale.x;
	y *= scale.y;
	return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -4.0 * scale.y * scale.y * (1.0 + sinf(x - M_PI / 2.0)) * sinf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfy_xx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -4.0 * scale.x * scale.x * cosf(2 * x) * (1.0 - cosf(y));
}

float GridTransformFixture::dfy_xy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -2.0 * scale.x * scale.y * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -2.0 * scale.y * scale.x * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return - scale.y * scale.y * (1.0 - cosf(2 * x)) * cosf(y);
}
