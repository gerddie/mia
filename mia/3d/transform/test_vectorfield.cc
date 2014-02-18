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
#include <mia/internal/plugintester.hh>

#include <mia/core/spacial_kernel.hh>
#include <mia/3d/transform/vectorfield.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;

PrepareTestPluginPath plugin_path_init; 

struct GridTransformFixture {
	GridTransformFixture():
		size(64, 32, 48),
		r(size.x - 1,size.y - 1, size.z - 1),
		field(size, C3DInterpolatorFactory("bspline:d=1", "mirror")),
		m_scale(1.0 / r.x, 1.0 / r.y, 1.0 / r.z)
	{
		C3DGridTransformation::field_iterator i = field.field_begin();
		C3DFVector v; 
		for (v.z = 0; v.z < size.z; ++v.z)
			for (v.y = 0; v.y < size.y; ++v.y)
				for (v.x = 0; v.x < size.x; ++v.x, ++i) {
					*i = C3DFVector( fx(v), fy(v), fz(v)); 
				}
	}
	C3DBounds size;
	C3DBounds r;
	C3DGridTransformation field;
protected:
	float fx(const C3DFVector& v);
	float fy(const C3DFVector& v);
	float fz(const C3DFVector& v);

	float dfx_x(const C3DFVector& v);
	float dfy_x(const C3DFVector& v);
	float dfz_x(const C3DFVector& v);

	float dfx_y(const C3DFVector& v);
	float dfy_y(const C3DFVector& v);
	float dfz_y(const C3DFVector& v);

	float dfx_z(const C3DFVector& v);
	float dfy_z(const C3DFVector& v);
	float dfz_z(const C3DFVector& v);

	float dfx_xx(float x, float y, float z);
	float dfx_xy(float x, float y, float z);
	float dfx_yx(float x, float y, float z);
	float dfx_yy(float x, float y, float z);
	float dfy_xx(float x, float y, float z);
	float dfy_xy(float x, float y, float z);
	float dfy_yx(float x, float y, float z);
	float dfy_yy(float x, float y, float z);

	C3DFVector scaled(C3DFVector x) const; 

	C3DFVector m_scale;
};

C3DFVector GridTransformFixture::scaled(C3DFVector x) const
{
	return m_scale * x; 
}

BOOST_AUTO_TEST_CASE(test_vf_plugin)
{
	auto vf_creator = BOOST_TEST_create_from_plugin<C3DGridTransformCreatorPlugin>("vf");
	BOOST_CHECK(vf_creator); 
	
	C3DBounds size(2,3,4); 
	auto vf = vf_creator->create(size); 
	
	BOOST_CHECK_EQUAL(vf->get_size(), size); 
}



BOOST_FIXTURE_TEST_CASE(test_gridtransform_basic_props, GridTransformFixture)
{
	BOOST_CHECK_EQUAL(field.degrees_of_freedom(), size.product()  * 3);


	C3DFVector testx0(22, 22, 23);
	C3DFVector result0 = field.apply(testx0);

	BOOST_CHECK_CLOSE(result0.x, fx(testx0), 1);
	BOOST_CHECK_CLOSE(result0.y, fy(testx0), 1);
	BOOST_CHECK_CLOSE(result0.z, fz(testx0), 1);

	C3DFVector testx1(32.5, 22, 23);
	C3DFVector result1 = field.apply(testx1);

	BOOST_CHECK_CLOSE(result1.x, fx(testx1), 1);
	BOOST_CHECK_CLOSE(result1.y, fy(testx1), 1);
	BOOST_CHECK_CLOSE(result1.z, fz(testx1), 1);

	C3DFVector testx2(32, 22, 23);
	C3DFVector result2 = field.apply(testx2);

	BOOST_CHECK_CLOSE(result2.x, fx(testx2), 1);
	BOOST_CHECK_CLOSE(result2.y, fy(testx2), 1);
	BOOST_CHECK_CLOSE(result2.z, fz(testx2), 1);


	// the non-linearity of the test function is quite high ... 
	C3DFVector testx(32.2, 21.9, 22.9);
	C3DFVector result = field.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx), 4);
	BOOST_CHECK_CLOSE(result.y, fy(testx), 4);
	BOOST_CHECK_CLOSE(result.z, fz(testx), 4);

	result = field(testx);

	BOOST_CHECK_CLOSE(result.x, testx.x - fx(testx), 0.1);
	BOOST_CHECK_CLOSE(result.y, testx.y - fy(testx), 0.1);
	BOOST_CHECK_CLOSE(result.z, testx.z - fz(testx), 0.1);

}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative_1, GridTransformFixture)
{
	C3DFVector v(10,11,12);
	C3DFMatrix dv =  field.derivative_at(10,11,12);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(v), 1);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(v), 1);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(v), 1);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(v), 1);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(v), 1);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(v), 1);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(v), 1);
	
}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative_at, GridTransformFixture)
{
	C3DFVector v(10.2,11.4,12.6);
	C3DFMatrix dv =  field.derivative_at(v);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(v), 1);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(v), 1);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(v), 1);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(v), 1);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(v), 1);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(v), 1);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(v), 1);
	
}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative_at_centered, GridTransformFixture)
{
	C3DFVector v(10.5,11.5,12.5);
	C3DFMatrix dv =  field.derivative_at(v);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(v), 1);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(v), 1);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(v), 1);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(v), 1);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(v), 1);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(v), 1);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(v), 1);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(v), 1);
	
}


BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative, GridTransformFixture)
{
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x) {
				C3DFVector v(x,y,z);
				C3DFMatrix dv =  field.derivative_at(x,y,z);
				if ( x > 0 && x < size.x- 1) {
					BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(v), 1);
					BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(v), 1);
					BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(v), 1);
				}else {
					BOOST_CHECK_EQUAL(dv.x.x, 1.0f);
					BOOST_CHECK_EQUAL(dv.x.y, 0);
					BOOST_CHECK_EQUAL(dv.x.z, 0);
				}
				if ( y > 0 && y < size.y - 1) {
					BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(v), 1);
					BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(v), 1);
					BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(v), 1);
				}else {
					BOOST_CHECK_EQUAL(dv.y.x, 0);
					BOOST_CHECK_EQUAL(dv.y.y, 1.0f);
					BOOST_CHECK_EQUAL(dv.y.z, 0);
				}
				if ( z > 0 && z < size.z - 1) {
					BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(v), 1);
					BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(v), 1);
					BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(v), 1);
				}else {
					BOOST_CHECK_EQUAL(dv.z.x, 0);
					BOOST_CHECK_EQUAL(dv.z.y, 0);
					BOOST_CHECK_EQUAL(dv.z.z, 1.0);
				}

			}
}


BOOST_FIXTURE_TEST_CASE(test_gridtransform_set_identity, GridTransformFixture)
{
	field.set_identity();
	C3DGridTransformation::const_iterator i = field.begin();
	for(size_t z = 0; z < size.z; ++z)
		for(size_t y = 0; y < size.y; ++y)
			for(size_t x = 0; x < size.x; ++x, ++i) {
				BOOST_CHECK_EQUAL( *i, C3DFVector(x,y,z));
			}
	
}

BOOST_AUTO_TEST_CASE( test_grid3d_iterator )
{
	C3DBounds size(5,10,7);
	C3DInterpolatorFactory ipf("bspline:d=1", "mirror"); 
	C3DGridTransformation gt(size, ipf);

	C3DGridTransformation::field_iterator i = gt.field_begin();
	C3DFVector value(0.12, 0.32, 0.36);

	while (i != gt.field_end()) {
		*i = value;
		value.x += 0.12f;
		value.y += 0.32f;
		value.z += 0.36f;
		++i;
	}

	C3DTransformation::const_iterator k = gt.begin();

	float n = 1.0f;
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++k, n += 1.0f) {
				BOOST_CHECK_CLOSE( k->x , (float)x - n * 0.12f, 0.1f );
				BOOST_CHECK_CLOSE( k->y , (float)y - n * 0.32f, 0.1f );
				BOOST_CHECK_CLOSE( k->z , (float)z - n * 0.36f, 0.1f );
				
			}
}

BOOST_AUTO_TEST_CASE( test_get_parameters )
{
	C3DFVector init_a[8] = {
		C3DFVector(1,1,1), C3DFVector(2,1,1), 
		C3DFVector(1,2,1), C3DFVector(2,2,1), 
		
		C3DFVector(1,1,2), C3DFVector(2,1,2), 
		C3DFVector(1,2,2), C3DFVector(2,2,2)
	};

	C3DBounds size(2,2,2);
	C3DInterpolatorFactory ipf("bspline:d=1", "mirror"); 
	C3DGridTransformation a(size, ipf);
	std::copy(init_a, init_a + 8, a.field_begin());
	
	CDoubleVector params = a.get_parameters(); 

	BOOST_CHECK_EQUAL(params.size(), 24u); 
	BOOST_REQUIRE(params.size() == 24u); 

	double test_params[24] =  {1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 2, 1, 
				   1, 1, 2, 2, 1, 2, 1, 2, 2, 2, 2, 2}; 

	for (int i = 0; i < 24; ++i)
		BOOST_CHECK_EQUAL(params[i], test_params[i]); 
	
	
}

