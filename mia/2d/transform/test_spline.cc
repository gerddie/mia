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
#include <mia/2d/interpolator.hh>
#include <mia/2d/transform/spline.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct PathInitializer {
	PathInitializer() {
		list< bfs::path> kernelsearchpath;
		kernelsearchpath.push_back(bfs::path("../../core/spacialkernel"));
		C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);
	}
}; 

PathInitializer lala; 

struct TransformSplineFixture {
	TransformSplineFixture():
		size(32,64),
		field(size),
		ipf(new C2DInterpolatorFactory(C2DInterpolatorFactory::ip_spline,
					       SHARED_PTR(CBSplineKernel) (new CBSplineKernel3()))),
		range(64, 128),
		r(range.x - 1, range.y - 1),
		stransf(range, ipf), 
		scale(2 * M_PI / r.x, 2 * M_PI / r.y)
	{
		C2DFVector ivscale(float(range.x - 1) / (size.x - 1),
				   float(range.y - 1) / (size.y - 1));
		C2DFVectorfield::iterator i = field.begin();
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				float sx = ivscale.x * x;
				float sy = ivscale.y * y;
				*i = C2DFVector( fx(sx, sy), fy(sx, sy));
			}

		SHARED_PTR(T2DInterpolator<C2DFVector> ) source(ipf->create(field));

		stransf.set_coefficients(field);
		stransf.reinit();
	}
	C2DBounds size;
	C2DFVectorfield field;
	P2DInterpolatorFactory ipf;

	C2DBounds range;
	C2DBounds r;
	C2DSplineTransformation  stransf;

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

	
private: 
	C2DFVector scale; 
};



float TransformSplineFixture::fx(float x, float y)
{

	x *= scale.x;
	y *= scale.y;
	return 	(1.0 + sinf(x - M_PI / 2.0)) * (1.0 + sinf(2 * y  - M_PI / 2.0));
}

float TransformSplineFixture::fy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return (1.0 - cosf(2 * x)) * (1.0 - cosf(y));
}


float TransformSplineFixture::dfx_x(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return scale.x * cosf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}

float TransformSplineFixture::dfx_y(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 4.0 * M_PI / r.y * (1.0 + sinf(x - M_PI / 2.0)) * cosf(2 * y  - M_PI / 2.0);
}

float TransformSplineFixture::dfy_x(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 4 * M_PI / r.x * sinf(2 * x) * (1.0 - cosf(y));
}

float TransformSplineFixture::dfy_y(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return scale.y * (1.0 - cosf(2 * x)) * sinf(y);
}


BOOST_FIXTURE_TEST_CASE( test_splines_transformation, TransformSplineFixture )
{
	BOOST_CHECK_EQUAL(stransf.degrees_of_freedom(), field.size() * 2);

	C2DFVector testx(33.4, 100.8);
	C2DFVector result = stransf.apply(testx);

	BOOST_CHECK_EQUAL(stransf.get_size(), range);

	C2DFVector scaledx = stransf.scale(testx);
	BOOST_CHECK_CLOSE(scaledx.x, testx.x * (size.x-1) / (range.x-1), 0.1);
	BOOST_CHECK_CLOSE(scaledx.y, testx.y * (size.y-1) / (range.y-1), 0.1);


	BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y), 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y),0.1);

	result = stransf(testx);
	BOOST_CHECK_CLOSE(result.x, testx.x - fx(testx.x, testx.y), 0.1);
	BOOST_CHECK_CLOSE(result.y, testx.y - fy(testx.x, testx.y),0.1);



}

