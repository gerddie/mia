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
#include <fstream>

#include <mia/core/spacial_kernel.hh>
#include <mia/3d/interpolator.hh>
#include <mia/3d/transform/spline.hh>


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


struct TransformSplineFixture: public ipfFixture {
	TransformSplineFixture():
		size(40,65,25),
		kernel(produce_spline_kernel("bspline:d=3")),
		range(50, 80, 30),
		r(range.x - 1, range.y - 1, range.z - 1),
		stransf(range, kernel, ipf, P3DSplineTransformPenalty()), 
		center(range.x/2.0, range.y/2.0, range.z/2.0), 
		fscale(range.x * range.y * range.z)

	{
		coeff_shift = kernel->get_active_halfrange() - 1; 
		
		ivscale = C3DFVector (float(range.x - 1) / (size.x - 1),
				      float(range.y - 1) / (size.y - 1),
				      float(range.z - 1) / (size.z - 1));
		cvdebug() << "ivscale= " << ivscale << "\n"; 
		
		size.x += 2*coeff_shift;
		size.y += 2*coeff_shift;
		size.z += 2*coeff_shift;
		

		cs = C3DFVector(coeff_shift, coeff_shift, coeff_shift); 

		C3DFVectorfield field(size); 

		C3DFVectorfield::iterator i = field.begin();
		for (size_t z = 0; z < size.z; ++z)
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x, ++i) {
					C3DFVector X(x,y,z); 
					C3DFVector s(ivscale * (X - cs)); 
					*i = C3DFVector( fx(s), fy(s), fz(s));
				}


		stransf.set_coefficients_and_prefilter(field); 
		stransf.reinit();
	}
	C3DBounds size;
	C3DFVector ivscale; 
	C3DFVector cs; 
	PSplineKernel kernel;

	C3DBounds range;
	C3DBounds r;
	C3DSplineTransformation  stransf;
	int coeff_shift; 
	C3DFVector center; 
	float fscale; 
protected:

	C3DFVector f(const C3DFVector& x)const;

	float fx(float x, float y, float z)const ;
	float fy(float x, float y, float z)const ;
	float fz(float x, float y, float z)const ;

	float fx(const C3DFVector& x)const;
	float fy(const C3DFVector& x)const;
	float fz(const C3DFVector& x)const;


	float dfx_x(const C3DFVector& x);
	float dfy_x(const C3DFVector& x);
	float dfz_x(const C3DFVector& x);
	float dfx_y(const C3DFVector& x);
	float dfy_y(const C3DFVector& x);
	float dfz_y(const C3DFVector& x);
	float dfx_z(const C3DFVector& x);
	float dfy_z(const C3DFVector& x);
	float dfz_z(const C3DFVector& x);
private:
	C3DFVector scale;
};

C3DFVector TransformSplineFixture::f(const C3DFVector& x)const
{
	return C3DFVector(fx(x), fy(x), fz(x)); 
}

float TransformSplineFixture::fx(const C3DFVector& v)const
{
	C3DFVector X = v - center; 
	return 	exp ( - (6 * X.x * X.x + 5 * X.y * X.y + 4 * X.z*X.z) / fscale ) ;
}

float TransformSplineFixture::fy(const C3DFVector& v)const
{
	C3DFVector X = v - center; 
	return 	exp ( - (3 * X.x * X.x + 7 * X.y * X.y + 2 * X.z*X.z) / fscale ) ;

}

float TransformSplineFixture::fz(const C3DFVector& v)const
{
	C3DFVector X = v - center; 
	return 	exp ( - (2 * X.x * X.x + 4 * X.y * X.y + 5 * X.z*X.z) / fscale ) ;

}

float TransformSplineFixture::fx(float x, float y, float z)const
{
	return fx(C3DFVector(x,y,z)); 
}

float TransformSplineFixture::fy(float x, float y, float z)const
{
	return fy(C3DFVector(x,y,z)); 
}

float TransformSplineFixture::fz(float x, float y, float z)const
{
	return fz(C3DFVector(x,y,z)); 
}


float TransformSplineFixture::dfx_x(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - (12 * X.x ) / fscale  * fx(v);
}

float TransformSplineFixture::dfx_y(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - (10 * X.y ) / fscale  * fx(v);

}

float TransformSplineFixture::dfx_z(const C3DFVector& v)
{
 	C3DFVector X = v - center; 
	return - ( 8 * X.z ) / fscale  * fx(v);
}

float TransformSplineFixture::dfy_x(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - ( 6 * X.x ) / fscale  * fy(v);
}

float TransformSplineFixture::dfy_y(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - (14 * X.y ) / fscale  * fy(v);

}

float TransformSplineFixture::dfy_z(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - ( 4 * X.z ) / fscale  * fy(v);

}

float TransformSplineFixture::dfz_x(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - ( 4 * X.x ) / fscale  * fz(v);
}

