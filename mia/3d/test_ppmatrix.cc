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

#include <mia/3d/ppmatrix.hh>

NS_MIA_USE;

struct TransformSplineFixtureFieldBase {
	TransformSplineFixtureFieldBase()
	{

	}
	void init(int dsize, double r, EInterpolation type) {
		ipf.reset(create_3dinterpolation_factory(type));
		size = C3DBounds(2 * dsize + 1,2 * dsize + 1,2 * dsize + 1);
		field = C3DFVectorfield(size);
		range = r; 
		scale.x = scale.y = scale.z = range / dsize;
		h = dsize / range;

		field_range.x = 2 * range; 
		field_range.y = 2 * range; 
		field_range.z = 2 * range; 
		
		graddiv2sum = 0.0; 
		C3DFVectorfield::iterator i = field.begin();
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
		assert(i == field.end()); 
		source.reset(ipf->create(field));

		cvinfo() << "scale = " << scale << "\n"; 		graddiv2sum *= scale.x * scale.y*scale.z; 
	}
	C3DBounds size;
	C3DFVectorfield field;
	P3DInterpolatorFactory ipf;
	C3DFVector field_range;
	double h;
	double range; 
	std::shared_ptr<T3DInterpolator<C3DFVector>  > source; 
protected:
	virtual double fx(double x, double y, double z)const  = 0;
	virtual double fy(double x, double y, double z)const  = 0;
	virtual double fz(double x, double y, double z)const  = 0;
	virtual double graddiv2(double x, double y, double z)const = 0;
	C3DFVector scale;
	double graddiv2sum; 
private:

};

struct TransformSplineFixtureDivOnly: public TransformSplineFixtureFieldBase {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
	double graddiv2(double x, double y, double z)const;
	double integrate() const ; 
};

struct TransformSplineFixtureCurlOnly: public TransformSplineFixtureFieldBase {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
};


struct TransformSplineFixtureexpm2Field: public TransformSplineFixtureFieldBase {
	TransformSplineFixtureexpm2Field(){}
	virtual double fx(double x, double y, double z)const ;
	virtual double fy(double x, double y, double z)const ;
	virtual double fz(double x, double y, double z)const ;

	double integrate_div(double x1, double x2, double y1, double y2, int xinterv, int yinterv);
	double div_value_at(double x, double y, double z);
	C3DFVector div_derivative_at(double x, double y, double z);
};

struct TransformSplineFixtureexpm2Field_44: public TransformSplineFixtureexpm2Field {
	void run(int dsize, double range, EInterpolation type, double corr=1.0); 
}; 

struct TransformSplineFixtureexpm2testInterp : public TransformSplineFixtureexpm2Field {
	void run(int dsize, double range, EInterpolation type); 
}; 


BOOST_FIXTURE_TEST_CASE( test_nocurl_bspline3, TransformSplineFixtureDivOnly )
{
	init(8, 4, ip_bspline3);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double testvalue = 105.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 8.0); 

	// sanity check 
	BOOST_CHECK_CLOSE(integrate(), testvalue, 0.1); 
	BOOST_CHECK_CLOSE(graddiv2sum, testvalue, 0.1); 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.1); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.1); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 0.1); 	
}


