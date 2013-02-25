/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/internal/autotest.hh>

#include <mia/3d/ppmatrix.hh>

NS_MIA_USE;

CSplineKernelTestPath splinekernel_init_path; 

struct TransformSplineFixtureFieldBase {
	TransformSplineFixtureFieldBase()
	{

	}
	void init(int dsize, double r, EInterpolation type) {
		ipf.reset(create_3dinterpolation_factory(type, bc_mirror_on_bounds));
		size = C3DBounds(2 * dsize + 1,2 * dsize + 1,2 * dsize + 1);
		field = C3DFVectorfield(size);
		range = r; 
		scale.x = scale.y = scale.z = range / dsize;

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

		cvinfo() << "scale = " << scale << "\n"; 		
		graddiv2sum *= scale.x * scale.y*scale.z; 
	}
	C3DBounds size;
	C3DFVectorfield field;
	P3DInterpolatorFactory ipf;
	C3DFVector field_range;
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

struct TransformSplineFixtureMixed: public TransformSplineFixtureFieldBase {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
	double graddiv2(double x, double y, double z)const;
};

struct TransformSplineFixtureMixed2: public TransformSplineFixtureFieldBase {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
	double graddiv2(double x, double y, double z)const;
};


BOOST_FIXTURE_TEST_CASE( test_bspline3_8_4_mix2,  TransformSplineFixtureMixed2 )
{
	init(8, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double h = 21.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 2.0);
	const double testdiv = h / 4.0; 
	const double testcurl = h; 


	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	BOOST_CHECK_CLOSE( div  * coeffs, testdiv, 0.5); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	BOOST_CHECK_CLOSE( divcurl  * coeffs, testcurl + testdiv, 0.5); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	BOOST_CHECK_CLOSE( rot * coeffs, testcurl, .5); 	
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_8_4, TransformSplineFixtureDivOnly )
{
	init(8, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double testvalue = 105.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 8.0); 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.5); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.5); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 1.0); 	
}


BOOST_FIXTURE_TEST_CASE( test_nocurl_bspline3_7_4, TransformSplineFixtureDivOnly )
{
	init(9, 4, ip_bspline3);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double testvalue = 105.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 8.0); 

	// sanity check 
	BOOST_CHECK_CLOSE(integrate(), testvalue, 0.1); 
	BOOST_CHECK_CLOSE(graddiv2sum, testvalue, 0.1); 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.8); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.8); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 4.0); 	
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


