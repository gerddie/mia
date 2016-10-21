/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/2d/interpolator.hh>
#include <mia/2d/transform/spline.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct TransformSplineFixture {
	TransformSplineFixture():
		size(33,65),
		kernel(CSplineKernelPluginHandler::instance().produce("bspline:d=3")),
		range(33, 65),
		r(range.x - 1, range.y - 1),
		stransf(range, kernel, C2DInterpolatorFactory("bspline:d=3", "mirror"), P2DSplineTransformPenalty()),
		scale(2 * M_PI / r.x, 2 * M_PI / r.y)
	{
		coeff_shift = kernel->get_active_halfrange() - 1; 
		C2DFVector ivscale(float(range.x - 1) / (size.x - 1),
				   float(range.y - 1) / (size.y - 1));
		size.x += 2*coeff_shift; 
		size.y += 2*coeff_shift; 
		field = C2DFVectorfield(size); 

		C2DFVectorfield::iterator i = field.begin();
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				float sx = ivscale.x * (float(x) - coeff_shift);
				float sy = ivscale.y * (float(y) - coeff_shift);
				*i = C2DFVector( fx(sx, sy), fy(sx, sy));
			}
		vector<C2DFVector> buffer(size.y); 
		C2DFVectorfield help1(size);
		auto bc = produce_spline_boundary_condition("mirror"); 
		bc->set_width(size.y); 
		for(size_t x = 0; x < size.x; ++x) {
			field.get_data_line_y(x, buffer); 
			bc->filter_line(buffer, kernel->get_poles()); 
			help1.put_data_line_y(x, buffer); 
		}
		C2DFVectorfield help2(size);
		buffer.resize(size.x); 
		bc->set_width(size.x); 
		for(size_t y = 0; y < size.y; ++y) {
			help1.get_data_line_x(y, buffer); 
			bc->filter_line(buffer, kernel->get_poles()); 
			help2.put_data_line_x(y, buffer); 
		}

		stransf.set_coefficients(help2); 
	}
	C2DBounds size;
	C2DFVectorfield field;
	PSplineKernel kernel;

	C2DBounds range;
	C2DBounds r;
	C2DSplineTransformation  stransf;
	int coeff_shift; 
protected:
	float fx(float x, float y);
	float fy(float x, float y);
	float fx(const C2DFVector &x); 
	float fy(const C2DFVector &x); 


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
	float vx = x - range.x / 2.0; 
	float vy = y - range.y / 2.0;
	
	return 	exp ( - (6 * vx * vx + 5 * vy * vy) / (range.x * range.y) ) ;
}

float TransformSplineFixture::fy(float x, float y)
{
	float vx = x - range.x / 2.0; 
	float vy = y - range.y / 2.0;

	return 	exp ( - (4 * vx * vx + 8 * vy * vy) / (range.x * range.y) ) ;
}

float TransformSplineFixture::fx(const C2DFVector &x)
{
	return 	fx(x.x, x.y); 
}

float TransformSplineFixture::fy(const C2DFVector &x)
{
	return 	fy(x.x, x.y); 
}



float TransformSplineFixture::dfx_x(float x, float y)
{
	float vx = x - range.x / 2.0; 
	return - 12 * vx / (range.x * range.y) * fx(x,y); 
}

float TransformSplineFixture::dfx_y(float x, float y)
{
	float vy = y - range.y / 2.0; 
	return - 10 * vy / (range.x * range.y) * fx(x,y); 
}

float TransformSplineFixture::dfy_x(float x, float y)
{
	float vx = x - range.x / 2.0; 
	return 	 - 8 * vx / (range.x * range.y) * fy(x,y);
}

float TransformSplineFixture::dfy_y(float x, float y)
{
	float vy = y - range.y / 2.0; 
	return 	 - 16 * vy / (range.x * range.y) * fy(x,y);
}


BOOST_FIXTURE_TEST_CASE( test_splines_transformation, TransformSplineFixture )
{
	BOOST_CHECK_EQUAL(stransf.degrees_of_freedom(), field.size() * 2);

	C2DFVector testx(15, 20);
	C2DFVector result = stransf.apply(testx);

	BOOST_CHECK_EQUAL(stransf.get_size(), range);

	C2DFVector scaledx = stransf.scale(testx);
	BOOST_CHECK_CLOSE(scaledx.x, testx.x * (size.x-2*coeff_shift - 1) / (range.x-1)+coeff_shift, 0.1);
	BOOST_CHECK_CLOSE(scaledx.y, testx.y * (size.y-2*coeff_shift - 1) / (range.y-1)+coeff_shift, 0.1);


	BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y), 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y), 0.1);

	result = stransf(testx);
	BOOST_CHECK_CLOSE(result.x, testx.x - fx(testx.x, testx.y), 0.1);
	BOOST_CHECK_CLOSE(result.y, testx.y - fy(testx.x, testx.y), 0.1);

}