float TransformSplineFixture::dfz_y(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - ( 8 * X.y ) / fscale  * fz(v);
}

float TransformSplineFixture::dfz_z(const C3DFVector& v)
{
	C3DFVector X = v - center; 
	return - (10 * X.z ) / fscale  * fz(v);
}


BOOST_FIXTURE_TEST_CASE( test_splines_transformation, TransformSplineFixture )
{
	BOOST_CHECK_EQUAL(stransf.degrees_of_freedom(), size.x*size.y*size.z * 3);

	C3DFVector testx(34.4, 50.8, 21.6); 

	BOOST_CHECK_EQUAL(stransf.get_size(), range);

	BOOST_CHECK_EQUAL(C3DFVector(stransf.get_enlarge()), 2 * cs); 

	C3DFVector scaled = stransf.scale(testx); 
	C3DFVector testscale = testx / ivscale  + cs; 

	BOOST_CHECK_CLOSE(scaled.x, testscale.x, 0.1);
	BOOST_CHECK_CLOSE(scaled.y, testscale.y, 0.1);
	BOOST_CHECK_CLOSE(scaled.z, testscale.z, 0.1);
	
	C3DFVector result_apply = stransf.apply(testx);
	BOOST_CHECK_CLOSE(result_apply.x, fx(testx), 0.1);
	BOOST_CHECK_CLOSE(result_apply.y, fy(testx), 0.1);
	BOOST_CHECK_CLOSE(result_apply.z, fz(testx), 0.1);

	C3DFVector result_operator = stransf(testx);
	BOOST_CHECK_CLOSE(result_operator.x, testx.x - fx(testx), 0.1);
	BOOST_CHECK_CLOSE(result_operator.y, testx.y - fy(testx), 0.1);
	BOOST_CHECK_CLOSE(result_operator.z, testx.z - fz(testx), 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator, TransformSplineFixture )
{
	
	auto i = stransf.begin();

	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++i) {
				C3DBounds X(x,y,z); 
				cvdebug() << X  << *i << "\n"; 
				if (fabs(x - fx(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + x - fx(x,y,z), 1.0 + i->x, .41);
				else
					BOOST_CHECK_CLOSE(x - fx(x,y,z), i->x, .41);

				if (fabs(y - fy(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + y - fy(x,y,z), 1.0 + i->y, 0.21);
				else 
					BOOST_CHECK_CLOSE(y - fy(x,y,z), i->y, 0.21);
				
				if (fabs(z - fz(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + z - fz(x,y,z), 1.0 + i->z, 0.3);
				else 
					BOOST_CHECK_CLOSE(z - fz(x,y,z), i->z, .8);
			}
}

BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_ranged_iterator, TransformSplineFixture )
{
	
	auto i = stransf.begin_range(C3DBounds(3,2,1), range - C3DBounds(3,2,1));

	for (size_t z = 1; z < range.z - 1; ++z)
		for (size_t y = 2; y < range.y - 2; ++y)
			for (size_t x = 3; x < range.x - 3; ++x, ++i) {
				C3DBounds X(x,y,z); 
				cvdebug() << X  << *i << "\n"; 
				if (fabs(x - fx(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + x - fx(x,y,z), 1.0 + i->x, .41);
				else
					BOOST_CHECK_CLOSE(x - fx(x,y,z), i->x, .41);

				if (fabs(y - fy(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + y - fy(x,y,z), 1.0 + i->y, 0.21);
				else 
					BOOST_CHECK_CLOSE(y - fy(x,y,z), i->y, 0.21);
				
				if (fabs(z - fz(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + z - fz(x,y,z), 1.0 + i->z, 0.3);
				else 
					BOOST_CHECK_CLOSE(z - fz(x,y,z), i->z, .8);
			}
	
	BOOST_CHECK(i == stransf.end_range(C3DBounds(3,2,1), range - C3DBounds(3,2,1)));
}


BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator2, TransformSplineFixture )
{
	
	auto i = stransf.begin();

	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++i) {
				C3DFVector help = stransf(C3DFVector(x,y,z));
				BOOST_CHECK_CLOSE(help.x, i->x, 0.1);
				BOOST_CHECK_CLOSE(help.y, i->y, 0.1);
				BOOST_CHECK_CLOSE(help.z, i->z, 0.1);
			}
}


BOOST_FIXTURE_TEST_CASE( test_splinestransform_postfix_iterator, TransformSplineFixture )
{
	auto i = stransf.begin();
	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, i++) {
				cvdebug() << "splinestransform_postfix_iterator" << *i << "\n"; 
				if (fabs(x - fx(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + x - fx(x,y,z), 1.0 + i->x, 1.0);
				else
					BOOST_CHECK_CLOSE(x - fx(x,y,z), i->x, 1.0);

				if (fabs(y - fy(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + y - fy(x,y,z), 1.0 + i->y, 0.3);
				else 
					BOOST_CHECK_CLOSE(y - fy(x,y,z), i->y, 0.3);
				
				if (fabs(z - fz(x,y,z)) < 1e-8) 
					BOOST_CHECK_CLOSE(1.0 + z - fz(x,y,z), 1.0 + i->z, 0.4);
				else 
					BOOST_CHECK_CLOSE(z - fz(x,y,z), i->z, 1.0);
			}
}

BOOST_FIXTURE_TEST_CASE( test_splines_deform, TransformSplineFixture )
{

	C3DFImage image(range);
	C3DFImage::iterator i = image.begin();
	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++i) {
				*i = (x + 1) * (y + 1) * (z+1);
			}
	
	C3DFImage test_image(range);
	
	unique_ptr<T3DInterpolator<float> > src(ipf.create(image.data()));

	C3DFImage::iterator t = test_image.begin();

	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++t) {
				const C3DFVector v(x,y,z);
				*t = (*src)(stransf(v));
			}
	
	P3DImage stransfed = stransf(image);
	const C3DFImage& ptransformed = dynamic_cast<const C3DFImage&>(*stransfed);

	BOOST_CHECK_EQUAL(ptransformed.get_size(), image.get_size());
	BOOST_REQUIRE(ptransformed.get_size() == image.get_size());

	BOOST_CHECK_EQUAL(ptransformed.get_pixel_type(), image.get_pixel_type());
	
	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++i) {
				BOOST_CHECK_CLOSE(ptransformed(x,y,z), test_image(x,y,z), 0.1);
			}
}

BOOST_FIXTURE_TEST_CASE( test_splines_translate, TransformSplineFixture )
{
	C3DFVectorfield gradient(range); 
	double scale_x = (size.x - 1 - 2*coeff_shift) / double(range.x-1);
	double scale_y = (size.y - 1 - 2*coeff_shift) / double(range.y-1);
	double scale_z = (size.z - 1 - 2*coeff_shift) / double(range.z-1);
	double scale = (scale_x * scale_y * scale_z);
	
	cvdebug() << range << size << ", scale=" << scale << "\n"; 

	fill(gradient.begin(), gradient.end(), C3DFVector(1.0, 2.0, 3.0));

	CDoubleVector force(stransf.degrees_of_freedom());
	stransf.translate(gradient, force);

	// the real test uses finite differences and is implemented elsewhere
	// the tolerance here is way to high ... 
	auto i = force.begin(); 
	for (size_t z = 0; z < stransf.get_coeff_size().z; ++z) {
		for (size_t y = 0; y < stransf.get_coeff_size().y; ++y) {
			for (size_t x = 0; x < stransf.get_coeff_size().x ; ++x, i+=3) {
				if (y > 2 && y < stransf.get_coeff_size().y - 2 && 
				    x > 2 && x < stransf.get_coeff_size().x - 2 && 
				    z > 2 && z < stransf.get_coeff_size().z - 2) {
					cvdebug() << "l = " << x << ", " << y << ", " << z << "\n"; 
					BOOST_CHECK_CLOSE( i[0] * scale, -1.0f, 1.0);
					BOOST_CHECK_CLOSE( i[1] * scale, -2.0f, 1.0);
					BOOST_CHECK_CLOSE( i[2] * scale, -3.0f, 1.0);
				}
			}
		}
	}
}

BOOST_FIXTURE_TEST_CASE( test_splines_get_set_parameters, TransformSplineFixture )
{
	auto params = stransf.get_parameters(); 
	float k = 0.1; 
	for (auto i = params.begin(); i != params.end(); ++i, k += 0.1) 
		*i = k; 
	
	stransf.set_parameters(params); 
	
	auto params2 = stransf.get_parameters(); 
	for (auto i = params.begin(), j = params2.begin(); i != params.end(); ++i, ++j) 
		BOOST_CHECK_EQUAL(*i, *j); 
	
}

BOOST_FIXTURE_TEST_CASE( test_splines_clone, TransformSplineFixture )
{
	P3DTransformation clone(stransf.clone());

	C3DSplineTransformation& spclone = dynamic_cast<C3DSplineTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), stransf.get_size());
	BOOST_CHECK_EQUAL(spclone.get_coeff_size(), stransf.get_coeff_size());

	C3DSplineTransformation::const_iterator ic = spclone.begin();
	C3DSplineTransformation::const_iterator ec = spclone.end();

	C3DSplineTransformation::const_iterator io = stransf.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}

BOOST_FIXTURE_TEST_CASE( test_splines_update, TransformSplineFixture )
{
	C3DFVectorfield update(stransf.get_coeff_size());
	for (unsigned int z = 0; z < update.get_size().z; ++z)
		for (unsigned int y = 0; y < update.get_size().y; ++y)
			for (unsigned int x = 0; x < update.get_size().x; ++x) {
				update(x,y,z) = C3DFVector(1.0, 2.0, 3.0);
			}
	
	stransf.update(2.0, update);
	stransf.reinit();

	C3DFVector testx(33.4, 20.4, 21.9);
	C3DFVector result = stransf.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx) + 2.0f, 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx) + 4.0f, 0.1);
	BOOST_CHECK_CLOSE(result.z, fz(testx) + 6.0f, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_splines_gridpoint_derivative, TransformSplineFixture )
{

	C3DFMatrix dv =  stransf.derivative_at(33,29,10);
	C3DFVector x(33,29,10);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(x), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(x), 0.2);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(x), 0.2);

}

BOOST_FIXTURE_TEST_CASE( test_splines_abitrary_derivative, TransformSplineFixture )
{

	C3DFVector x(33.7, 29.8, 10.1);
	C3DFMatrix dv =  stransf.derivative_at(x);
	

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(x), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(x), 0.2);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(x), 0.2);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(x), 0.2);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(x), 0.2);

}