#if 0 
BOOST_FIXTURE_TEST_CASE( test_splines_transformation_curl, TransformSplineFixture )
{
	double curl = 0.0; 
	for (size_t y = 1; y < range.y-1; ++y)
		for (size_t x = 1; x < range.x-1; ++x) {
			const float lcurl = dfx_y(x,y) - dfy_x(x,y); 
			curl += lcurl * lcurl; 
		}

	BOOST_CHECK_CLOSE(stransf.curl(), curl/ ((range.x - 2) * (range.y-2)), 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_splines_transformation_divergence, TransformSplineFixture )
{
	double div = 0.0; 
	for (size_t y = 1; y < range.y-1; ++y)
		for (size_t x = 1; x < range.x-1; ++x) {
			const float dfxx = dfx_x(x,y); 
			const float dfyy = dfy_y(x,y); 
			div += dfxx * dfxx + dfyy * dfyy; 
		}

	BOOST_CHECK_CLOSE(stransf.curl(), div / ((range.x - 2) * (range.y-2)), 0.1);
}
#endif

BOOST_FIXTURE_TEST_CASE( test_splines_transformation_upscale, TransformSplineFixture )
{
	const C2DBounds scale(2.0, 3.0);
	C2DBounds new_range = scale * range;

	const C2DFVector fscale((new_range.x - 1.0) / (range.x - 1.0),
				(new_range.y - 1.0) / (range.y - 1.0));

	cvdebug() << fscale << "\n";
	P2DTransformation  stransf_upscaled = stransf.upscale(new_range);

	stransf_upscaled->reinit();

	C2DFVector test2(33.4, 90.3);

	C2DFVector result2 = stransf_upscaled->apply(fscale * test2);

	BOOST_CHECK_CLOSE(result2.x, scale.x * fx(test2.x, test2.y), 0.1);
	BOOST_CHECK_CLOSE(result2.y, scale.y * fy(test2.x, test2.y), 0.1);

}

BOOST_FIXTURE_TEST_CASE( test_splines_add, TransformSplineFixture )
{
	stransf.reinit();
	C2DFVector testx(40.4, 62.8);

	C2DFVector r1( testx.x - fx(testx.x, testx.y), testx.y - fy(testx.x, testx.y));
	C2DFVector r2( r1.x - fx(r1.x,r1.y), r1.y -fy(r1.x, r1.y));

	stransf.add(stransf);
	stransf.reinit();
	C2DFVector result = stransf(testx);
	BOOST_CHECK_CLOSE(result.x, r2.x, 0.1);
	BOOST_CHECK_CLOSE(result.y, r2.y, 0.1);

}

BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator, TransformSplineFixture )
{
	C2DSplineTransformation::const_iterator i = stransf.begin();

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			BOOST_CHECK_CLOSE(1.0 + x - fx(x,y), 1.0 + i->x, 0.1);
			BOOST_CHECK_CLOSE(1.0 + y - fy(x,y), 1.0 + i->y, 0.1);
		}
}


BOOST_FIXTURE_TEST_CASE( test_splinestransform_postfix_iterator, TransformSplineFixture )
{
	C2DSplineTransformation::const_iterator i = stransf.begin();
	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x) {
			C2DFVector test = *i++;
			BOOST_CHECK_CLOSE(1.0 + x - fx(x,y), 1.0 + test.x, 0.1);
			BOOST_CHECK_CLOSE(1.0 + y - fy(x,y), 1.0 + test.y, 0.1);
		}
}


BOOST_FIXTURE_TEST_CASE( test_splines_deform, TransformSplineFixture )
{

	C2DFImage image(range);
	C2DFImage::iterator i = image.begin();
	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			*i = (x + 1) * (y + 1);
		}

	C2DFImage test_image(range);

	auto_ptr<T2DInterpolator<float> > src(ipf->create(image.data()));

	C2DFImage::iterator t = test_image.begin();

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++t) {
			const C2DFVector v(x,y);
			*t = (*src)(stransf(v));
		}

	P2DImage stransfed = stransf(image, *ipf);
	const C2DFImage& ptransformed = dynamic_cast<const C2DFImage&>(*stransfed);

	BOOST_CHECK_EQUAL(ptransformed.get_size(), image.get_size());
	BOOST_REQUIRE(ptransformed.get_size() == image.get_size());

	BOOST_CHECK_EQUAL(ptransformed.get_pixel_type(), image.get_pixel_type());

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			BOOST_CHECK_CLOSE(ptransformed(x,y), test_image(x,y), 0.1);
		}


}


