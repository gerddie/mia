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

#include <mia/2d/transformmock.hh>

#include <mia/internal/plugintester.hh>
#include <mia/2d/splinepenalty/divcurl.hh>
#include <mia/2d/interpolator.hh>

using namespace divcurl_splinepenalty; 
NS_MIA_USE
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_divcurl_cost ) 
{
	C2DDivcurlSplinePenalty  penalty(1.0, false, 1.0, 1.0); 

	C2DBounds size(31,31); 
	C2DFVector range(8,8); 
	C2DFVectorfield coef(size); 
	
	double scale = range.x / (size.x - 1); 
	double mid = (size.x - 1.0) / 2.0; 

	auto ic = coef.begin(); 
	for(unsigned int y = 0; y < size.y; ++y) {
		double fy = (float(y) - mid) * scale; 
		for(unsigned  x = 0; x < size.x; ++x, ++ic) {
			double fx = (float(x) - mid) * scale;
			double v = exp(- fx * fx - fy * fy); 
			*ic = C2DFVector( fx * v, fy * v);
		}
	}
	auto kernel = produce_spline_kernel("bspline:d=4"); 

	T2DConvoluteInterpolator<C2DFVector> source(coef, kernel);
	auto coeff_double = source.get_coefficients(); 
	transform(coeff_double.begin(), coeff_double.end(), coef.begin(), 
		  [](const C2DDVector& x) { return C2DFVector(x);}); 
	
	penalty.initialize(size, range, kernel); 
	BOOST_CHECK_CLOSE(penalty.value(coef), 6 * M_PI, 0.5); 

}

BOOST_AUTO_TEST_CASE( test_divcurl_cost_scale_weight ) 
{
	C2DDivcurlSplinePenalty  penalty(.5, false, 1.0, 1.0); 

	C2DBounds size(31,31); 
	C2DFVector range(8,8); 
	C2DFVectorfield coef(size); 
	
	double scale = range.x / (size.x - 1); 
	double mid = (size.x - 1.0) / 2.0; 

	auto ic = coef.begin(); 
	for(unsigned int y = 0; y < size.y; ++y) {
		double fy = (float(y) - mid) * scale; 
		for(unsigned  x = 0; x < size.x; ++x, ++ic) {
			double fx = (float(x) - mid) * scale;
			double v = exp(- fx * fx - fy * fy); 
			*ic = C2DFVector( fx * v, fy * v);
		}
	}
	auto kernel = produce_spline_kernel("bspline:d=4"); 

	T2DConvoluteInterpolator<C2DFVector> source(coef, kernel);
	auto coeff_double = source.get_coefficients(); 
	transform(coeff_double.begin(), coeff_double.end(), coef.begin(), 
		  [](const C2DDVector& x) { return C2DFVector(x);}); 
	
	penalty.initialize(size, range, kernel); 
	BOOST_CHECK_CLOSE(penalty.value(coef), 3 * M_PI, 0.5); 

}

BOOST_AUTO_TEST_CASE( test_divcurl_cost_scale_div_weight ) 
{
	C2DDivcurlSplinePenalty  penalty(1.0, false, 0.5, 1.0); 

	C2DBounds size(31,31); 
	C2DFVector range(8,8); 
	C2DFVectorfield coef(size); 
	
	double scale = range.x / (size.x - 1); 
	double mid = (size.x - 1.0) / 2.0; 

	auto ic = coef.begin(); 
	for(unsigned int y = 0; y < size.y; ++y) {
		double fy = (float(y) - mid) * scale; 
		for(unsigned  x = 0; x < size.x; ++x, ++ic) {
			double fx = (float(x) - mid) * scale;
			double v = exp(- fx * fx - fy * fy); 
			*ic = C2DFVector( fx * v, fy * v);
		}
	}
	auto kernel = produce_spline_kernel("bspline:d=4"); 

	T2DConvoluteInterpolator<C2DFVector> source(coef, kernel);
	auto coeff_double = source.get_coefficients(); 
	transform(coeff_double.begin(), coeff_double.end(), coef.begin(), 
		  [](const C2DDVector& x) { return C2DFVector(x);}); 
	
	penalty.initialize(size, range, kernel); 
	BOOST_CHECK_CLOSE(penalty.value(coef), 3 * M_PI, 0.5); 

}

BOOST_AUTO_TEST_CASE( test_divcurl_cost_scale_curl_weight ) 
{
	C2DDivcurlSplinePenalty  penalty(1.0, false, 1.0, 0.5); 

	C2DBounds size(31,31); 
	C2DFVector range(8,8); 
	C2DFVectorfield coef(size); 
	
	double scale = range.x / (size.x - 1); 
	double mid = (size.x - 1.0) / 2.0; 

	auto ic = coef.begin(); 
	for(unsigned int y = 0; y < size.y; ++y) {
		double fy = (float(y) - mid) * scale; 
		for(unsigned  x = 0; x < size.x; ++x, ++ic) {
			double fx = (float(x) - mid) * scale;
			double v = exp(- fx * fx - fy * fy); 
			*ic = C2DFVector( fy * v, -fx * v);
		}
	}
	auto kernel = produce_spline_kernel("bspline:d=4"); 

	T2DConvoluteInterpolator<C2DFVector> source(coef, kernel);
	auto coeff_double = source.get_coefficients(); 
	transform(coeff_double.begin(), coeff_double.end(), coef.begin(), 
		  [](const C2DDVector& x) { return C2DFVector(x);}); 
	
	penalty.initialize(size, range, kernel); 
	BOOST_CHECK_CLOSE(penalty.value(coef), 3 * M_PI, 0.5); 

}