BOOST_FIXTURE_TEST_CASE( test_splines_gridpoint_derivative_out_of_range, TransformSplineFixture )
{
	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(-0.1, 1, 1)), C3DFMatrix::_1);
	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(1, -0.1, 1)), C3DFMatrix::_1);
	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(1, 1, -0.1)), C3DFMatrix::_1);

	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(range.x, 1, 1)), C3DFMatrix::_1);
	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(1, range.y, 1)), C3DFMatrix::_1);
	BOOST_CHECK_EQUAL(stransf.derivative_at(C3DFVector(1, 1, range.z)), C3DFMatrix::_1);

}


BOOST_FIXTURE_TEST_CASE( test_splines_set_identity, TransformSplineFixture )
{
	stransf.set_identity();

	C3DSplineTransformation::const_iterator i = stransf.begin();
	for(size_t z = 0; z < range.z; ++z)
		for(size_t y = 0; y < range.y; ++y)
			for(size_t x = 0; x < range.x; ++x, ++i) {
				BOOST_CHECK_EQUAL( *i, C3DFVector(x,y,z));
			}
}

#if 0 
BOOST_FIXTURE_TEST_CASE( test_splines_get_max_transform, TransformSplineFixture )
{
	float fx0 = fx(48,64);
	float fy0 = fy(48,64);

	BOOST_CHECK_CLOSE(sqrt(fx0*fx0 + fy0*fy0), stransf.get_max_transform(), 2.0);
}
#endif