#define REVIEW_THIS_TEST
#ifdef REVIEW_THIS_TEST
BOOST_FIXTURE_TEST_CASE( test_splines_transformation_upscale, TransformSplineFixture )
{
	const C2DBounds scale(2.0, 3.0);
	C2DBounds new_range = scale * range;

	const C2DFVector fscale(float(new_range.x - 1.0) / (range.x - 1.0),
				float(new_range.y - 1.0) / (range.y - 1.0));

	cvdebug() << fscale << "\n";
	P2DTransformation  stransf_upscaled = stransf.upscale(new_range);

	C2DFVector test2(15.4, 20.3);

	C2DFVector result2 = stransf_upscaled->apply(fscale * test2);

	BOOST_CHECK_CLOSE(result2.x, fscale.x * fx(test2.x, test2.y), 0.1);
	BOOST_CHECK_CLOSE(result2.y, fscale.y * fy(test2.x, test2.y), 0.1);

}
#endif


BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator, TransformSplineFixture )
{
	
	auto i = stransf.begin();

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			cvdebug() << "x=" << x << " y=" << y << ", " << range << "\n"; 
			C2DFVector vf(fx(x,y), fy(x,y)); 
			C2DFVector tv(x - i->x, y - i->y); 
			if (std::fabs(vf.x) > 1e-8 && std::fabs(tv.x) > 1e-8) {
				BOOST_CHECK_CLOSE(fx(x,y), x - i->x, 0.25);
				BOOST_CHECK_CLOSE(fy(x,y), y - i->y, 0.25);
			}
		}
}

BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator2, TransformSplineFixture )
{
	
	auto i = stransf.begin();

	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++i) {
			C2DFVector help = stransf(C2DFVector(x,y));
			BOOST_CHECK_CLOSE(help.x, i->x, 0.1);
			BOOST_CHECK_CLOSE(help.y, i->y, 0.1);
		}
}


BOOST_FIXTURE_TEST_CASE( test_splinestransform_postfix_iterator, TransformSplineFixture )
{
	auto i = stransf.begin();
	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x) {
			C2DFVector test = *i++;
			cvdebug() << "splinestransform_postfix_iterator" << test << "\n"; 
			BOOST_CHECK_CLOSE(1.0 + x - fx(x,y), 1.0 + test.x, 0.2);
			BOOST_CHECK_CLOSE(1.0 + y - fy(x,y), 1.0 + test.y, 0.2);
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
	
	C2DFImage::iterator t = test_image.begin();
	C2DInterpolatorFactory ipf("bspline:d=3", "mirror"); 
	unique_ptr<T2DInterpolator<float> > src(ipf.create(image.data())); 


	for (size_t y = 0; y < range.y; ++y)
		for (size_t x = 0; x < range.x; ++x, ++t) {
			const C2DFVector v(x,y);
			*t = (*src)(stransf(v));
		}

	P2DImage stransfed = stransf(image);
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
	C2DFVectorfield gradient(range); 
	double scale_x = double(range.x - 1) / (size.x - 1 - 2*coeff_shift); 
	double scale_y = double(range.y - 1) / (size.y - 1 - 2*coeff_shift); 
	double scale = scale_x * scale_y; 
	
	cvdebug() << range << size << ", scale=" << scale << "\n"; 

	fill(gradient.begin(), gradient.end(), C2DFVector(1.0, 2.0));

	CDoubleVector force(stransf.degrees_of_freedom());
	stransf.translate(gradient, force);

	auto  i = force.begin();
	for (size_t y = 0; y < stransf.get_coeff_size().y; ++y)
		for (size_t x = 0; x < stransf.get_coeff_size().x; ++x, i+=2) {
			if (y > 3 && y < stransf.get_coeff_size().y - 3 && 
			    x > 3 && x < stransf.get_coeff_size().x - 3) {
				BOOST_CHECK_CLOSE( i[0], -1.0f * scale, 0.1);
				BOOST_CHECK_CLOSE( i[1], -2.0f * scale, 0.1);

			}
			
			
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

	C2DFVector testx(20.4, 42.4);
	C2DFVector result = stransf.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y) + 2.0f, 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y) + 4.0f, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_splines_gridpoint_derivative, TransformSplineFixture )
{
	C2DFVector x(20,41);
	C2DFMatrix dv =  stransf.derivative_at(x.x, x.y);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x.x, x.y), 0.2);
}