BOOST_FIXTURE_TEST_CASE( test_splines_translate, TransformSplineFixture )
{
	C2DFVectorfield gradient(C2DBounds(128,128));
	fill(gradient.begin(), gradient.end(), C2DFVector(1.0, 2.0));

	gsl::DoubleVector force(2 * stransf.get_coeff_size().x * stransf.get_coeff_size().y); 
	stransf.translate(gradient, force);

	auto  i = force.begin();
	while ( i != force.end() ) {
		BOOST_CHECK_CLOSE(*i++, 1.0f, 0.1);
		BOOST_CHECK_CLOSE(*i++, 2.0f, 0.1);
	}
}

BOOST_FIXTURE_TEST_CASE( test_splines_clone, TransformSplineFixture )
{
	P2DTransformation clone(stransf.clone());

	C2DSplineTransformation& spclone = dynamic_cast<C2DSplineTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), stransf.get_size());
	BOOST_CHECK_EQUAL(spclone.get_coeff_size(), stransf.get_coeff_size());

	C2DSplineTransformation::const_iterator ic = spclone.begin();
	C2DSplineTransformation::const_iterator ec = spclone.end();

	C2DSplineTransformation::const_iterator io = stransf.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}

BOOST_FIXTURE_TEST_CASE( test_splines_update, TransformSplineFixture )
{
	C2DFVectorfield update(stransf.get_coeff_size());
	for (unsigned int y = 0; y < update.get_size().y; ++y)
		for (unsigned int x = 0; x < update.get_size().x; ++x) {
			update(x,y) = C2DFVector(1.0, 2.0);
		}

	stransf.update(2.0, update);
	stransf.reinit();

	C2DFVector testx(33.4, 82.4);
	C2DFVector result = stransf.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y) + 2.0f, 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y) + 4.0f, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_splines_gridpoint_derivative, TransformSplineFixture )
{
	C2DFVector x(33,80);
	C2DFMatrix dv =  stransf.derivative_at(33,80);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x.x, x.y), 0.2);
}

BOOST_FIXTURE_TEST_CASE(test_splines_gridpoint_derivative_new, TransformSplineFixture)
{


	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x) {
			C2DFMatrix dv =  stransf.derivative_at(x, y);
			BOOST_CHECK_CLOSE(1.0 + dv.x.x, 2.0 - dfx_x(x, y), 0.1);
			BOOST_CHECK_CLOSE(1.0 + dv.x.y, 1.0 - dfy_x(x, y), 0.1);
			BOOST_CHECK_CLOSE(1.0 + dv.y.x, 1.0 - dfx_y(x, y), 0.1);
			BOOST_CHECK_CLOSE(1.0 + dv.y.y, 2.0 - dfy_y(x, y), 0.1);
		}

}

BOOST_FIXTURE_TEST_CASE( test_splines_set_identity, TransformSplineFixture )
{
	stransf.set_identity();

	C2DSplineTransformation::const_iterator i = stransf.begin();
	for(size_t y = 0; y < range.y; ++y)
		for(size_t x = 0; x < range.x; ++x, ++i) {
			BOOST_CHECK_EQUAL( *i, C2DFVector(x,y));
		}
}

BOOST_FIXTURE_TEST_CASE( test_splines_get_max_transform, TransformSplineFixture )
{
	float fx0 = fx(48,64);
	float fy0 = fy(48,64);

	BOOST_CHECK_CLOSE(sqrt(fx0*fx0 + fy0*fy0), stransf.get_max_transform(), 1);
}

BOOST_FIXTURE_TEST_CASE( test_splines_pertuberate, TransformSplineFixture )
{
	C2DFVectorfield v(size);
	C2DFVector vv(1.0, 2.0);
	float scalex = float(range.x - 1.0)  / (size.x - 1.0);
	float scaley = float(range.y - 1.0)  / (size.y - 1.0);


	fill(v.begin(), v.end(), vv);

	// this location is hand-picked and is not really the position ofthe maximun
	// but only an approximation
	float gamma = stransf.pertuberate(v);
	C2DFVector lmg(12* scalex, 27 * scaley);
	C2DFVector mg(vv.x - vv.x * dfx_x(lmg.x,lmg.y) - vv.y * dfx_y(lmg.x,lmg.y),
		      vv.y - vv.x * dfy_x(lmg.x,lmg.y) - vv.y * dfy_y(lmg.x,lmg.y));

	BOOST_CHECK_CLOSE(gamma, mg.norm(), 0.1);

	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x) {
			const C2DFVector& iv = v(x,y);
			const float sx = x * scalex;
			const float sy = y * scaley;

			BOOST_CHECK_CLOSE(iv.x, vv.x - vv.x * dfx_x(sx,sy) - vv.y * dfx_y(sx,sy), 0.1);
			BOOST_CHECK_CLOSE(iv.y, vv.y - vv.x * dfy_x(sx,sy) - vv.y * dfy_y(sx,sy), 0.1);
		}
}


