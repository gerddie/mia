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
#include <fstream>
#include <mia/internal/autotest.hh>

#include <mia/core/spacial_kernel.hh>
#include <mia/3d/interpolator.hh>
#include <mia/3d/transform/spline.hh>


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
		size(33,65,20),
		kernel(new CBSplineKernel3()),
		range(50, 70, 25),
		r(range.x - 1, range.y - 1, range.z - 1),
		stransf(range, kernel),
		scale(2 * M_PI / r.x, 2 * M_PI / r.y, 2 * M_PI / r.z )
	{
		coeff_shift = kernel->get_active_halfrange() - 1; 
		C3DFVector ivscale(float(range.x - 1) / (size.x - 1),
				   float(range.y - 1) / (size.y - 1),
				   float(range.z - 1) / (size.z - 1));
		
		size.x += 2*coeff_shift; 
		size.y += 2*coeff_shift; 
		size.z += 2*coeff_shift; 
		C3DDVectorfield field(size); 

		C3DDVectorfield::iterator i = field.begin();
		for (size_t z = 0; z < size.z; ++z)
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x, ++i) {
					float sx = ivscale.x * (float(x) - coeff_shift);
					float sy = ivscale.y * (float(y) - coeff_shift);
					float sz = ivscale.z * (float(z) - coeff_shift);
					*i = C3DFVector( fx(sx, sy, sz), 
							 fy(sx, sy, sz), 
							 fz(sx, sy, sz));
				}
		
		// now filter 
		C3DDVectorfield help1(size);
		vector<C3DDVector> buffer(size.x); 
		for(size_t z = 0; z < size.z; ++z)
			for(size_t y = 0; y < size.y; ++y) {
				field.get_data_line_x(y, z, buffer); 
				kernel->filter_line(buffer); 
				help1.put_data_line_x(y, z, buffer); 
			}

		buffer.resize(size.y); 
		for(size_t z = 0; z < size.z; ++z)
			for(size_t x = 0; x < size.x; ++x) {
				help1.get_data_line_y(x, z, buffer); 
				kernel->filter_line(buffer); 
				field.put_data_line_y(x, z, buffer); 
			}
		buffer.resize(size.z); 
		for(size_t y = 0; y < size.y; ++y)
			for(size_t x = 0; x < size.x; ++x) {
				field.get_data_line_z(x, y, buffer); 
				kernel->filter_line(buffer); 
				help1.put_data_line_z(x, y, buffer); 
			}
		
		
		stransf.set_coefficients(help1); 
		stransf.reinit();
	}
	C3DBounds size;

	PBSplineKernel kernel;

	C3DBounds range;
	C3DBounds r;
	C3DSplineTransformation  stransf;
	int coeff_shift; 
protected:
	float fx(float x, float y, float z)const ;
	float fy(float x, float y, float z)const ;
	float fz(float x, float y, float z)const ;

	float fx(const C3DFVector& x)const;
	float fy(const C3DFVector& x)const;
	float fz(const C3DFVector& x)const;


	float dfx_x(float x, float y, float z);
	float dfy_x(float x, float y, float z);
	float dfz_x(float x, float y, float z);
	float dfx_y(float x, float y, float z);
	float dfy_y(float x, float y, float z);
	float dfz_y(float x, float y, float z);
	float dfx_z(float x, float y, float z);
	float dfy_z(float x, float y, float z);
	float dfz_z(float x, float y, float z);
private:
	C3DFVector scale;
};


float TransformSplineFixture::fx(const C3DFVector& v)const
{
	return fx(v.x, v.y, v.z); 
}

float TransformSplineFixture::fy(const C3DFVector& v)const
{
	return fy(v.x, v.y, v.z); 
}

float TransformSplineFixture::fz(const C3DFVector& v)const
{
	return fy(v.x, v.y, v.z); 
}



float TransformSplineFixture::fx(float x, float y, float z)const
{

	return 	20 * (1.0 - cosf(    scale.x * x)) * 
		(1.0 - cosf(2 * scale.y * y)) * 
		(1.0 - cosf(3 * scale.z * z)); 
}

float TransformSplineFixture::fy(float x, float y, float z)const
{
	return 	10 * (1.0 - cosf(2 * scale.x * x)) * 
		(1.0 - cosf(    scale.y * y)) * 
		(1.0 - cosf(    scale.z * z)); 
}

float TransformSplineFixture::fz(float x, float y, float z)const
{
	return 	30 * (1.0 - cosf(2 * scale.x * x)) * 
		(1.0 - cosf(3 * scale.y * y)) * 
		(1.0 - cosf(    scale.z * z)); 
}