BOOST_FIXTURE_TEST_CASE( test_splines_derivative, TransformSplineFixture )
{
	C2DFVector x(20.4,41.7);
	C2DFMatrix dv =  stransf.derivative_at(x);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x.x, x.y), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x.x, x.y), 0.2);
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
	C2DFVector center((range.x - 1.0)/2.0, (range.y - 1.0)/2.0); 
	float fx0 = fx(center);
	float fy0 = fy(center);

	BOOST_CHECK_CLOSE(sqrt(fx0*fx0 + fy0*fy0), stransf.get_max_transform(), 2.0);
}

#if 0 
BOOST_FIXTURE_TEST_CASE( test_splines_pertuberate, TransformSplineFixture )
{
	C2DFVectorfield v(size);
	C2DFVector vv(1.0, 2.0);
	float scalex = float(range.x - 1.0)  / (size.x - 1.0);
	float scaley = float(range.y - 1.0)  / (size.y - 1.0);


	fill(v.begin(), v.end(), vv);

	// this location is hand-picked and is not really the position of the maximun
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
#endif

BOOST_AUTO_TEST_CASE( test_spline_c_rate_create )
{
	PSplineKernel kernel(CSplineKernelPluginHandler::instance().produce("bspline:d=3")); 
	C2DBounds size(20, 32);
	C2DFVector c_rate(2.5, 3.2);
	C2DSplineTransformation  stransf(size, kernel, c_rate, 	C2DInterpolatorFactory(kernel, "mirror"), P2DSplineTransformPenalty());

	C2DBounds gridsize = stransf.get_coeff_size();
	BOOST_CHECK_EQUAL(gridsize.x, 10u);
	BOOST_CHECK_EQUAL(gridsize.y, 12u);

	C2DSplineTransformation::const_iterator i = stransf.begin();
	for(size_t y = 0; y < size.y; ++y)
		for(size_t x = 0; x < size.x; ++x, ++i) {
			BOOST_CHECK_EQUAL( *i, C2DFVector(x,y));
		}

}


// the boundaries are not well interpolated after upscaling 
// which is kind of strange, since upscaling only means, multiplying 
// the coefficients by a scaling factor 
BOOST_FIXTURE_TEST_CASE( test_splines_upscale, TransformSplineFixture )
{
	C2DBounds new_range(2 * range.x - 1, 2 * range.y - 1); 

	P2DTransformation upscaled(stransf.upscale(new_range)); 
	C2DFVector rs(C2DFVector(new_range - C2DBounds::_1)/C2DFVector(range - C2DBounds::_1)); 

	auto i = upscaled->begin() + (2 * new_range.x + 2); 
	for (size_t y = 2; y < new_range.y - 2; ++y) {
		for (size_t x = 2; x < new_range.x -2 ; ++x, ++i) {
			cvdebug() << x << ", " << y << "\n"; 
			C2DFVector X(x,y); 
			C2DFVector f = C2DFVector(fx(X/rs), fy(X/rs)) * rs; 
			C2DFVector res = X - *i; 
			
			if (std::fabs(f.x > 1e-8) || std::fabs(res.x) > 1e-8) 
				BOOST_CHECK_CLOSE(res.x, f.x, 2);
			    
			if (std::fabs(f.y > 1e-8)|| std::fabs(res.y) > 1e-8) 
				BOOST_CHECK_CLOSE(res.y, f.y, 2);
		}
		i += 4; 
	}
}

// Make this test independend of the upscaling test 

BOOST_FIXTURE_TEST_CASE( test_splines_refine, TransformSplineFixture )
{
	size = C2DBounds(17,33); 
	
	C2DFVector ivscale(float(range.x - 1) / (size.x - 1),
			   float(range.y - 1) / (size.y - 1));

	
	size.x += 2*coeff_shift; 
	size.y += 2*coeff_shift; 
	C2DFVectorfield coefs(size); 
	
	auto i = coefs.begin();
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++i) {
			float sx = ivscale.x * (float(x) - coeff_shift);
			float sy = ivscale.y * (float(y) - coeff_shift);
			*i = C2DFVector( fx(sx, sy), fy(sx, sy));
		}
	
	stransf.set_coefficients(coefs);

	BOOST_CHECK(stransf.refine()); 
	auto k = stransf.begin(); 
	for (size_t y = 0; y < range.y; ++y) {
		for (size_t x = 0; x < range.x; ++x, ++k) {
			

			C2DFVector X(x,y); 
			C2DFVector f(fx(X), fy(X)); 
			
			cvdebug() << X << " have " << X - *k << " exp " << f <<"\n"; 
			
			BOOST_CHECK_CLOSE(f.x, x -  k->x, 2);
			BOOST_CHECK_CLOSE(f.y, y -  k->y, 2);
		}
	}
}


