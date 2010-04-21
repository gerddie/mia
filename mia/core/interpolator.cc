/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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
  The spline kernels are based on code by 
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/

 */


#include <cmath>
#include <cassert>
#include <iomanip>
#include <mia/core/interpolator.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/simpson.hh>

NS_MIA_BEGIN
using namespace std; 

template <int sd, int degree> 
struct bspline {
	static double apply(double x) {
		THROW(invalid_argument, "Spline "<< sd << ":derivative degree " 
		      <<  degree << " not supported for spline of degree 2"); 
	}
}; 

template <> 
struct bspline<0,0> {
	static double apply(double x) {
		return fabs(x < 0.5) ? 1.0 : 0.0; 
	}
}; 

CBSplineKernel::CBSplineKernel(size_t degree, double shift):
	_M_half_degree(degree >> 1),
	_M_shift(shift),
	_M_support_size(degree + 1)
{
}

CBSplineKernel::~CBSplineKernel()
{
}

void CBSplineKernel::operator () (double x, std::vector<double>& weight, std::vector<int>& index)const
{
	assert(index.size() == _M_support_size);
	int ix = get_indices(x, index);
	get_weights(x - ix, weight);
}

int CBSplineKernel::get_indices(double x, std::vector<int>& index) const
{
	int ix = (int)floor(x + _M_shift);
	int i = ix - _M_half_degree;
	for (size_t k = 0; k < index.size(); ++k)
		index[k] = i++;
	return ix;
}

double CBSplineKernel::get_weight_at(double /*x*/, int degree) const
{
	THROW(invalid_argument, "B-Spline: derivative degree " 
	      <<  degree << " not supported" ); 
}

const std::vector<double>& CBSplineKernel::get_poles() const
{
	return _M_poles;
}

void CBSplineKernel::add_pole(double x)
{
	_M_poles.push_back(x);
}

void CBSplineKernel::derivative(double x, std::vector<double>& weight, std::vector<int>& index)const
{
	assert(index.size() == _M_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight);
}

void CBSplineKernel::derivative(double x, std::vector<double>& weight, std::vector<int>& index, int degree)const
{
	assert(index.size() == _M_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight, degree);
}

double CBSplineKernel::get_nonzero_radius() const
{
	return _M_support_size / 2.0;  
}

CBSplineKernel2::CBSplineKernel2():
	CBSplineKernel(2, 0.5)
{
	add_pole(sqrt(8.0) - 3.0);
}

void CBSplineKernel2::get_weights(double x, std::vector<double>&  weight)const
{
	weight[1] = 0.75 - x * x;
	weight[2] = 0.5 * (x - weight[1] + 1.0);
	weight[0] = 1.0 - weight[1] - weight[2];
}

template <> 
struct bspline<2,0> {
	static double apply(double x) {
		x=fabs(x);
		if (x <= 0.5)
			return 0.75-x*x;
		if (x <= 1.5)
			return (1.5-x)*(1.5-x) *0.5;
		return 0.0;
	}
}; 

template <> 
struct bspline<2,1> {
	static double apply(double x) {
		double xa=fabs(x) ;
		if (xa <= 0.5)
			return -2.0*x;
		if (xa <= 1.5)
			return (x>0.0) ? xa-1.5 : 1.5-xa;
		return 0.0;
	}
}; 


double CBSplineKernel2::get_weight_at(double x, int degree) const
{
	switch (degree) {
	case 0: return bspline<2,0>::apply(x); 
	case 1: return bspline<2,1>::apply(x); 
	default: 
		THROW(invalid_argument, "B-Spline 2:derivative degree " 
		      <<  degree << " not supported" ); 
	}
}

void CBSplineKernel2::get_derivative_weights(double x, std::vector<double>& weight) const
{
	weight[1] =  - 2 * x;
	weight[2] = 0.5 * (1 - weight[1]);
	weight[0] = - weight[1] - weight[2];
}

void CBSplineKernel2::get_derivative_weights(double x, std::vector<double>& weight, int degree) const
{
	switch (degree) {
	case 0: get_weights(x, weight); 
		break; 
	case 1: get_derivative_weights(x, weight); 
		break; 
	case 2: {
		weight[1] =  - 2;
		weight[2] = 1;
		weight[0] = 1;
	}break; 
	default: 
		weight[0] = weight[1] = weight[2] = 0.0; 
	}
}

CBSplineKernel3::CBSplineKernel3():
	CBSplineKernel(3, 0.0)
{
	add_pole(sqrt(3.0) - 2.0);
}

void CBSplineKernel3::get_weights(double x, std::vector<double>&  weight)const
{
	const double onebysix = 1.0 / 6.0;

	weight[3] = onebysix * x * x * x;
	weight[0] = onebysix + 0.5 * x * (x - 1.0) - weight[3];
	weight[2] = x + weight[0] - 2.0 * weight[3];
	weight[1] = 1.0 - weight[0] - weight[2] - weight[3];
}