float TransformSplineFixture::dfx_x(float x, float y, float z)
{
	x *= scale.x;
	return 20 * scale.x * sinf(x) * ( 1.0 - cosf(2 * scale.y * y))  * (1.0 - cosf(3 * scale.z * z)); 
}

float TransformSplineFixture::dfx_y(float x, float y, float z)
{

	return 	20 * (1.0 - cosf(scale.x * x)) * 
		sinf(2 * scale.y * y) * 2* scale.y *  
		(1.0 - cosf(3 * scale.z * z)); 
}

float TransformSplineFixture::dfx_z(float x, float y, float z)
{

	return 	20 * (1.0 - cosf(scale.x * x)) * 
		(1.0 - cosf(2 * scale.y * y)) * 
		sinf(3 * scale.z * z) *3 * scale.z; 
}

float TransformSplineFixture::dfy_x(float x, float y, float z)
{
	return 	20 * 2* scale.x * sinf(2 * scale.x * x) * 
		(1.0 - cosf(    scale.y * y)) * 
		(1.0 - cosf(    scale.z * z)); 
}

float TransformSplineFixture::dfy_y(float x, float y, float z)
{
	return 	20 * scale.y * (1.0 - cosf(2 * scale.x * x)) 
		* sinf( scale.y * y) 
		* (1.0 - cosf( scale.z * z)); 
}

float TransformSplineFixture::dfy_z(float x, float y, float z)
{
	return 	20 * (1.0 - cosf(2 * scale.x * x)) * 
		(1.0 - cosf(    scale.y * y)) * 
		scale.z * sinf( scale.z * z) ; 
}

float TransformSplineFixture::dfz_x(float x, float y, float z)
{
	return 	20 * sinf(2 * scale.x * x) * 2 * scale.x * 
		(1.0 - cosf(3 * scale.y * y)) * 
		(1.0 - cosf(    scale.z * z)); 
}

float TransformSplineFixture::dfz_y(float x, float y, float z)
{
	return 	20 * (1.0 - cosf(2 * scale.x * x)) * 
		 3 * scale.y * sinf(3 * scale.y * y) * 
		(1.0 - cosf(    scale.z * z)); 
}

float TransformSplineFixture::dfz_z(float x, float y, float z)
{
	return 	20 * (1.0 - cosf(2 * scale.x * x)) * 
		(1.0 - cosf(3 * scale.y * y)) * 
		sinf( scale.z * z) * scale.z; 
}


BOOST_FIXTURE_TEST_CASE( test_splines_transformation, TransformSplineFixture )
{
	BOOST_CHECK_EQUAL(stransf.degrees_of_freedom(), size.x*size.y*size.z * 3);

	C3DFVector testx(34.4, 74.8, 21.6); 

	BOOST_CHECK_EQUAL(stransf.get_size(), range);

	C3DFVector scaledx = stransf.scale(testx);
	BOOST_CHECK_CLOSE(scaledx.x, testx.x * (size.x-2*coeff_shift - 1) / (range.x-1)+coeff_shift, 0.1);
	BOOST_CHECK_CLOSE(scaledx.y, testx.y * (size.y-2*coeff_shift - 1) / (range.y-1)+coeff_shift, 0.1);
	BOOST_CHECK_CLOSE(scaledx.z, testx.z * (size.z-2*coeff_shift - 1) / (range.z-1)+coeff_shift, 0.1);


	C3DFVector result_apply = stransf.apply(testx);
	BOOST_CHECK_CLOSE(result_apply.x, fx(testx), 0.1);
	BOOST_CHECK_CLOSE(result_apply.y, fy(testx), 0.1);
	BOOST_CHECK_CLOSE(result_apply.z, fz(testx), 0.1);

	C3DFVector result_operator = stransf(testx);
	BOOST_CHECK_CLOSE(result_operator.x, testx.x - fx(testx), 0.1);
	BOOST_CHECK_CLOSE(result_operator.y, testx.y - fy(testx), 0.1);
	BOOST_CHECK_CLOSE(result_operator.z, testx.z - fz(testx), 0.1);

}


#ifdef REVIEW_THIS_TEST
BOOST_FIXTURE_TEST_CASE( test_splines_transformation_upscale, TransformSplineFixture )
{
	const C3DBounds scale(2.0, 3.0);
	C3DBounds new_range = scale * range;

	const C3DFVector fscale(float(new_range.x - 1.0) / (range.x - 1.0),
				float(new_range.y - 1.0) / (range.y - 1.0));

	cvdebug() << fscale << "\n";
	P3DTransformation  stransf_upscaled = stransf.upscale(new_range);

	stransf_upscaled->reinit();

	C3DFVector test2(34.4, 90.3);

	C3DFVector result2 = stransf_upscaled->apply(fscale * test2);

	BOOST_CHECK_CLOSE(result2.x, scale.x * fx(test2), 0.1);
	BOOST_CHECK_CLOSE(result2.y, scale.y * fy(test2), 0.1);

}
#endif