BOOST_AUTO_TEST_CASE( test_splines_transform )
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
	const C2DBounds size(10,10);
	
	PSplineKernel kernel = produce_spline_kernel("bspline:d=3"); 

	P2DInterpolatorFactory ipf(new C2DInterpolatorFactory(kernel, "mirror")); 

	C2DSplineTransformation trans(size, kernel, *ipf, P2DSplineTransformPenalty());

	P2DImage src(new C2DFImage(size, src_image_init));
	C2DFVectorfield field(C2DBounds(5,4)); 

	C2DFVector shift(1,2); 
	fill(field.begin(), field.end(), shift); 
	trans.set_coefficients(field);
	
	P2DImage result = trans(*src);
	
	const C2DFImage& r = dynamic_cast<const C2DFImage&>(*result); 
	
	BOOST_REQUIRE(r.get_size() == size); 
	float *itest = ref_image_init; 
	auto ir = r.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ir, ++itest) {
			cvinfo() << x << "," << y << ", " << *ir << " ?= " <<*itest << "\n"; 
			if (*itest > 0.001 || *ir > 0.001) 
				BOOST_CHECK_CLOSE(*ir, *itest, 0.1); 
		}
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
		kernel(CSplineKernelPluginHandler::instance().produce("bspline:d=3")), 
		ipf( new C2DInterpolatorFactory(kernel, 
						*produce_spline_boundary_condition("mirror"), 
						*produce_spline_boundary_condition("mirror"))), 
		range(16, 16),
		stransf(range, kernel, *ipf, P2DSplineTransformPenalty()),
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

		stransf.set_coefficients(field);
	}
	C2DBounds size;
	C2DFVectorfield field;
	PSplineKernel kernel; 
	P2DInterpolatorFactory ipf;

	C2DBounds range;
	C2DSplineTransformation  stransf;

protected:
	virtual float fx(float x, float y) = 0;
	virtual float fy(float x, float y) = 0;
private:
	C2DFVector scale;
};



class Cost2DMock {
public: 
	Cost2DMock(const C2DBounds& size); 
	double value(const C2DTransformation& t) const;  
	double value_and_gradient(C2DFVectorfield& gradient) const;
	
	double src_value(const C2DFVector& x)const; 
	double ref_value(const C2DFVector& x)const; 
	C2DFVector src_grad(const C2DFVector& x)const; 
	C2DBounds m_size; 
	C2DFVector m_center; 
	float m_r; 
}; 

class TransformGradientFixture {
public: 
	TransformGradientFixture(); 

	void run_test(C2DTransformation& t, double tol=0.1)const; 

	C2DBounds size; 
	Cost2DMock cost; 
	PSplineKernel kernel; 
	C2DFVector x; 
	C2DFVectorfield gradient; 
}; 

BOOST_AUTO_TEST_CASE (test_spline_set_parameter) 
{
	C2DBounds size(20,30); 
	C2DInterpolatorFactory ipf("bspline:d=3", "mirror"); 
	PSplineKernel kernel(CSplineKernelPluginHandler::instance().produce("bspline:d=3")); 
	C2DSplineTransformation t(size, kernel, C2DFVector(5.0,5.0), ipf, P2DSplineTransformPenalty());
	auto params = t.get_parameters();
	
	params[0] = 1.0; 
	t.set_parameters(params);
	
	auto i = t.begin();
	for ( size_t y = 0; y < size.y; ++y) 
		for ( size_t x = 0; x < size.x; ++x, ++i) {
			cvdebug()<< x << ", " << y << *i << "\n"; 
		}
	
	
	
}