#if 0 
BOOST_FIXTURE_TEST_CASE( test_splines_pertuberate, TransformSplineFixture )
{
	C3DFVectorfield v(size);
	C3DFVector vv(1.0, 2.0);
	float scalex = float(range.x - 1.0)  / (size.x - 1.0);
	float scaley = float(range.y - 1.0)  / (size.y - 1.0);


	fill(v.begin(), v.end(), vv);

	// this location is hand-picked and is not really the position ofthe maximun
	// but only an approximation
	float gamma = stransf.pertuberate(v);
	C3DFVector lmg(12* scalex, 27 * scaley);
	C3DFVector mg(vv.x - vv.x * dfx_x(lmg.x,lmg.y) - vv.y * dfx_y(lmg.x,lmg.y),
		      vv.y - vv.x * dfy_x(lmg.x,lmg.y) - vv.y * dfy_y(lmg.x,lmg.y));

	BOOST_CHECK_CLOSE(gamma, mg.norm(), 0.1);

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x) {
			const C3DFVector& iv = v(x,y);
			const float sx = x * scalex;
			const float sy = y * scaley;

			BOOST_CHECK_CLOSE(iv.x, vv.x - vv.x * dfx_x(sx,sy) - vv.y * dfx_y(sx,sy), 0.1);
			BOOST_CHECK_CLOSE(iv.y, vv.y - vv.x * dfy_x(sx,sy) - vv.y * dfy_y(sx,sy), 0.1);
		}
}

BOOST_FIXTURE_TEST_CASE( test_splines_get_jacobian, TransformSplineFixture )
{
	C3DFVectorfield v(size);
	fill(v.begin(), v.end(), C3DFVector(10.0,20.0));

	// this is not the location of the minimal value, we onyl approximate
	// by using the minimal value on the coefficient grid
	float scalex = float(range.x - 1.0)  / (size.x - 1.0);
	float scaley = float(range.y - 1.0)  / (size.y - 1.0);

	C3DFVector lmg(8*scalex, 16*scaley);
	float j = stransf.get_jacobian(v, 1.0);
	C3DFMatrix J = stransf.derivative_at(lmg.x, lmg.y);

	BOOST_CHECK_CLOSE(j, J.x.x * J.y.y - J.x.y * J.y.x, 0.1);
}
#endif