BOOST_AUTO_TEST_CASE( test_set_parameters )
{
	double init_params[24] =  {1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 2, 1, 
				   1, 1, 2, 2, 1, 2, 1, 2, 2, 2, 2, 2}; 
	

	C3DFVector test_vectors[8] = {
		C3DFVector(1,1,1), C3DFVector(2,1,1), 
		C3DFVector(1,2,1), C3DFVector(2,2,1), 
		
		C3DFVector(1,1,2), C3DFVector(2,1,2), 
		C3DFVector(1,2,2), C3DFVector(2,2,2)
	};

	C3DBounds size(2,2,2);
	C3DInterpolatorFactory ipf("bspline:d=1", "mirror"); 
	C3DGridTransformation a(size, ipf);
	
	CDoubleVector params(24); 
	std::copy(init_params, init_params + 24, params.begin());
	
	a.set_parameters(params); 

	auto ia = a.field_begin(); 
	for (int i = 0; i < 8; ++i, ++ia)
		BOOST_CHECK_EQUAL(*ia, test_vectors[i]); 
	
	
}

BOOST_AUTO_TEST_CASE( test_gridtransform_add )
{
	C3DFVector init_a[18] = {
		C3DFVector(2,3,2), C3DFVector(0,0,0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(-1,-1, 0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(0,0,0), C3DFVector(0,0,0),

		C3DFVector(2,3,2), C3DFVector(0,0,0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(-1,-1, 0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(0,0,0), C3DFVector(0,0,0)
	};

	C3DFVector init_b[18] = {
		C3DFVector(0,0,0), C3DFVector(0,0,0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(1,1,0), C3DFVector(0.5,0,0),
		C3DFVector(0,0,0), C3DFVector(0,0.5,0), C3DFVector(4,5,0),

		C3DFVector(0,0,0), C3DFVector(0,0,0), C3DFVector(0,0,0),
		C3DFVector(0,0,0), C3DFVector(1,1,1), C3DFVector(0.5,0,0),
		C3DFVector(0,0,0), C3DFVector(0,0.5,0), C3DFVector(4,5,0)
	};


	C3DBounds size(3,3, 2);
	C3DInterpolatorFactory ipf("bspline:d=1", "mirror"); 
	C3DGridTransformation a(size, ipf);
	std::copy(init_a, init_a + 18, a.field_begin());

	C3DGridTransformation b(size, ipf);
	std::copy(init_b, init_b + 18, b.field_begin());

	C3DGridTransformation c = a + b;

	BOOST_CHECK_EQUAL( c.apply(C3DFVector(1,1,0)), C3DFVector(3,4,2));
	BOOST_CHECK_EQUAL( c.apply(C3DFVector(1,1,1)), C3DFVector(3,4,3));
	BOOST_CHECK_EQUAL( c.apply(C3DFVector(2,1,0)), C3DFVector(0.0,-0.5,0));
	BOOST_CHECK_EQUAL( c.apply(C3DFVector(1,2,0)), C3DFVector(-0.5,0.0,0));

}


BOOST_FIXTURE_TEST_CASE( test_gridtransform_max, GridTransformFixture )
{
	const float max = sqrt(49 + 49 + 49 * 2.25 * 2.25);
	BOOST_CHECK_CLOSE(field.get_max_transform(), max, 1);
}

#if 0 
BOOST_FIXTURE_TEST_CASE( test_gridtransform_pertuberate, GridTransformFixture )
{
	C3DFVectorfield v(size);
	C3DFVector vv(10.0, 20.0);
	fill(v.begin(), v.end(), vv);

	float gamma = field.pertuberate(v);

	C3DFVector lmg(184, 107);
	C3DFVector mg(vv.x - vv.x * dfx_x(lmg.x,lmg.y) - vv.y * dfx_y(lmg.x,lmg.y),
		      vv.y - vv.x * dfy_x(lmg.x,lmg.y) - vv.y * dfy_y(lmg.x,lmg.y));
	BOOST_CHECK_CLOSE(gamma, mg.norm(), 0.1);

	for (size_t y = 1; y < size.y - 1; ++y)
		for (size_t x = 1; x < size.x - 1; ++x) {
			const C3DFVector& iv = v(x,y);
			BOOST_CHECK_CLOSE(iv.x, vv.x - vv.x * dfx_x(x,y) - vv.y * dfx_y(x,y), 1);
			BOOST_CHECK_CLOSE(iv.y, vv.y - vv.x * dfy_x(x,y) - vv.y * dfy_y(x,y), 1);
		}
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_jacobian, GridTransformFixture )
{
	C3DFVectorfield v(size);
	fill(v.begin(), v.end(), C3DFVector(10.0,20.0));

	C3DFVector lmg(64, 32);
	float j = field.get_jacobian(v, 1.0);
	C3DFMatrix J = field.derivative_at(lmg.x, lmg.y);

	BOOST_CHECK_CLOSE(j, J.x.x * J.y.y - J.x.y * J.y.x, 0.1);
}
#endif

BOOST_FIXTURE_TEST_CASE( test_grid_clone, GridTransformFixture )
{
	P3DTransformation clone(field.clone());

	C3DGridTransformation& spclone = dynamic_cast<C3DGridTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), field.get_size());

	C3DGridTransformation::const_iterator ic = spclone.begin();
	C3DGridTransformation::const_iterator ec = spclone.end();

	C3DGridTransformation::const_iterator io = field.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}



float GridTransformFixture::fx(const C3DFVector& v)
{

	const C3DFVector s = scaled(v); 
	const C3DFVector h( 1.0 - s.x, 2.0 - 2 * s.y, 3.0 - 3 * s.z);  
	return 	0.5 * h.norm2(); 
}

float GridTransformFixture::fy(const C3DFVector& v)
{
	const C3DFVector s = scaled(v); 	
	const C3DFVector h( 1.0 - s.x, 2.0 - 2 * s.y, 3.0 - 3 * s.z);  
	return 0.5 * 2.25 * h.norm2(); 
}

float GridTransformFixture::fz(const C3DFVector& v)
{
	const C3DFVector s = scaled(v); 
	const C3DFVector h( 1.0 - s.x, 2.0 - 2 * s.y, 3.0 - 3 * s.z);  
	return 	-0.5 * h.norm2(); 
}

float GridTransformFixture::dfx_x(const C3DFVector& v)
{
	return m_scale.x * (m_scale.x * v.x - 1); 
}

float GridTransformFixture::dfx_y(const C3DFVector& v)
{
	return m_scale.y * (2.0 * m_scale.y * v.y - 2.0) * 2.0; 

}

float GridTransformFixture::dfx_z(const C3DFVector& v)
{
	return m_scale.z *  (3.0 * m_scale.z * v.z - 3.0) * 3.0; 
}


float GridTransformFixture::dfy_x(const C3DFVector& v)
{
	return 2.25 * m_scale.x *  (m_scale.x *v.x - 1); 
}

float GridTransformFixture::dfy_y(const C3DFVector& v)
{
	return 2.25  * m_scale.y *  (2.0 * m_scale.y *v.y - 2.0) * 2.0; 
}

float GridTransformFixture::dfy_z(const C3DFVector& v)
{
	return 2.25 * m_scale.z *  (3.0 * m_scale.z *v.z - 3.0) * 3.0; 
}

float GridTransformFixture::dfz_x(const C3DFVector& v)
{
	return - m_scale.x *  (m_scale.x *v.x - 1); 
}

float GridTransformFixture::dfz_y(const C3DFVector& v)
{
	return - m_scale.y *  (2.0 * m_scale.y *v.y - 2.0) * 2.0; 
}

float GridTransformFixture::dfz_z(const C3DFVector& v)
{
	return - m_scale.z *  (3.0 * m_scale.z *v.z - 3.0) * 3.0; 
}

float GridTransformFixture::dfx_xx(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return - m_scale.x * m_scale.x * sinf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::dfx_xy(float x, float y, float z)

{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return 2.0 * m_scale.x * m_scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yx(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return 2.0 * m_scale.x * m_scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yy(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return -4.0 * m_scale.y * m_scale.y * (1.0 + sinf(x - M_PI / 2.0)) * sinf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfy_xx(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return -4.0 * m_scale.x * m_scale.x * cosf(2 * x) * (1.0 - cosf(y));
}

float GridTransformFixture::dfy_xy(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return -2.0 * m_scale.x * m_scale.y * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yx(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return -2.0 * m_scale.y * m_scale.x * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yy(float x, float y, float z)
{
	x *= m_scale.x;
	y *= m_scale.y;
	z *= m_scale.z;
	return - m_scale.y * m_scale.y * (1.0 - cosf(2 * x)) * cosf(y);
}