BOOST_FIXTURE_TEST_CASE( test_splines_get_jacobian, TransformSplineFixture )
{
	C2DFVectorfield v(size);
	fill(v.begin(), v.end(), C2DFVector(10.0,20.0));

	// this is not the location of the minimal value, we onyl approximate
	// by using the minimal value on the coefficient grid
	float scalex = float(range.x - 1.0)  / (size.x - 1.0);
	float scaley = float(range.y - 1.0)  / (size.y - 1.0);

	C2DFVector lmg(8*scalex, 16*scaley);
	float j = stransf.get_jacobian(v, 1.0);
	C2DFMatrix J = stransf.derivative_at(lmg.x, lmg.y);

	BOOST_CHECK_CLOSE(j, J.x.x * J.y.y - J.x.y * J.y.x, 0.1);
}

BOOST_AUTO_TEST_CASE( test_spline_c_rate_create )
{
	P2DInterpolatorFactory ipf(new C2DInterpolatorFactory(C2DInterpolatorFactory::ip_spline,
							      SHARED_PTR(CBSplineKernel) (new CBSplineKernel4())));

	C2DBounds size(20, 32);
	C2DFVector c_rate(2.5, 3.2);
	C2DSplineTransformation  stransf(size, ipf, c_rate);

	C2DBounds gridsize = stransf.get_coeff_size();
	BOOST_CHECK_EQUAL(gridsize.x, 8);
	BOOST_CHECK_EQUAL(gridsize.y, 10);

	C2DSplineTransformation::const_iterator i = stransf.begin();
	for(size_t y = 0; y < size.y; ++y)
		for(size_t x = 0; x < size.x; ++x, ++i) {
			BOOST_CHECK_EQUAL( *i, C2DFVector(x,y));
		}

}


BOOST_FIXTURE_TEST_CASE( test_splines_refine, TransformSplineFixture )
{
	// decrease c-rate
	C2DBounds new_size(40,50);

	auto_ptr<C2DSplineTransformation> refined(stransf.refine(new_size));

	C2DSplineTransformation::const_iterator i = refined->begin();

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			BOOST_CHECK_CLOSE(1.0 + x - fx(x,y), 1.0 + i->x, 0.1);
			BOOST_CHECK_CLOSE(1.0 + y - fy(x,y), 1.0 + i->y, 0.1);
		}
}


BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_grad_curl, TransformSplineFixture )
{
	double gradcurl = 0.0; 
	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x) {
			const double gdfx_xy = dfx_xy(x,y); 
			const double gdfx_yy = dfx_yy(x,y);
			const double gdfy_xx = dfy_xx(x,y); 
			const double gdfy_xy = dfy_xy(x,y);
			const double v = gdfy_xx + gdfy_xy - gdfx_yy - gdfx_xy; 
			gradcurl += v * v; 
		}

	
	BOOST_CHECK_CLOSE(stransf.grad_curl(), gradcurl, 1.0);
}



BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_grad_divergence, TransformSplineFixture )
{
	double graddiv = 0.0; 
	for (size_t y = 1; y < range.y-1; ++y)
		for (size_t x = 1; x < range.x-1; ++x) {
			const double gdfxx_x = dfx_xx(x,y); 
			const double gdfxx_y = dfx_xy(x,y);
			const double gdfyy_x = dfy_yx(x,y); 
			const double gdfyy_y = dfy_yy(x,y);
			const double v = gdfxx_x + gdfxx_y + gdfyy_x + gdfyy_y; 
			graddiv += v * v;
		}

	BOOST_CHECK_CLOSE(stransf.grad_divergence(), graddiv, 1);
}