BOOST_FIXTURE_TEST_CASE( test_spline_c_rate_create, ipfFixture )
{
	PSplineKernel kernel = produce_spline_kernel("bspline:d=3"); 
	C3DBounds size(20, 32, 25);
	C3DFVector c_rate(2.5, 3.2, 5.0);
	C3DSplineTransformation  stransf(size, kernel, c_rate, ipf, P3DSplineTransformPenalty());

	C3DBounds gridsize = stransf.get_coeff_size();
	BOOST_CHECK_EQUAL(gridsize.x, 10u);
	BOOST_CHECK_EQUAL(gridsize.y, 12u);
	BOOST_CHECK_EQUAL(gridsize.z,  7u);

	C3DSplineTransformation::const_iterator i = stransf.begin();
	for(size_t z = 0; z < size.z; ++z)
		for(size_t y = 0; y < size.y; ++y)
			for(size_t x = 0; x < size.x; ++x, ++i) {
				BOOST_CHECK_EQUAL( *i, C3DFVector(x,y,z));
			}

}


BOOST_FIXTURE_TEST_CASE( test_splines_upscale, TransformSplineFixture )
{
	C3DBounds new_range(2 * range - C3DBounds::_1);

	auto upscaled = stransf.upscale(new_range); 
	BOOST_REQUIRE(upscaled); 

	C3DFVector rs(C3DFVector(new_range - C3DBounds::_1)/C3DFVector(range - C3DBounds::_1));

	auto i = upscaled->begin(); 
	for (size_t z = 0; z < new_range.z; ++z) {
		for (size_t y = 0; y < new_range.y; ++y) {
			for (size_t x = 0; x < new_range.x ; ++x, ++i) {
				cvdebug() << x << ", " << y << "\n"; 
				C3DFVector X(x,y,z); 
				C3DFVector ff = f(X/rs) * rs; 
				C3DFVector res = X - *i; 
				
				if (std::fabs(ff.x > 1e-8) || std::fabs(res.x) > 1e-8) 
					BOOST_CHECK_CLOSE(res.x, ff.x, 2);
				
				if (std::fabs(ff.y > 1e-8)|| std::fabs(res.y) > 1e-8) 
					BOOST_CHECK_CLOSE(res.y, ff.y, 2);
				
				if (std::fabs(ff.z > 1e-8)|| std::fabs(res.z) > 1e-8) 
					BOOST_CHECK_CLOSE(res.z, ff.z, 2);

			}
		}
	}

}

BOOST_FIXTURE_TEST_CASE( test_splines_refine, TransformSplineFixture )
{
	size = C3DBounds(17, 33, 23); 
	
	C3DFVector ivscale = (C3DFVector(range) - C3DFVector::_1) / 
		(C3DFVector(size) - C3DFVector::_1); 

	
	size.x += 2*coeff_shift; 
	size.y += 2*coeff_shift; 
	size.z += 2*coeff_shift; 

	C3DFVectorfield coefs(size); 
	
	auto i = coefs.begin();
	C3DFVector cshift(coeff_shift, coeff_shift, coeff_shift); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i)
				*i = f(ivscale * (C3DFVector(x,y,z) - cshift));
	
	stransf.set_coefficients(coefs);

	BOOST_CHECK(stransf.refine());
	auto k = stransf.begin(); 
	for (size_t z = 0; z < range.z; ++z) {
		for (size_t y = 0; y < range.y; ++y) {
			for (size_t x = 0; x < range.x; ++x, ++k) {
				
				C3DFVector X(x,y,z); 
				C3DFVector ff(f(X));
				cvdebug() << X  << " have " << (X - *k) << " exp " << ff <<"\n"; 
				
				BOOST_CHECK_CLOSE(ff.x, x - k->x, 0.1);
				BOOST_CHECK_CLOSE(ff.y, y - k->y, 0.1);
				BOOST_CHECK_CLOSE(ff.z, z - k->z, 0.1);
			}
		}
	}
}

BOOST_FIXTURE_TEST_CASE( test_splines_transform, ipfFixture )
{
	float src_image_init[10 * 10] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0,10,30,30, 0, 0, 0,
		0, 0, 0, 0,50,50,50, 0, 0, 0,
		0, 0, 0, 0,50,50,50, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	float ref_image_init[10 * 10] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,10,30,30, 0, 0,
		0, 0, 0, 0, 0,50,50,50, 0, 0,
		0, 0, 0, 0, 0,50,50,50, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	const C3DBounds size(10,10, 10);
	
	PSplineKernel kernel = produce_spline_kernel("bspline:d=3"); 

	C3DSplineTransformation trans(size, kernel, ipf, P3DSplineTransformPenalty());

	C3DFImage *psrc = new C3DFImage(size); 
	P3DImage src(psrc);
	
	copy(src_image_init, src_image_init + 100, psrc->begin_at(0,0,4)); 
	

	C3DFVectorfield field(C3DBounds(5,4,4)); 

	C3DFVector shift(1,2,1); 
	fill(field.begin(), field.end(), shift); 
	trans.set_coefficients(field);
	
	P3DImage result = trans(*src);
	
	const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result); 
	
	BOOST_REQUIRE(r.get_size() == size); 
	float *itest = ref_image_init; 
	auto ir = r.begin_at(0,0,5); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ir, ++itest) {
			cvinfo() << x << "," << y << ", " << *ir << " ?= " <<*itest << "\n"; 
			if (*itest > 0.001 || *ir > 0.001) 
				BOOST_CHECK_CLOSE(*ir, *itest, 0.1); 
		}
}



