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
		size(32,32),
		field(size),
		ipf(new C2DInterpolatorFactory(C2DInterpolatorFactory::ip_spline,
					       SHARED_PTR(CBSplineKernel) (new CBSplineKernel3())))
	{

	}
	void init() {
		C2DFVectorfield::iterator i = field.begin();
		for (int y = 0; y < (int)size.y; ++y)
			for (int x = 0; x < (int)size.x; ++x, ++i) {
				float sx = x - size.x / 2.0;
				float sy = y - size.y / 2.0;
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

BOOST_FIXTURE_TEST_CASE( test_divergence_zero_bspline3, TransformSplineFixtureConstDivergence )
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

BOOST_FIXTURE_TEST_CASE( test_divergence_grad_4_bspline3, TransformSplineFixtureDivergence2 )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(16.0, divcurl.multiply(field), 1.0);
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



BOOST_FIXTURE_TEST_CASE( test_divergence_OnlyXConst_bspline3, TransformSplineFixtureOnlyXConstDivField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(1.0, 1.0 + divcurl.multiply(field), 1.0);
}



struct TransformSplineFixtureOnlyYConstDivField: public TransformSplineFixtureFieldBase {
	virtual float fx(float /*x*/, float /*y*/) {return 0.0;};
	virtual float fy(float /*x*/, float y) {return y;};
}; 



BOOST_FIXTURE_TEST_CASE( test_divergence_OnlyYConst_bspline3, TransformSplineFixtureOnlyYConstDivField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(1.0, 1.0 + divcurl.multiply(field), 1.0);
}



struct TransformSplineFixtureGaussDivField: public TransformSplineFixtureFieldBase {
	virtual float fx(float x, float y);
	virtual float fy(float x, float y);
}; 



BOOST_FIXTURE_TEST_CASE( test_divergence_Gauss_bspline3, TransformSplineFixtureGaussDivField )
{
	init(); 
	C2DDivCurlMatrix divcurl(size, ipf->get_kernel());
	BOOST_CHECK_CLOSE(0.0, divcurl.multiply(field), 1.0);
}

float TransformSplineFixtureGaussDivField::fx(float x, float /*y*/)
{
	return exp( -x*x); 
}

float TransformSplineFixtureGaussDivField::fy(float /*x*/, float y)
{
	return exp( -y*y); 
}