BOOST_FIXTURE_TEST_CASE (test_spline_Gradient, TransformGradientFixture) 
{
	C2DInterpolatorFactory ipf("bspline:d=3", "mirror"); 
	C2DSplineTransformation t(size, kernel, C2DFVector(5.0,5.0), ipf, P2DSplineTransformPenalty());
	

	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.1; 

	auto itrg =  trgrad.begin(); 
	auto iparam = params.begin(); 
	for(size_t y = 0; y < t.get_coeff_size().y; ++y) {
		for(size_t x = 0; x < t.get_coeff_size().x; ++x) {
			for (size_t i = 0; i < 2; ++i, ++itrg, ++iparam) {
				*iparam += delta; 
				t.set_parameters(params);
				double cost_plus = cost.value(t);
				*iparam -= 2*delta; 
				t.set_parameters(params);
				double cost_minus = cost.value(t);
				*iparam += delta; 
				double test_val = (cost_plus - cost_minus)/ (2*delta); 
				if (fabs(*itrg) > 1e-09 || fabs(test_val) > 1e-09) 
					BOOST_CHECK_CLOSE(*itrg, test_val, 5); 
				
			}
		}
	}
	
}



TransformGradientFixture::TransformGradientFixture():
	size(60,80), 
	cost(size),
	kernel(CSplineKernelPluginHandler::instance().produce("bspline:d=3")), 
	x(11,16), 
	gradient(size)

{
	cost.value_and_gradient(gradient);
	
}

void TransformGradientFixture::run_test(C2DTransformation& t, double tol)const
{
	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.0001; 

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

Cost2DMock::Cost2DMock(const C2DBounds& size):
	m_size(size), 
	m_center(0.5 * size.x, 0.5 * size.y),
	m_r(sqrt(m_center.x * m_center.x + m_center.y * m_center.y))
{
}
	
double Cost2DMock::value(const C2DTransformation& t) const
{
	assert(m_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t y = 0; y < m_size.y; ++y) 
		for (size_t x = 0; x < m_size.x; ++x, ++it) {
			double v = src_value(*it) - ref_value(C2DFVector(x,y)); 
			result += v * v; 
		}
	return result; 
		
}

double Cost2DMock::value_and_gradient(C2DFVectorfield& gradient) const
{
	assert(gradient.get_size() == m_size); 
	
	double result = 0.0; 

	auto ig = gradient.begin(); 
	for (size_t y = 0; y < m_size.y; ++y) 
		for (size_t x = 0; x < m_size.x; ++x, ++ig) {
			C2DFVector pos(x,y);
			double v = src_value(pos) - ref_value(pos); 
			result += v * v; 
			*ig = 2.0 * v * src_grad(pos);  
		}
	return result; 
}

double Cost2DMock::src_value(const C2DFVector& x)const
{
	const C2DFVector p = x - m_center; 
	return exp( - (p.x * p.x + p.y * p.y) / m_r); 
}

C2DFVector Cost2DMock::src_grad(const C2DFVector& x)const
{
	
	return - 2.0f / m_r * (x-m_center) * src_value(x); 
}

double Cost2DMock::ref_value(const C2DFVector& x)const 
{
	const C2DFVector p = x - m_center - C2DFVector(2.0,2.0); 
	return exp( - (p.x * p.x + p.y * p.y) / m_r); 
}

BOOST_AUTO_TEST_CASE(check_small_range_throw) 
{
	BOOST_CHECK_THROW(C2DSplineTransformation(C2DBounds(10,10), 
						  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
						  C2DFVector(10,2),  C2DInterpolatorFactory("bspline:d=3", "mirror"), 
						  P2DSplineTransformPenalty()), invalid_argument); 

	BOOST_CHECK_THROW(C2DSplineTransformation(C2DBounds(10,10), 
						  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
						  C2DFVector(2,10),  C2DInterpolatorFactory("bspline:d=3", "mirror"), 
						  P2DSplineTransformPenalty()), invalid_argument); 

}


BOOST_AUTO_TEST_CASE(check_small_field_throw) 
{
	C2DSplineTransformation t(C2DBounds(10,10), 
				  CSplineKernelPluginHandler::instance().produce("bspline:d=3"), 
				  C2DInterpolatorFactory("bspline:d=3", "mirror"), 
				  P2DSplineTransformPenalty()); 
	
		
	BOOST_CHECK_THROW(t.set_coefficients(C2DFVectorfield(C2DBounds(1,10))), invalid_argument); 
	BOOST_CHECK_THROW(t.set_coefficients(C2DFVectorfield(C2DBounds(10,1))), invalid_argument); 

}