float TransformSplineFixture::dfx_xx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return - scale.x * scale.x * sinf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}
	
float TransformSplineFixture::dfx_xy(float x, float y)

{
	x *= scale.x;
	y *= scale.y;
	return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float TransformSplineFixture::dfx_yx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float TransformSplineFixture::dfx_yy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -4.0 * scale.y * scale.y * (1.0 + sinf(x - M_PI / 2.0)) * sinf(2 * y  - M_PI / 2.0);
}

float TransformSplineFixture::dfy_xx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -4.0 * scale.x * scale.x * cosf(2 * x) * (1.0 - cosf(y));
}

float TransformSplineFixture::dfy_xy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -2.0 * scale.x * scale.y * sinf(2 * x) * sinf(y);
}

float TransformSplineFixture::dfy_yx(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return -2.0 * scale.y * scale.x * sinf(2 * x) * sinf(y);
}

float TransformSplineFixture::dfy_yy(float x, float y)
{
	x *= scale.x;
	y *= scale.y;
	return - scale.y * scale.y * (1.0 - cosf(2 * x)) * cosf(y);
}

struct TransformSplineFixtureFieldBase {
	TransformSplineFixtureFieldBase():
		size(16,16),
		field(size),
		ipf(new C2DInterpolatorFactory(C2DInterpolatorFactory::ip_spline,
					       SHARED_PTR(CBSplineKernel) (new CBSplineKernel3()))),
		range(16, 16),
		stransf(range, ipf), 
		scale(1.0 / range.x, 1.0 / range.y)
	{

	}
	void init() {
		C2DFVectorfield::iterator i = field.begin();
		for (int y = 0; y < (int)size.y; ++y)
			for (int x = 0; x < (int)size.x; ++x, ++i) {
				float sx = scale.x * (x - size.x / 2.0);
				float sy = scale.y * (y - size.y / 2.0);
				*i = C2DFVector( fx(sx, sy), fy(sx, sy)); 
				cvdebug() <<"DIV[" << x << ", " << y 
					  << "]:(" << sx << "," << sy << ")=" << *i << "\n"; ;
			}

		SHARED_PTR(T2DInterpolator<C2DFVector> ) source(ipf->create(field));

		stransf.set_coefficients(field);
		stransf.reinit();
	}
	C2DBounds size;
	C2DFVectorfield field;
	P2DInterpolatorFactory ipf;

	C2DBounds range;
	C2DSplineTransformation  stransf;

protected:
	virtual float fx(float x, float y) = 0;
	virtual float fy(float x, float y) = 0;
private: 
	C2DFVector scale; 
};

struct TransformSplineFixtureConstDivergence: public TransformSplineFixtureFieldBase {
	virtual float fx(float x, float y);
	virtual float fy(float x, float y);
}; 

BOOST_FIXTURE_TEST_CASE( test_gridtransform_constdivonly_get_grad_divcurl, TransformSplineFixtureConstDivergence )
{
	init(); 
	BOOST_CHECK_CLOSE(0.0, stransf.grad_curl(), 1.0);
	BOOST_CHECK_CLOSE(0.0, stransf.grad_divergence(),  1);
}



float TransformSplineFixtureConstDivergence::fx(float x, float /*y*/)
{
	return x; 
}

float TransformSplineFixtureConstDivergence::fy(float  /*x*/, float y)
{
	return y; 
}


struct TransformSplineFixtureConstCurl: public TransformSplineFixtureFieldBase {
	virtual float fx(float x, float y);
	virtual float fy(float x, float y);
}; 


BOOST_FIXTURE_TEST_CASE( test_gridtransform_constcurlonly_get_grad_curl_div, TransformSplineFixtureConstCurl )
{
	init(); 
	BOOST_CHECK_CLOSE(0.0, stransf.grad_curl(), 1.0);
	BOOST_CHECK_CLOSE(0.0, stransf.grad_divergence(), 1.0);
}


float TransformSplineFixtureConstCurl::fx(float /*x*/, float y)
{
	return -y; 
}

float TransformSplineFixtureConstCurl::fy(float  x, float /*y*/)
{
	return x; 
}