BOOST_FIXTURE_TEST_CASE( test_splines_add, TransformSplineFixture )
{
	stransf.reinit();
	C3DFVector testx(30.4, 42.8, 12.3); 

	C3DFVector r1( testx.x - fx(testx), 
		       testx.y - fy(testx), 
		       testx.z - fz(testx));
	C3DFVector r2( r1.x - fx(r1), r1.y -fy(r1), r1.z - fz(r1)); 

	stransf.add(stransf);
	stransf.reinit();
	C3DFVector result = stransf(testx);
	BOOST_CHECK_CLOSE(result.x, r2.x, 0.1);
	BOOST_CHECK_CLOSE(result.y, r2.y, 0.1);
	BOOST_CHECK_CLOSE(result.z, r2.z, 0.1);

}

BOOST_FIXTURE_TEST_CASE( test_splinestransform_prefix_iterator, TransformSplineFixture )
{
	
	auto i = stransf.begin();

	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++i) {
				cvdebug() << "x=" << x << " y=" << y << ", " << range << "\n"; 
				BOOST_CHECK_CLOSE(1.0 + x - fx(x,y,z), 1.0 + (*i).x, 0.8);
				BOOST_CHECK_CLOSE(1.0 + y - fy(x,y,z), 1.0 + (*i).y, 0.8);
				BOOST_CHECK_CLOSE(1.0 + z - fz(x,y,z), 1.0 + (*i).z, 0.8);
			}
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
			for (size_t x = 0; x < range.x; ++x) {
				C3DFVector test = *i++;
				cvdebug() << "splinestransform_postfix_iterator" << test << "\n"; 
				BOOST_CHECK_CLOSE(1.0 + x - fx(x,y,z), 1.0 + test.x, 0.8);
				BOOST_CHECK_CLOSE(1.0 + y - fy(x,y,z), 1.0 + test.y, 0.8);
				BOOST_CHECK_CLOSE(1.0 + z - fz(x,y,z), 1.0 + test.z, 0.8);
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
	
	P3DInterpolatorFactory ipf(new C3DInterpolatorFactory(C3DInterpolatorFactory::ip_spline,
							      std::shared_ptr<CBSplineKernel > (new CBSplineKernel3()))); 
	auto_ptr<T3DInterpolator<float> > src(ipf->create(image.data()));

	C3DFImage::iterator t = test_image.begin();

	for (size_t z = 0; z < range.z; ++z)
		for (size_t y = 0; y < range.y; ++y)
			for (size_t x = 0; x < range.x; ++x, ++t) {
				const C3DFVector v(x,y,z);
				*t = (*src)(stransf(v));
			}
	
	P3DImage stransfed = stransf(image, *ipf);
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
	double scale_x = double(range.x - 1) / (size.x - 1 - 2*coeff_shift); 
	double scale_y = double(range.y - 1) / (size.y - 1 - 2*coeff_shift); 
	double scale_z = double(range.z - 1) / (size.z - 1 - 2*coeff_shift); 
	double scale = scale_x * scale_y * scale_z; 
	
	cvdebug() << range << size << ", scale=" << scale << "\n"; 

	fill(gradient.begin(), gradient.end(), C3DFVector(1.0, 2.0, 3.0));

	gsl::DoubleVector force(stransf.degrees_of_freedom());
	stransf.translate(gradient, force);

	auto  i = force.begin();
	for (size_t z = 0; z < stransf.get_coeff_size().z; ++z)
		for (size_t y = 0; y < stransf.get_coeff_size().y; ++y)
			for (size_t x = 0; x < stransf.get_coeff_size().x; ++x, i+=3) {
				if (y > 3 && y < stransf.get_coeff_size().y - 3 && 
				    x > 3 && x < stransf.get_coeff_size().x - 3 && 
				    z > 3 && z < stransf.get_coeff_size().z - 3) {
					BOOST_CHECK_CLOSE( i[0], -1.0f * scale, 0.1);
					BOOST_CHECK_CLOSE( i[1], -2.0f * scale, 0.1);
					BOOST_CHECK_CLOSE( i[2], -3.0f * scale, 0.1);
				}
			}
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

	C3DFVector testx(33.4, 82.4, 21.9);
	C3DFVector result = stransf.apply(testx);

	BOOST_CHECK_CLOSE(result.x, fx(testx) + 2.0f, 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(testx) + 4.0f, 0.1);
	BOOST_CHECK_CLOSE(result.z, fz(testx) + 6.0f, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_splines_gridpoint_derivative, TransformSplineFixture )
{
	C3DFVector x(33,80,60);
	C3DFMatrix dv =  stransf.derivative_at(33,80,60);

	BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.y.x,      - dfx_y(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.z.x,      - dfx_z(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.x.y,      - dfy_x(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.z.y,      - dfy_z(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.x.z,      - dfz_x(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.y.z,      - dfz_y(x.x, x.y, x.z), 0.2);
	BOOST_CHECK_CLOSE(dv.z.z, 1.0f - dfz_z(x.x, x.y, x.z), 0.2);

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

BOOST_AUTO_TEST_CASE( test_spline_c_rate_create )
{
	PBSplineKernel kernel(new CBSplineKernel3()); 
	C3DBounds size(20, 32, 25);
	C3DFVector c_rate(2.5, 3.2, 5.0);
	C3DSplineTransformation  stransf(size, kernel, c_rate);

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
	C3DFVector upscale(2, 3, 4);
	C3DBounds new_range(range.x * upscale.x, 
			    range.y * upscale.y, 
			    range.z * upscale.z); 

	P3DTransformation upscaled(stransf.upscale(new_range)); 
	auto i = upscaled->begin();
	for (size_t z = 0; z < new_range.z; ++z)
		for (size_t y = 0; y < new_range.y; ++y)
			for (size_t x = 0; x < new_range.x; ++x, ++i) {
				BOOST_CHECK_CLOSE(1.0 + x - fx(x/upscale.x,y/upscale.y, z/upscale.z)*upscale.x, 
						  1.0 + i->x, 0.5);
				BOOST_CHECK_CLOSE(1.0 + y - fy(x/upscale.x,y/upscale.y, z/upscale.z)*upscale.y, 
						  1.0 + i->y, 0.5);
				BOOST_CHECK_CLOSE(1.0 + z - fz(x/upscale.x,y/upscale.y, z/upscale.z)*upscale.z, 
						  1.0 + i->z, 0.5);
			}
}

BOOST_FIXTURE_TEST_CASE( test_splines_refine, TransformSplineFixture )
{
	BOOST_CHECK(!stransf.refine()); 

	C3DFVector upscale(2, 3, 2);
	C3DBounds new_range(range.x * upscale.x, 
			    range.y * upscale.y,
			    range.z * upscale.z);
	

	P3DTransformation upscaled(stransf.upscale(new_range)); 

	BOOST_CHECK(upscaled->refine()); 
	C3DSplineTransformation::const_iterator i = upscaled->begin();
	
	for (size_t z = 0; z < new_range.z; ++z)
		for (size_t y = 0; y < new_range.y; ++y)
			for (size_t x = 0; x < new_range.x; ++x, ++i) {
				BOOST_CHECK_CLOSE(1.0 + x - fx(x/upscale.x,y/upscale.y,z/upscale.z)*upscale.x, 
						  1.0 + i->x, 0.5);
				BOOST_CHECK_CLOSE(1.0 + y - fy(x/upscale.x,y/upscale.y,z/upscale.z)*upscale.y, 
						  1.0 + i->y, 0.5);
				BOOST_CHECK_CLOSE(1.0 + y - fy(x/upscale.x,y/upscale.y,z/upscale.z)*upscale.z, 
						  1.0 + i->z, 0.5);
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
	const C3DBounds size(10,10, 10);
	
	PBSplineKernel kernel(new CBSplineKernel3()); 

	P3DInterpolatorFactory ipf(new C3DInterpolatorFactory(C3DInterpolatorFactory::ip_spline,
							      kernel));

	C3DSplineTransformation trans(size, kernel);

	C3DFImage *psrc = new C3DFImage(size); 
	P3DImage src(psrc);
	
	copy(src_image_init, src_image_init + 100, psrc->begin_at(0,0,4)); 
	

	C3DDVectorfield field(C3DBounds(5,4,4)); 

	C3DFVector shift(1,2,1); 
	fill(field.begin(), field.end(), shift); 
	trans.set_coefficients(field);
	
	P3DImage result = trans.apply(*src, *ipf);
	
	const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result); 
	
	BOOST_REQUIRE(r.get_size() == size); 
	float *itest = ref_image_init; 
	auto ir = r.begin_at(0,0,5); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ir, ++itest) {
			cvinfo() << x << "," << y << ", " << *ir << " ?= " <<*itest << "\n"; 
			if (*itest > 0.001 || *ir > 0.001) 
				BOOST_CHECK_EQUAL(*ir, *itest); 
		}
}



struct TransformSplineFixtureFieldBase {
	TransformSplineFixtureFieldBase():
		size(16,16,16),
		field(size),
		kernel(new CBSplineKernel3()), 
		ipf(new C3DInterpolatorFactory(C3DInterpolatorFactory::ip_spline, kernel)),
		range(16, 16,16),
		stransf(range, kernel),
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
	C3DDVectorfield field;
	PBSplineKernel kernel; 
	P3DInterpolatorFactory ipf;

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
	C3DBounds _M_size; 
	C3DFVector _M_center; 
	float _M_r; 
}; 

class TransformGradientFixture {
public: 
	TransformGradientFixture(); 

	void run_test(C3DTransformation& t, double tol=0.1)const; 

	C3DBounds size; 
	Cost3DMock cost; 
	PBSplineKernel kernel; 
	C3DFVector x; 
	C3DFVectorfield gradient; 
}; 

BOOST_AUTO_TEST_CASE (test_spline_set_parameter) 
{
	C3DBounds size(20,30,25); 
	P3DInterpolatorFactory ipf(create_3dinterpolation_factory(ip_bspline3)); 
	PBSplineKernel kernel(new CBSplineKernel3()); 
	C3DSplineTransformation t(size, kernel, C3DFVector(5.0,5.0,5.0));
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


BOOST_FIXTURE_TEST_CASE (test_spline_Gradient, TransformGradientFixture) 
{
	P3DInterpolatorFactory ipf(create_3dinterpolation_factory(ip_bspline3)); 
	C3DSplineTransformation t(size, kernel, C3DFVector(5.0, 5.0, 5.0));
	

	auto params = t.get_parameters();
	gsl::DoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.1; 

	auto itrg =  trgrad.begin(); 
	
	for ( auto iparam = params.begin(); iparam != params.end(); ++itrg, ++iparam) {
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



TransformGradientFixture::TransformGradientFixture():
	size(60,80,70), 
	cost(size),
	kernel(new CBSplineKernel3()), 
	x(11,16,22), 
	gradient(size)

{
	cost.value_and_gradient(gradient);
	
}

void TransformGradientFixture::run_test(C3DTransformation& t, double tol)const
{
	auto params = t.get_parameters();
	gsl::DoubleVector trgrad(params.size()); 
	
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

Cost3DMock::Cost3DMock(const C3DBounds& size):
	_M_size(size), 
	_M_center(0.5 * size.x, 0.5 * size.y, 0.5 * size.z),
	_M_r(sqrt(_M_center.x * _M_center.x + 
		  _M_center.y * _M_center.y + 
		  _M_center.z * _M_center.z))
{
}
	
double Cost3DMock::value(const C3DTransformation& t) const
{
	assert(_M_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t z = 0; z < _M_size.z; ++z) 
		for (size_t y = 0; y < _M_size.y; ++y) 
			for (size_t x = 0; x < _M_size.x; ++x, ++it) {
				double v = src_value(*it) - ref_value(C3DFVector(x,y,z)); 
				result += v * v; 
			}
	return result; 
		
}

double Cost3DMock::value_and_gradient(C3DFVectorfield& gradient) const
{
	assert(gradient.get_size() == _M_size); 
	
	double result = 0.0; 

	auto ig = gradient.begin(); 
	for (size_t z = 0; z < _M_size.z; ++z) 
		for (size_t y = 0; y < _M_size.y; ++y) 
			for (size_t x = 0; x < _M_size.x; ++x, ++ig) {
				C3DFVector pos(x,y,z);
				double v = src_value(pos) - ref_value(pos); 
				result += v * v; 
				*ig = 2.0 * v * src_grad(pos);  
			}
	return result; 
}

double Cost3DMock::src_value(const C3DFVector& x)const
{
	const C3DFVector p = x - _M_center; 
	return exp( - (p.x * p.x + p.y * p.y + p.z * p.z) / _M_r); 
}

C3DFVector Cost3DMock::src_grad(const C3DFVector& x)const
{
	
	return - 2.0f / _M_r * (x-_M_center) * src_value(x); 
}

double Cost3DMock::ref_value(const C3DFVector& x)const 
{
	const C3DFVector p = x - _M_center - C3DFVector(2.0,2.0, 2.0); 
	return exp( - (p.x * p.x + p.y * p.y  + p.z * p.z) / _M_r); 
}