struct TransformSplineFixtureFieldBase: public ipfFixture {
	TransformSplineFixtureFieldBase():
		size(16,16,16),
		field(size),
		kernel(produce_spline_kernel("bspline:d=3")), 
		range(16, 16,16),
		stransf(range, kernel, ipf, P3DSplineTransformPenalty()),
		scale(1.0 / range.x, 1.0 / range.y, 1.0 / range.z)
	{

	}
	void init() {
		auto i = field.begin();
		for (int z = 0; z < (int)size.z; ++z)
			for (int y = 0; y < (int)size.y; ++y)
				for (int x = 0; x < (int)size.x; ++x, ++i) {
					float sx = scale.x * (x - size.x / 2.0);
					float sy = scale.y * (y - size.y / 2.0);
					float sz = scale.z * (z - size.z / 2.0);
					*i = C3DFVector( fx(sx, sy, sz), 
							 fy(sx, sy, sz),
							 fz(sx, sy, sz)); 
					cvdebug() <<"DIV[" << x << ", " << y << ", "<< z
						  << "]:(" << sx << "," << sy << ", " << sz << ")=" << *i << "\n";
				}

		stransf.set_coefficients(field);
		stransf.reinit();
	}
	C3DBounds size;
	C3DFVectorfield field;
	PSplineKernel kernel; 

	C3DBounds range;
	C3DSplineTransformation  stransf;

protected:
	virtual float fx(float x, float y, float z) = 0;
	virtual float fy(float x, float y, float z) = 0;
	virtual float fz(float x, float y, float z) = 0;
private:
	C3DFVector scale;
};



class Cost3DMock {
public: 
	Cost3DMock(const C3DBounds& size); 
	double value(const C3DTransformation& t) const;  
	double value_and_gradient(C3DFVectorfield& gradient) const;
	
	double src_value(const C3DFVector& x)const; 
	double ref_value(const C3DFVector& x)const; 
	C3DFVector src_grad(const C3DFVector& x)const; 
	C3DBounds m_size; 
	C3DFVector m_center; 
	float m_r; 
}; 

class TransformGradientFixture: public ipfFixture {
public: 
	TransformGradientFixture(); 

	void run_test(C3DTransformation& t, double tol=0.1)const; 

	C3DBounds size; 
	Cost3DMock cost; 
	PSplineKernel kernel; 
	C3DFVector x; 
	C3DFVectorfield gradient; 
}; 

BOOST_FIXTURE_TEST_CASE (test_spline_set_parameter, ipfFixture) 
{
	C3DBounds size(20,30,25); 
	PSplineKernel kernel(produce_spline_kernel("bspline:d=3")); 
	C3DSplineTransformation t(size, kernel, C3DFVector(5.0,5.0,5.0), ipf, P3DSplineTransformPenalty());
	auto params = t.get_parameters();
	
	params[0] = 1.0; 
	t.set_parameters(params);
	
	auto i = t.begin();
	for ( size_t z = 0; z < size.z; ++z) 
		for ( size_t y = 0; y < size.y; ++y) 
			for ( size_t x = 0; x < size.x; ++x, ++i) {
				cvdebug()<< x << ", " << y << *i << "\n"; 
			}
}


BOOST_FIXTURE_TEST_CASE (test_3d_cost_mock, ipfFixture ) 
{
	C3DBounds size(2,3,4);
	Cost3DMock cost(size); 
	
	C3DFVector x(1.5,2.3,3.7); 

	C3DFVector test_grad = cost.src_grad(x); 
	
	C3DFVector fd_grad; 
	const float delta = 0.001; 

	fd_grad.x = (cost.src_value(x + C3DFVector(delta, 0, 0)) - 
		     cost.src_value(x - C3DFVector(delta, 0, 0))) / (2 * delta); 

	fd_grad.y = (cost.src_value(x + C3DFVector(0.0, delta, 0.0)) - 
		     cost.src_value(x - C3DFVector(0.0, delta, 0.0))) / (2 * delta); 
	
	fd_grad.z = (cost.src_value(x + C3DFVector(0.0, 0.0, delta)) - 
		     cost.src_value(x - C3DFVector(0.0, 0.0, delta))) / (2 * delta); 

	
	BOOST_CHECK_CLOSE(test_grad.x, fd_grad.x, 0.1); 
	BOOST_CHECK_CLOSE(test_grad.y, fd_grad.y, 0.1); 
	BOOST_CHECK_CLOSE(test_grad.z, fd_grad.z, 0.1); 
	

}