void CBSplineKernel3::get_derivative_weights(double x, std::vector<double>& weight) const
{
	weight[3] = 0.5 * x * x;
	weight[0] = x - 0.5 - weight[3];
	weight[2] = 1 + weight[0] - 2.0 * weight[3];
	weight[1] = - weight[0] - weight[2] - weight[3];
}

void CBSplineKernel3::get_derivative_weights(double x, std::vector<double>& weight, int degree) const
{
	switch (degree) {
	case 0: get_weights(x, weight); 
		break; 
	case 1: get_derivative_weights(x, weight); 
		break; 
	case 2: {
		weight[3] = x;
		weight[0] = 1 - weight[3];
		weight[2] = weight[0] - 2.0 * weight[3];
		weight[1] = - weight[0] - weight[2] - weight[3];
	}break; 
	case 3: {
		weight[3] = 1;
		weight[0] = - weight[3];
		weight[2] = weight[0] - 2.0 * weight[3];
		weight[1] = - weight[0] - weight[2] - weight[3];
	}break; 
	default: {
		fill(weight.begin(), weight.end(), 0.0); 
	}
	}
}

template <>
struct bspline<3, 0> {
	static double apply (double x) /* cubic */
	{
		const double onebysix = 1.0/6.0;
		x=fabs(x);
		if (x <= 1.0)
			return 2.0 / 3.0 - x * x * ( 1 - 0.5 * x );

		if (x < 2.0) {
			const double dx =  2.0-x; 
			return dx*dx*dx * onebysix;
		}
		return 0.0;
	}
};

template <>
struct bspline<3, 1> {
	static double apply(double x) /* cubic, derivative */
	{
		double ax = fabs(x);
		if (ax <= 1.0)
			return (1.5 * ax - 2) * x;
		if (ax <= 2.0)
			return (x > 0.0 ? -0.5 : 0.5) * (2-ax) * (2-ax) ;
		return 0.0;
	}
};

template <>
struct bspline<3, 2> {
	static double apply(double x) /* cubic, second derivative */
	{
		x=fabs(x) ;
		if (x>2.0) return 0.0;
		if (x>1.0) return 2.0-x ;
		return 3.0*x-2.0 ;
	}
};

double CBSplineKernel3::get_weight_at(double x, int degree) const
{
	switch (degree) {
	case 0: return bspline<3,0>::apply(x); 
	case 1: return bspline<3,1>::apply(x); 
	case 2: return bspline<3,2>::apply(x); 
	default: 
		THROW(invalid_argument, "B-Spline 3:derivative degree " 
		      <<  degree << " not supported" ); 
	}
}


CBSplineKernelOMoms3::CBSplineKernelOMoms3():
	CBSplineKernel(3, 0.0)
{
	add_pole((sqrt(105.0) - 13.0)/8.0);
}

void CBSplineKernelOMoms3::get_weights(double x, std::vector<double>&  weight)const
{
	double x2 = x*x;
	double x3 = x2 * x;

	weight[1] =   x3 * 0.5 - x2  + 1.0 / 14.0 * x + 13.0/21.0;
	weight[2] = - x3 *0.5 + 0.5 * x2  + 3.0 / 7.0 *x + 4.0 / 21.0;
	weight[3] =   x3 / 6.0 + x / 42.0;
	weight[0] = 1.0 - weight[3] - weight[1] - weight[2];

}

void CBSplineKernelOMoms3::get_derivative_weights(double x, std::vector<double>& weight) const
{
	double x2 = 2.0 * x;
	double x3 = 3.0 * x * x;

	weight[1] =   x3 * 0.5 - x2  + 1.0 / 14.0;
	weight[2] = - x3 * 0.5 + 0.5 * x2  + 3.0 / 7.0;
	weight[3] =   x3 / 6.0 + 1.0 / 42.0;
	weight[0] =  - weight[3] - weight[1] - weight[2];
}

void CBSplineKernelOMoms3::get_derivative_weights(double x, std::vector<double>& weight, int degree) const
{
	switch (degree) {
	case 0: get_weights(x, weight); 
		break; 
	case 1: get_derivative_weights(x, weight); 
		break; 
	case 2: {
		double x2 = 2.0;
		double x3 = 6.0 * x;
		
		weight[1] =   x3 * 0.5 - x2;
		weight[2] = - x3 * 0.5 + 0.5 * x2;
		weight[3] =   x3 / 6.0;
		weight[0] =  - weight[3] - weight[1] - weight[2];
	}break; 
	default: 
		fill(weight.begin(), weight.end(), 0.0); 
	}
}