double TransformSplineFixtureDivOnly::fx(double x, double y, double z)const 
{
	return x * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureDivOnly::fy(double x, double y, double z)const 
{
	return y * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureDivOnly::fz(double x, double y, double z)const 
{
	return z * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureDivOnly::graddiv2(double x, double y, double z) const 
{
	double h1 = x*x+y*y+z*z; 
	double h2 = 2.0 * h1; 
	double h3 = h2 - 5.0; 
	
	return 4 * h1 * h3 * h3 * exp(-h2); 
}

double TransformSplineFixtureDivOnly::integrate() const
{
	const double step = range/100.0; 
	
	double sum = 0.0; 
	for(double z = -range; z < range; z += step) 
		for(double y = -range; y < range; y += step) 
			for(double x = -range; x < range; x += step) {
				sum += graddiv2(x, y, z); 
			}
	return sum * step * step * step; 
}


#if 0 

BOOST_FIXTURE_TEST_CASE( test_nodiv_bspline4, TransformSplineFixtureCurlOnly )
{
	init(16, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double testvalue = 6.0 * M_PI;

	C3DPPDivcurlMatrix curl(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	BOOST_CHECK_CLOSE( curl  * coeffs, testvalue, 0.1); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	BOOST_CHECK_CLOSE( divcurl * coeffs, testvalue, 0.1); 	
	
	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	BOOST_CHECK_CLOSE( 1.0 + div * coeffs, 1.0, 0.1); 	
}



// test whether the interpolation is "good enough" 
BOOST_FIXTURE_TEST_CASE( test_interpolation_16_2_bspline3, TransformSplineFixtureexpm2testInterp ) 
{
	run( 8, 2.0, ip_bspline3);
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_16_8, TransformSplineFixtureexpm2Field_44 )
{
	run(16, 4, ip_bspline3, 1.0);

}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_8_8, TransformSplineFixtureexpm2Field_44 )
{
	run(8, 4, ip_bspline3, 1.0);

}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_grad, TransformSplineFixtureexpm2Field_44 )
{
	init(8, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);


	gsl::DoubleVector gradient(field.size() * 2, true); 
	div.evaluate(coeffs, gradient); 
	

	auto ig = gradient.begin(); 
	auto ic = coeffs.begin(); 
	for(size_t y = 0; y < field.get_size().y; ++y) 
		for(size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
			ic->x += 0.001; 
			double graddivp = div * coeffs; 
			ic->x -= 0.002; 
			double graddivm = div * coeffs; 
			ic->x += 0.001; 
			double test_grad = (graddivp - graddivm)/ 0.002; 
			cvdebug() << x << " " << y << " (x)\n"; 
			if (abs(test_grad) > 0.0001)
				BOOST_CHECK_CLOSE(ig[0], test_grad, 2);
			
			ic->y += 0.001; 
			graddivp = div * coeffs; 
			ic->y -= 0.002; 
			graddivm = div * coeffs; 
			ic->y += 0.001; 
			test_grad = (graddivp - graddivm)/ 0.002; 
			cvdebug() << x << " " << y << " (y)\n"; 
			
			if (abs(test_grad) > 0.0001)
				BOOST_CHECK_CLOSE(ig[1], test_grad, 2);
		}
}

BOOST_FIXTURE_TEST_CASE( test_rotation_expm2_bspline3_grad, TransformSplineFixtureexpm2Field_44 )
{
	init(8, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);


	gsl::DoubleVector gradient(field.size() * 2, true); 
	rot.evaluate(coeffs, gradient); 
	

	auto ig = gradient.begin(); 
	auto ic = coeffs.begin(); 
	for(size_t y = 0; y < field.get_size().y; ++y) 
		for(size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
			ic->x += 0.001; 
			double graddivp = rot * coeffs; 
			ic->x -= 0.002; 
			double graddivm = rot * coeffs; 
			ic->x += 0.001; 
			double test_grad = (graddivp - graddivm)/ 0.002; 
			cvdebug() << x << " " << y << " (x)\n"; 
			if (abs(test_grad) > 0.0001)
				BOOST_CHECK_CLOSE(ig[0], test_grad, 2);
			
			ic->y += 0.001; 
			graddivp = rot * coeffs; 
			ic->y -= 0.002; 
			graddivm = rot * coeffs; 
			ic->y += 0.001; 
			test_grad = (graddivp - graddivm)/ 0.002; 
			cvdebug() << x << " " << y << " (y)\n"; 
			
			if (abs(test_grad) > 0.0001)
				BOOST_CHECK_CLOSE(ig[1], test_grad, 2);
		}
}


BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline4, TransformSplineFixtureexpm2Field_44 )
{
	run(16, 4, ip_bspline4, 1.0);

}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline4_8_4, TransformSplineFixtureexpm2Field_44 )
{
	run(8, 4, ip_bspline4, 1.0);

}

BOOST_FIXTURE_TEST_CASE( test_divergence_zero_x, TransformSplineFixtureConst )
{
	prepare(16, 16, ip_bspline4, 0, .01);

	const double testvalue = 0.0;

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	double spline = div * coeffs;
	BOOST_CHECK_CLOSE(spline + 1.0, testvalue + 1.0,  1);

	BOOST_CHECK_EQUAL(field.get_size(), div.get_size()); 
}

#endif


double TransformSplineFixtureCurlOnly::fx(double x, double y, double z)const 
{
	return y * exp(-x*x-y*y-z*z);
}


double TransformSplineFixtureCurlOnly::fy(double x, double y, double z)const 
{
	return -x * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureCurlOnly::fz(double /*x*/, double /*y*/, double /*z*/)const 
{
	return 0.0;
}

double TransformSplineFixtureexpm2Field::fx(double x, double y, double z)const 
{
	return exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureexpm2Field::fy(double x, double y, double z)const 
{
	return exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureexpm2Field::fz(double x, double y, double z)const 
{
	return exp(-x*x-y*y-z*z);
}

#if 0 


double TransformSplineFixtureexpm2Field::div_value_at(double x, double y, double z)
{
	const double vfx = dfx_xx(x,y,z) + dfy_xy(x,y,z);
	const double vfy = dfx_xy(x,y,z) + dfy_yy(x,y,z);
	return   vfx * vfx + vfy * vfy;

}

C3DFVector TransformSplineFixtureexpm2Field::div_derivative_at(double x, double y, double z)
{
	double fy_yyy = dfy_yyy(x,y,z); 
	double fy_xyy = dfy_xyy(x,y,z); 
	double fx_xyy = dfx_xyy(x,y,z); 
	double fx_xxy = dfx_xxy(x,y,z); 
	double fy_xxy = dfy_xxy(x,y,z);
	double fx_xxx = dfx_xxx(x,y,z); 
	double fy_yy =  dfy_yy(x,y,z); 
	double fx_xx =  dfx_xx(x,y,z); 
	double fx_xy =  dfx_xy(x,y,z); 
	double fy_xy =  dfy_xy(x,y,z); 
	
	return C3DFVector(
		2* (( fy_xyy + fx_xxy ) * ( fy_yy + fx_xy) + 
		    ( fy_xy + fx_xx) * (fy_xxy + fx_xxx)), 
		2* (( fy_yy + fx_xy) * (fy_yyy + fx_xyy) + 
		    ( fy_xy + fx_xx) * (fy_xyy + fx_xxy))
		); 
}


double TransformSplineFixtureexpm2Field::integrate_div(double x1, double x2,
							  double y1, double y2, int xinterv, int yinterv)
{
	double hx = (x2 - x1) / xinterv;
	double hy = (y2 - y1) / yinterv;

	double sum = 0.0;

	for (double y = y1 + hx / 2.0; y < y2; y += hy)
		for (double x = x1 + hx / 2.0; x < x2; x += hx) 
			sum += div_value_at(x,y,z);
	
	sum *= hx * hy;
	return sum;
}



void TransformSplineFixtureexpm2testInterp::run(int size, double range, EInterpolation type)
{
	init( size, range, type);
	
	for (double y = -range; y < range; y += range/5.0)
		for (double x = -range; x < range; x += range/5.0) {
			C3DFVector hx(x * h + size, y * h + size); 
			C3DFVector hy = (*source)(hx); 
			BOOST_CHECK_CLOSE(1.0 + hy.x, 1.0 + fx(x , y ), 0.3); 
			BOOST_CHECK_CLOSE(1.0 + hy.y, 1.0 + fy(x , y ), 0.3); 
		}
	
}

void TransformSplineFixtureexpm2Field_44::run(int dsize, double range, EInterpolation type, double corr)
{
	init(dsize, range, type);

	// evaluated using maxima
	const double testvalue = 4.0 * M_PI;

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 

	// this test is just to compare the maxima value to a approximate integration
	double manual = integrate_div(-range, range, -range, range, 63, 63);
	BOOST_CHECK_CLOSE(manual, testvalue, 1);

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	double spline = corr * (div * coeffs);
       	cvmsg() << "(" << dsize << "," << range <<  ") " << spline << ", ratio=" << manual / spline <<"\n"; 
	BOOST_CHECK_CLOSE(spline, manual,  1);


}
#endif