BOOST_FIXTURE_TEST_CASE (test_spline_Gradient, TransformGradientFixture) 
{
	C3DSplineTransformation t(size, kernel, ipf, P3DSplineTransformPenalty());
	C3DFVectorfield coefs(C3DBounds(12,12,12)); 
	t.set_coefficients(coefs); 
	t.reinit(); 

	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.1; 



	for (size_t z = 0; z < t.get_coeff_size().z; z += 4)
		for(size_t y = 0; y < t.get_coeff_size().y; y +=4)
			for(size_t x = 0; x < t.get_coeff_size().x; x+=4) {
				auto ofs = 3 * ((z * t.get_coeff_size().y + y) * t.get_coeff_size().x + x); 
				auto itrg =  trgrad.begin() + ofs; 
				auto iparam = params.begin() + ofs;
				for (size_t i = 0; i < 3; ++i, ++itrg, ++iparam) {
					*iparam += delta; 
					t.set_parameters(params);
					double cost_plus = cost.value(t);
					*iparam -= 2*delta; 
					t.set_parameters(params);
					double cost_minus = cost.value(t);
					*iparam += delta; 
					double test_val = (cost_plus - cost_minus)/ (2*delta); 
					
					cvdebug() << z << ", " << y << ", " << x << ", " << i <<": got " 
						  << *itrg << " expect " << test_val << " Q: " 
						  << *itrg / test_val 
						  << "\n"; 
					
					BOOST_CHECK_CLOSE(*itrg, test_val,1); 
				}
			}
}



TransformGradientFixture::TransformGradientFixture():
	size(10,10,10), 
	cost(size),
	kernel(produce_spline_kernel("bspline:d=3")), 
	x(11,16,22), 
	gradient(size)

{
	cost.value_and_gradient(gradient);
}

void TransformGradientFixture::run_test(C3DTransformation& t, double tol)const
{
	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.01; 

	int n_close_zero = 0; 
	int n_zero = 0; 
	for(auto itrg =  trgrad.begin(), 
		    iparam = params.begin(); itrg != trgrad.end(); ++itrg, ++iparam) {
		*iparam += delta; 
		t.set_parameters(params);
		double cost_plus = cost.value(t);
		*iparam -= 2*delta; 
		t.set_parameters(params);
		double cost_minus = cost.value(t);
		*iparam += delta; 
		cvdebug() << cost_plus << ", " << cost_minus << "\n"; 

		double test_val = (cost_plus - cost_minus)/ (2*delta); 
		cvdebug() << *itrg << " vs " << test_val << "\n"; 
		if (fabs(*itrg) < 1e-8 && fabs(test_val) < 1e-8) {
			n_close_zero++; 
			continue; 
		}
		if (*itrg == 0.0 && fabs(test_val) < 1e-7) {
			n_zero++; 
			continue; 
		}

		BOOST_CHECK_CLOSE(*itrg, test_val, tol); 
	}
	cvmsg() << "value pairs < 1e-8 = " << n_close_zero << "\n"; 
	cvmsg() << "grad value zero, but finite difference below 1e-7 = " << n_zero << "\n"; 
}

Cost3DMock::Cost3DMock(const C3DBounds& size):
	m_size(size), 
	m_center(0.5 * size.x, 0.5 * size.y, 0.5 * size.z),
	m_r(sqrt(m_center.x * m_center.x + 
		  m_center.y * m_center.y + 
		  m_center.z * m_center.z))
{
	m_r *= m_r * m_r; 
}
	
double Cost3DMock::value(const C3DTransformation& t) const
{
	assert(m_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t z = 0; z < m_size.z; ++z) 
		for (size_t y = 0; y < m_size.y; ++y) 
			for (size_t x = 0; x < m_size.x; ++x, ++it) {
				double v = src_value(*it) - ref_value(C3DFVector(x,y,z)); 
				result += v * v; 
			}
	return result; 
		
}

double Cost3DMock::value_and_gradient(C3DFVectorfield& gradient) const
{
	assert(gradient.get_size() == m_size); 
	
	double result = 0.0; 

	auto ig = gradient.begin(); 
	for (size_t z = 0; z < m_size.z; ++z) 
		for (size_t y = 0; y < m_size.y; ++y) 
			for (size_t x = 0; x < m_size.x; ++x, ++ig) {
				C3DFVector pos(x,y,z);
				double v = src_value(pos) - ref_value(pos); 
				result += v * v; 
				*ig = 2.0 * v * src_grad(pos);  
			}
	return result; 
}