template <>
struct bspline<4, 0> {
	static double apply (double x)
	{
		x=fabs(x);
		if (x <= 0.5) {
			const double f = 1.0 / 192.0; 
			const double x2 = x * x; 
			return   f * (115.0 + (48.0 * x2 - 120) * x2); 
		}
		if (x <= 1.5) {
			const double f = 1.0 / 96.0; 
			double ax = x - 1.0; 
			return -f * ((((16 * ax - 16) * ax -24) * ax + 44)* ax - 19); 
		}
		if (x <= 2.5) {
			double h = 2.5 - x;
			h *= h; 
			return h * h / 24.0; 
		}
		return 0.0;
	}
};

template <>
struct bspline<4, 1> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax <= 0.5) {
			const double x2 = x * x; 
			return   x * (x2  - 1.25); 
		}
		if (ax <= 1.5) {
			const double f = x > 0 ? - 1.0 / 96.0 : 1.0 / 96.0; 
			double bx =  ax - 1.0; 
			return f * (64 * bx * bx * bx - 48 * bx * bx - 48 * bx + 44); 
		}
		if (ax <= 2.5) {
			double h = x < 0 ? 2.5 - ax : ax - 2.5;
			return h * h * h / 6.0; 
		}
		return 0.0;
	}
};

template <>
struct bspline<4, 2> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax <= 0.5)
			return   3 * x * x  - 1.25; 
		if (ax <= 1.5) {
			const double f = - 1.0 / 2.0; 
			double bx =  ax - 1.0; 
			return f * (4 * bx * bx - 2 * bx - 1); 
		}
		if (ax <= 2.5) {
			double h = x > 0 ? 2.5 - ax : ax - 2.5;
			return h * h / 2.0; 
		}
		return 0.0;
	}
};


template <>
struct bspline<4, 3> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax <= 0.5)
			return   6 * x; 
		if (ax <= 1.5) {
			const double f = x > 0 ? -0.5: 0.5; 
			double bx =  ax - 1.0; 
			return f * (8 * bx - 2); 
		}
		if (ax <= 2.5) 
			return  x < 0 ? 2.5 - ax : ax - 2.5;
		return 0.0;
	}
};

template <>
struct bspline<4, 4> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax <= 0.5)
			return  6; 
		if (ax <= 1.5)
			return  -4; 
		if (ax <= 2.5)
			return  1;
		return 0.0;
	}
};


double CBSplineKernel4::get_weight_at(double x, int degree) const
{
	switch (degree) {
	case 0: return bspline<4,0>::apply(x); 
	case 1: return bspline<4,1>::apply(x); 
	case 2: return bspline<4,2>::apply(x); 
	case 3: return bspline<4,3>::apply(x); 
	case 4: return bspline<4,4>::apply(x); 
	default: 
		THROW(invalid_argument, "B-Spline 3:derivative degree " 
		      <<  degree << " not supported" ); 
	}
}

CBSplineKernel4::CBSplineKernel4():
	CBSplineKernel(4, 0.5)
{
	add_pole(sqrt(664.0 - sqrt(438976.0)) + sqrt(304.0) - 19.0);
	add_pole(sqrt(664.0 + sqrt(438976.0)) - sqrt(304.0) - 19.0);
}

void CBSplineKernel4::get_weights(double x, std::vector<double>&  weight)const
{
	double x2 = x * x;
	double t = (1.0 / 6.0) * x2;

	weight[0] = 1.0 / 2.0 - x;
	weight[0] *= weight[0];
	weight[0] *= (1.0 / 24.0) * weight[0];

	double t0 = x * (t - 11.0 / 24.0);
	double t1 = 19.0 / 96.0 + x2 * (1.0 / 4.0 - t);
	weight[1] = t1 + t0;
	weight[3] = t1 - t0;
	weight[4] = weight[0] + t0 + (1.0 / 2.0) * x;
	weight[2] = 1.0 - weight[0] - weight[1] - weight[3] - weight[4];
}

void CBSplineKernel4::get_derivative_weights(double x, std::vector<double>& weight) const
{
	const double x2 = x * x;
	weight[0] = 1.0 / 2.0 - x;
	weight[0] *= - weight[0] * weight[0] / 6.0;

	const double t = x2 / 6.0;
	const double t0 = (t - 11.0 / 24.0) + x2 / 3.0;

	const double t1 = 2.0 * x * (0.25 - t) - x2 * x / 3.0;

	weight[1] = t1 + t0;
	weight[3] = t1 - t0;
	weight[4] = weight[0] + t0 + 0.5;
	weight[2] = - weight[0] - weight[1] - weight[3] - weight[4];

}