BOOST_FIXTURE_TEST_CASE( test_mix_bspline4_10_4, TransformSplineFixtureMixed )
{
	init(10, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double testdiv = 7.0 * pow(M_PI, 1.5) / sqrt(2.0); 
	const double testcurl = testdiv / 4.0; 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	double graddiv = div  * coeffs; 
	BOOST_CHECK_CLOSE( graddiv, testdiv, 0.1); 	


	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	double graddivcurl = divcurl  * coeffs; 
	BOOST_CHECK_CLOSE( graddivcurl, testdiv + testcurl, 0.1); 	
	

	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	double gradcurl = rot  * coeffs; 
	BOOST_CHECK_CLOSE( gradcurl, testcurl, 0.1); 	

	cvinfo() << "graddiv  / testdiv= " << graddiv  / testdiv << "\n"; 
	cvinfo() << "gradcurl / testcurl = " << gradcurl / testcurl << "\n"; 
}

#if 1
BOOST_FIXTURE_TEST_CASE( test_mix_bspline4_10_4_grad, TransformSplineFixtureMixed )
{
	init(4, 2, ip_bspline3);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	CDoubleVector gradient(3 * coeffs.size()); 
	
	divcurl.evaluate(coeffs, gradient); 


	const float delta = 0.001; 
	int i = 0; 
	for (auto c = coeffs.begin(); c != coeffs.end(); ++c, i+=3) {
		
		c->x += delta; 
		double vpx = divcurl  * coeffs;
		c->x -= 2.0*delta; 
		double vmx = divcurl  * coeffs;
		c->x += delta; 

		double test_x = (vpx - vmx) / (2.0 * delta); 
		if (fabs(test_x) < 1e-8) 
			BOOST_CHECK_CLOSE( gradient[i] + 1e-7, test_x + 1e-7, 5); 
		else
			BOOST_CHECK_CLOSE(gradient[i], test_x, 0.1); 

		c->y += delta; 
		double vpy = divcurl  * coeffs;
		c->y -= 2.0*delta; 
		double vmy = divcurl  * coeffs;
		c->y += delta; 
		
		double test_y = (vpy - vmy) / (2.0 * delta); 
		if (fabs(test_y) < 1e-8) 
			BOOST_CHECK_CLOSE( gradient[i+1] + 1e-7, test_y + 1e-7, 5); 
		else
			BOOST_CHECK_CLOSE(gradient[i+1], test_y, 0.1); 


		c->z += delta; 
		double vpz = divcurl  * coeffs;
		c->z -= 2.0*delta; 
		double vmz = divcurl  * coeffs;
		c->z += delta; 
		
		double test_z = (vpz - vmz) / (2.0 * delta); 
		if (fabs(test_z) < 1e-8  ) 
			BOOST_CHECK_CLOSE( gradient[i+2] + 1e-7, test_z + 1e-7, 5); 
		else
			BOOST_CHECK_CLOSE(gradient[i+2], test_z, 0.1); 


	}

	
}

#endif




struct TransformSplineFixtureFieldNonuniform {
	TransformSplineFixtureFieldNonuniform()
	{

	}
	void init(const C3DBounds& dsize, double r, EInterpolation type) {
		ipf.reset(create_3dinterpolation_factory(type, bc_mirror_on_bounds));
		size = C3DBounds(2*dsize.x + 1, 2*dsize.y + 1, 2*dsize.z + 1); 
		field = C3DFVectorfield(size);
		range = r; 
		scale.x = range / dsize.x; 
		scale.y = range / dsize.y; 
		scale.z = range / dsize.z;

		field_range.x = 2 * range; 
		field_range.y = 2 * range; 
		field_range.z = 2 * range; 
		
		double divcurl2sum = 0.0;
		C3DFVectorfield::iterator i = field.begin();
		for (int z = -(int)dsize.z; z <= (int)dsize.z; ++z)
			for (int y = -(int)dsize.y; y <= (int)dsize.y; ++y)
				for (int x = -(int)dsize.x; x <= (int)dsize.x; ++x, ++i) {
					double sx = x * scale.x;
					double sy = y * scale.y;
					double sz = z * scale.z;
					*i = C3DFVector( fx(sx, sy, sz), fy(sx, sy, sz), fz(sx, sy, sz));
					cvdebug() << "(" << sx << ", " << sy << ", " << sz << *i << "\n"; 
					divcurl2sum += divcurl2(sx, sy, sz); 
				}
		assert(i == field.end()); 
		source.reset(ipf->create(field));
		divcurl2sum *= scale.x * scale.y * scale.z; 
		
		BOOST_CHECK_CLOSE(divcurl2sum, 105.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 8.0), 1); 

		cvinfo() << "scale = " << scale << "\n"; 		
	}
	C3DBounds size;
	C3DFVectorfield field;
	P3DInterpolatorFactory ipf;
	C3DFVector field_range;
	double range; 
	std::shared_ptr<T3DInterpolator<C3DFVector>  > source; 
protected:
	virtual double fx(double x, double y, double z)const  = 0;
	virtual double fy(double x, double y, double z)const  = 0;
	virtual double fz(double x, double y, double z)const  = 0;
	virtual double divcurl2(double x, double y, double z)const  = 0;
	C3DFVector scale;
private:

};

struct TransformSplineFixtureMixedNonuniform: public TransformSplineFixtureFieldNonuniform {
	double fx(double x, double y, double z)const ;
	double fy(double x, double y, double z)const ;
	double fz(double x, double y, double z)const ;
	double divcurl2(double x, double y, double z)const;
};


