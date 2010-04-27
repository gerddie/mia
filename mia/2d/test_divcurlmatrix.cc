/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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
 */


#include <mia/internal/autotest.hh>

#include <mia/2d/divcurlmatrix.hh>

NS_MIA_USE; 


struct TransformSplineFixtureFieldBase {
	TransformSplineFixtureFieldBase():
		size(33,33),
		field(size),
		ipf(new C2DInterpolatorFactory(C2DInterpolatorFactory::ip_spline,
					       SHARED_PTR(CBSplineKernel) (new CBSplineKernel3())))
	{

	}
	void init() {
		C2DFVectorfield::iterator i = field.begin();
		for (int y = 0; y < (int)size.y; ++y)
			for (int x = 0; x < (int)size.x; ++x, ++i) {
				float sx = (x - size.x / 2.0)/16.0;
				float sy = (y - size.y / 2.0)/16.0;
				*i = C2DFVector( fx(sx, sy), fy(sx, sy)); 
			}

		SHARED_PTR(T2DInterpolator<C2DFVector> ) source(ipf->create(field));
	}
	C2DBounds size;
	C2DFVectorfield field;
	P2DInterpolatorFactory ipf;

	C2DBounds range;
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

BOOST_FIXTURE_TEST_CASE( test_divergence_X_x_Y_x_bspline3, TransformSplineFixtureConstDivergence )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(1.0, 1.0 + divcurl.multiply(field), 1.0);
}

float TransformSplineFixtureConstDivergence::fx(float x, float /*y*/)
{
	return x; 
}

float TransformSplineFixtureConstDivergence::fy(float  /*x*/, float y)
{
	return y; 
}

struct TransformSplineFixtureDivergence2: public TransformSplineFixtureFieldBase {
	virtual float fx(float x, float y);
	virtual float fy(float x, float y);
}; 

BOOST_FIXTURE_TEST_CASE( test_divergence_xsq_ysq_bspline3, TransformSplineFixtureDivergence2 )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(64, divcurl.multiply(field), 1.0);
}


float TransformSplineFixtureDivergence2::fx(float x, float /*y*/)
{
	return x * x; 
}

float TransformSplineFixtureDivergence2::fy(float  /*x*/, float y)
{
	return y * y; 
}

struct TransformSplineFixtureOnlyXConstDivField: public TransformSplineFixtureFieldBase {
	virtual float fx(float x, float /*y*/) {return x;};
	virtual float fy(float /*x*/, float /*y*/) {return 0.0;};
}; 



BOOST_FIXTURE_TEST_CASE( test_divergence_X_x_Y_0_bspline3, TransformSplineFixtureOnlyXConstDivField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(1.0, 1.0 + divcurl.multiply(field), 1.0);
}

struct TransformSplineFixtureOnlyYConstDivField: public TransformSplineFixtureFieldBase {
	virtual float fx(float /*x*/, float /*y*/) {return 0.0;};
	virtual float fy(float /*x*/, float y) {return y;};
}; 

BOOST_FIXTURE_TEST_CASE( test_divergence_X_0_Y_y_bspline3, TransformSplineFixtureOnlyYConstDivField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(1.0, 1.0 + divcurl.multiply(field), 1.0);
}

struct TransformSplineFixtureInvSqField: public TransformSplineFixtureFieldBase {
	TransformSplineFixtureInvSqField():
		r(sqrt(size.x * size.x + size.y * size.y)){}
	virtual float fx(float x, float y);
	virtual float fy(float x, float y);
private: 
	double r; 
}; 

BOOST_FIXTURE_TEST_CASE( test_divergence_Gauss_bspline3, TransformSplineFixtureInvSqField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE((15.0 * M_PI + 56.0)/ 10.0, divcurl.multiply(field), 1.0);
}

float TransformSplineFixtureInvSqField::fx(float x, float /*y*/)
{
	return 1.0 / (x*x + 1); 
}

float TransformSplineFixtureInvSqField::fy(float /*x*/, float y)
{
	return 1.0 / (y*y + 1); 
}

BOOST_FIXTURE_TEST_CASE( test_get_index_Lower_boundary_bspline3, TransformSplineFixtureInvSqField )
{

	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());	
	BOOST_CHECK_EQUAL(divcurl.get_index(0,0,10), 0); 
	BOOST_CHECK_EQUAL(divcurl.get_index(0,1,10), 1);
	BOOST_CHECK_EQUAL(divcurl.get_index(0,2,10), 2); 
	BOOST_CHECK_EQUAL(divcurl.get_index(0,3,10), 3);

	BOOST_CHECK_EQUAL(divcurl.get_index(1,1,10), 8);
	BOOST_CHECK_EQUAL(divcurl.get_index(1,2,10), 9);
	BOOST_CHECK_EQUAL(divcurl.get_index(1,3,10), 10);
	BOOST_CHECK_EQUAL(divcurl.get_index(1,4,10), 11);


	BOOST_CHECK_EQUAL(divcurl.get_index(2,2,10), 16);
	BOOST_CHECK_EQUAL(divcurl.get_index(2,3,10), 17);
	BOOST_CHECK_EQUAL(divcurl.get_index(2,4,10), 18);
	BOOST_CHECK_EQUAL(divcurl.get_index(2,5,10), 19);

	BOOST_CHECK_EQUAL(divcurl.get_index(3,3,10), 24);
	BOOST_CHECK_EQUAL(divcurl.get_index(3,4,10), 25);
	BOOST_CHECK_EQUAL(divcurl.get_index(3,5,10), 26);
	BOOST_CHECK_EQUAL(divcurl.get_index(3,6,10), 27);
	BOOST_CHECK_EQUAL(divcurl.get_index(3,7,10), -1);
	BOOST_CHECK_EQUAL(divcurl.get_index(3,8,10), -1);



	BOOST_CHECK_EQUAL(divcurl.get_index(19,19,20), 0); 
	BOOST_CHECK_EQUAL(divcurl.get_index(19,18,20), 1);
	BOOST_CHECK_EQUAL(divcurl.get_index(19,17,20), 2); 
	BOOST_CHECK_EQUAL(divcurl.get_index(19,16,20), 3);
	BOOST_CHECK_EQUAL(divcurl.get_index(19,15,20), -1);

	BOOST_CHECK_EQUAL(divcurl.get_index(18,18,20), 8);
	BOOST_CHECK_EQUAL(divcurl.get_index(18,17,20), 9);
	BOOST_CHECK_EQUAL(divcurl.get_index(18,16,20),10);
	BOOST_CHECK_EQUAL(divcurl.get_index(18,15,20), 11);


	BOOST_CHECK_EQUAL(divcurl.get_index(17,17,20), 16);
	BOOST_CHECK_EQUAL(divcurl.get_index(17,16,20), 17);
	BOOST_CHECK_EQUAL(divcurl.get_index(17,15,20), 18);
	BOOST_CHECK_EQUAL(divcurl.get_index(17,14,20), 19);

	BOOST_CHECK_EQUAL(divcurl.get_index(16,16,20), 24);
	BOOST_CHECK_EQUAL(divcurl.get_index(16,15,20), 25);
	BOOST_CHECK_EQUAL(divcurl.get_index(16,14,20), 26);
	BOOST_CHECK_EQUAL(divcurl.get_index(16,13,20), 27);
	BOOST_CHECK_EQUAL(divcurl.get_index(16,12,20), -1);


	BOOST_CHECK_EQUAL(divcurl.get_index(31,31,32), 0);
	
	
}