void CBSplineKernel4::get_derivative_weights(double x, std::vector<double>& weight, int degree) const
{
	switch (degree) {
	case 0: get_weights(x, weight); 
		break; 
	case 1: get_derivative_weights(x, weight); 
		break; 
	case 2: {
		weight[0] = 1.0 / 2.0 - x;
		weight[0] *= weight[0] / 2.0;
		const double t0 = x;
		const double t1 = 0.5 - 2.0 * x * x ;
		weight[1] = t1 + t0;
		weight[3] = t1 - t0;
		weight[4] = weight[0] + t0;
		weight[2] = - weight[0] - weight[1] - weight[3] - weight[4];
		cvdebug() << "weight[2] = " << fixed << weight[2] << "\n"; 
	}break; 
	default: 
		fill(weight.begin(), weight.end(), 0.0); 
	}
}
	
CBSplineKernel5::CBSplineKernel5():
	CBSplineKernel(5, 0.0)
{
	add_pole((sqrt(270.0 - sqrt(70980.0)) + sqrt(105.0)- 13.0) / 2.0);
	add_pole((sqrt(270.0 + sqrt(70980.0)) - sqrt(105.0)- 13.0) / 2.0);
}


void CBSplineKernel5::get_weights(double x, std::vector<double>&  weight)const
{
	double w2 = x * x;
	weight[5] = (1.0 / 120.0) * x * w2 * w2;
	w2 -= x;
	double w4 = w2 * w2;
	x -= 1.0 / 2.0;
	double t = w2 * (w2 - 3.0);
	weight[0] = (1.0 / 24.0) * (1.0 / 5.0 + w2 + w4) - weight[5];
	double t0 = (1.0 / 24.0) * (w2 * (w2 - 5.0) + 46.0 / 5.0);
	double t1 = (-1.0 / 12.0) * x * (t + 4.0);
	weight[2] = t0 + t1;
	weight[3] = t0 - t1;
	t0 = (1.0 / 16.0) * (9.0 / 5.0 - t);
	t1 = (1.0 / 24.0) * x * (w4 - w2 - 5.0);
	weight[1] = t0 + t1;
	weight[4] = t0 - t1;
}

void CBSplineKernel5::get_derivative_weights(double x, std::vector<double>& weight) const
{
	double w2 = x * x;
	weight[5] = (1.0 / 24.0) * w2 * w2;

	double h2 = 2.0 * x - 1.0;
	w2 -= x;
	double t = h2 * (2.0 * w2 - 3.0);

	weight[0] = (h2 + 2.0 * w2 * h2) / 24.0  - weight[5];
	x -= 0.5;

	double t0 =   h2 * (2.0 * w2 - 5.0) / 24.0;
	double t1 = - 1.0 / 3.0 - (w2 * (w2 - 3.0)  + x * t) / 12.0;

	weight[2] = t0 + t1;
	weight[3] = t0 - t1;

	t0 =  - t / 16.0;
	t1 = ((w2 * w2 - w2 - 5.0) + x * (2 * w2 * h2 - h2)) / 24.0;

	weight[1] = t0 + t1;
	weight[4] = t0 - t1;

}

void CBSplineKernel5::get_derivative_weights(double x, std::vector<double>& weight, int degree) const
{
	switch (degree) {
	case 0: get_weights(x, weight); 
		break; 
	case 1: get_derivative_weights(x, weight); 
		break; 
	case 2: {
		weight[5] = (1.0 / 6.0) * x * x * x; 
		
		const double h2 = 2.0 * x - 1.0;
		const double w2 = x * x - x;
		const double h2w2 = 2 * h2 * w2; 
		const double h2h22 = 2 * h2 * h2; 
		const double t = 4 * w2  + h2h22 - 6; 

		weight[0] = (t + 8.0) / 24.0  - weight[5];
		
		const double t0 = (t - 4.0 )/ 24.0;
		const double xm = x - 0.5;
		const double t1 = ( - 2.0 * h2w2 +  6 * h2 - xm * t) / 12; 
		
		weight[2] = t0 + t1;
		weight[3] = t0 - t1;
		
		const double tt0 =  - t / 16.0;
		const double tt1 = (2 * (h2w2 - h2) + xm * (t + 4)) / 24.0;
		
		weight[1] = tt0 + tt1;
		weight[4] = tt0 - tt1;		
	}break; 
	default: 
		fill(weight.begin(), weight.end(), 0.0); 
	}
}


static TDictMap<EInterpolation>::Table InterpolationOptions[] = {
	{"nn", ip_nn},
	{"tri", ip_linear},
	{"bspline2", ip_bspline2},
	{"bspline3", ip_bspline3},
	{"bspline4", ip_bspline4},
	{"bspline5", ip_bspline5},
	{"omoms3", ip_omoms3},
	{NULL, ip_unknown}
};

EXPORT_CORE TDictMap<EInterpolation> GInterpolatorTable(InterpolationOptions);

NS_MIA_END