double TransformSplineFixtureMixedNonuniform::fx(double x, double y, double z)const 
{
	return x * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixedNonuniform::fy(double x, double y, double z)const 
{
	return -z * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixedNonuniform::fz(double x, double y, double z)const
{
	return y * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixedNonuniform::divcurl2(double x, double y, double z)const
{
	double h = -x*x - y*y - z*z; 
	double exp2h = 4 * exp(2*h); 
	double f = 4 *pow(z,6) + (12*y*y + 12*x*x - 16) * pow(z,4) + 
		(12*pow(y,4) + (24 *x*x - 32) * y * y +12 *pow(x,4)-28*x*x+19) *z *z + 
		4 * pow(y,6) + (12*x*x-16)*pow(y,4)+ (12*x*x*x*x-28*x*x+19)*y*y + 4*pow(x,6)-12*pow(x,4)+17*x*x; 
	return f * exp2h; 
}

double TransformSplineFixtureMixed2::fx(double x, double y, double z)const 
{
	return x * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixed2::fy(double x, double y, double z)const 
{
	return -z * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixed2::fz(double x, double y, double z)const
{
	return y * exp(-x*x-y*y-z*z);
}

double TransformSplineFixtureMixed2::graddiv2(double x, double y, double z)const
{
	double h = -x*x - y*y - z*z; 
	double exp2h = 4 * exp(2*h); 
	double f = 4 *pow(z,6) + (12*y*y + 12*x*x - 16) * pow(z,4) + 
		(12*pow(y,4) + (24 *x*x - 32) * y * y +12 *pow(x,4)-28*x*x+19) *z *z + 
		4 * pow(y,6) + (12*x*x-16)*pow(y,4)+ (12*x*x*x*x-28*x*x+19)*y*y + 4*pow(x,6)-12*pow(x,4)+17*x*x; 
	return f * exp2h; 
}


BOOST_FIXTURE_TEST_CASE( test_bspline3_nonuniform, TransformSplineFixtureMixedNonuniform )
{
	C3DBounds size(8,9,7); 
	init(size, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double h = 21.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 2.0);
	const double testdiv = h / 4.0; 
	const double testcurl = h; 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	double realdiv = div  * coeffs; 
	BOOST_CHECK_CLOSE( realdiv, testdiv, 0.6); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	double realsum = divcurl  * coeffs; 
	BOOST_CHECK_CLOSE( realsum, testdiv + testcurl, 0.6); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	double realrot = rot * coeffs; 
	BOOST_CHECK_CLOSE( realrot, testcurl, 0.6); 	

	cvinfo() << "realdiv/ testdiv" << realdiv / testdiv << "\n"; 
	cvinfo() << "realsum/ sum" << realsum / (testdiv + testcurl) << "\n"; 
	cvinfo() << "realrot/ testcurl" << realrot/ testcurl << "\n"; 
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_uniform, TransformSplineFixtureMixedNonuniform )
{
	C3DBounds size(8,8,8); 
	init(size, 4, ip_bspline4);

	const T3DConvoluteInterpolator<C3DFVector>& interp = 
		dynamic_cast<const T3DConvoluteInterpolator<C3DFVector>&>(*source); 
	
	auto coeffs = interp.get_coefficients(); 
	const double h = 21.0 * pow(M_PI, 1.5) / (sqrt(2.0) * 2.0);
	const double testdiv = h / 4.0; 
	const double testcurl = h; 

	C3DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
	double realdiv = div  * coeffs; 
	BOOST_CHECK_CLOSE( realdiv, testdiv, 0.5); 	

	C3DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
	double realsum = divcurl  * coeffs; 
	BOOST_CHECK_CLOSE( realsum, testdiv + testcurl, 0.5); 	
	
	C3DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
	double realrot = rot * coeffs; 
	BOOST_CHECK_CLOSE( realrot, testcurl, 0.5); 	

	cvinfo() << "realdiv/ testdiv" << realdiv / testdiv << "\n"; 
	cvinfo() << "realsum/ sum" << realsum / (testdiv + testcurl) << "\n"; 
	cvinfo() << "realrot/ testcurl" << realrot/ testcurl << "\n"; 
}