double Cost3DMock::src_value(const C3DFVector& x)const
{
	const C3DFVector p = x - m_center; 
	return 10000.0 * exp(- (p.x * p.x + p.y * p.y + p.z * p.z) / m_r); 
}

C3DFVector Cost3DMock::src_grad(const C3DFVector& x)const
{
	
	return - 2.0f / m_r * (x-m_center) * src_value(x); 
}

double Cost3DMock::ref_value(const C3DFVector& x) const 
{
	const C3DFVector p = x - m_center - C3DFVector(12.0,7.0, 2.0); 
	return 10000.0 * exp( - (p.x * p.x + p.y * p.y  + p.z * p.z) / m_r); 
}






/////////////////////////////////////
struct TransformSplineFixtureFieldBase2 : public ipfFixture{
	TransformSplineFixtureFieldBase2():
		kernel(produce_spline_kernel("bspline:d=4")), 
		range(1.0), 
		graddiv2sum(0.0)
	{

	}
	void init(int dsize, int r) {
		size = C3DBounds(2 * dsize + 1,2 * dsize + 1,2 * dsize + 1);
		C3DFVectorfield field(size);
		range = r; 
		scale.x = scale.y = scale.z = range / dsize;


		field_range.x = 2 * range; 
		field_range.y = 2 * range; 
		field_range.z = 2 * range; 

		auto t = new C3DSplineTransformation(C3DBounds(field_range), kernel, ipf, P3DSplineTransformPenalty()); 
		transform.reset(t);
		
		graddiv2sum = 0.0; 
		auto i = field.begin();
		for (int z = -dsize; z <= dsize; ++z)
			for (int y = -dsize; y <= dsize; ++y)
				for (int x = -dsize; x <= dsize; ++x, ++i) {
					double sx = x * scale.x;
					double sy = y * scale.y;
					double sz = z * scale.z;
					*i = C3DFVector( fx(sx, sy, sz), fy(sx, sy, sz), fz(sx, sy, sz));
					cvdebug() << "(" << sx << ", " << sy << ", " << sz << *i << "\n"; 

					graddiv2sum += graddiv2(sx, sy, sz); 
				}

		
		cvinfo() << "scale = " << scale << "\n"; 		
		graddiv2sum *= scale.x * scale.y * scale.z; 
		t->set_coefficients_and_prefilter(field); 
	}
	C3DBounds size;

	PSplineKernel kernel;
	C3DFVector field_range;
	double range; 
	P3DTransformation transform; 
protected:
	virtual double fx(double x, double y, double z)const  = 0;
	virtual double fy(double x, double y, double z)const  = 0;
	virtual double fz(double x, double y, double z)const  = 0;
	virtual double graddiv2(double x, double y, double z)const = 0;
	C3DFVector scale;
	double graddiv2sum; 
private:

};

struct TransformSplineFixtureMixed: public TransformSplineFixtureFieldBase2 {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
	double graddiv2(double x, double y, double z)const;
};

double TransformSplineFixtureMixed::fx(double x, double y, double z)const 
{
	return x * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixed::fy(double x, double y, double z)const 
{
	return y * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixed::fz(double , double , double )const
{
	return 0.0;
}

double TransformSplineFixtureMixed::graddiv2(double , double , double )const
{
	return 0.0; 
}


BOOST_AUTO_TEST_CASE(check_small_range_throw) 
{
	BOOST_CHECK_THROW(C3DSplineTransformation(C3DBounds(10,10,10), 
						  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
						  C3DFVector(10,2,10),  C3DInterpolatorFactory("bspline:d=3", "mirror"), 
						  P3DSplineTransformPenalty()), invalid_argument); 

	BOOST_CHECK_THROW(C3DSplineTransformation(C3DBounds(10,10,10), 
						  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
						  C3DFVector(2,10,10),  C3DInterpolatorFactory("bspline:d=3", "mirror"), 
						  P3DSplineTransformPenalty()), invalid_argument); 

	BOOST_CHECK_THROW(C3DSplineTransformation(C3DBounds(10,10,10), 
						  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
						  C3DFVector(10,10,2),  C3DInterpolatorFactory("bspline:d=3", "mirror"), 
						  P3DSplineTransformPenalty()), invalid_argument); 

}


BOOST_AUTO_TEST_CASE(check_small_field_throw) 
{
	C3DSplineTransformation t(C3DBounds(10,10,10), 
				  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
				  C3DInterpolatorFactory("bspline:d=3", "mirror"), 
				  P3DSplineTransformPenalty()); 
	
		
	BOOST_CHECK_THROW(t.set_coefficients(C3DFVectorfield(C3DBounds(3,10,5))), invalid_argument); 
	BOOST_CHECK_THROW(t.set_coefficients(C3DFVectorfield(C3DBounds(10,3,5))), invalid_argument); 
	BOOST_CHECK_THROW(t.set_coefficients(C3DFVectorfield(C3DBounds(10,10,3))), invalid_argument); 

}



