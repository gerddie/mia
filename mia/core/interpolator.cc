/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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
  The spline kernels are based on code by
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/

 */


#include <cmath>
#include <cassert>
#include <iomanip>
#include <mia/core/unaryfunction.hh>
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

int CBSplineKernel::get_active_halfrange() const   
{
	return (_M_support_size  + 1) / 2;
}

double CBSplineKernel::get_mult_int(int s1, int s2, int range, EIntegralType type) const
{
	int deg1 = 0; 
	int deg2 = 0; 
	switch (type) {
	case integral_11:
		deg1 = deg2 = 1; 
		break;
	case integral_20:
		deg1 = 2; 
		break; 
	case integral_02:
		deg2 = 2; 
		break; 
	default:
		assert(0 && "unknown integral type specified"); 
	}
	return integrate2(*this, s1, s2, deg1, deg2, 1, 0, range); 
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


double CBSplineKernel3::read_table(int skip, int delta, bool swapped, EIntegralType type) const
{
	// for simplicity this table is redundant 
	// columns refere to the absolute difference |s1-s2|
	// rows correspond to the skip at boundaries 
	// row 3 is no skip 
	const double integral_11[7][4] = {
		{  1.0/20.0,        0.0,       0.0,       0.0 },
		{  1.0/ 3.0,  7.0/120.0,       0.0,       0.0 },
		{ 37.0/60.0,-11.0/ 60.0, -1.0/10.0,       0.0 },
		{  2.0/ 3.0, -1.0/  8.0, -1.0/ 5.0, -1.0/120.0}, 
		{ 37.0/60.0,-11.0/ 60.0, -1.0/10.0,       0.0 },
		{  1.0/ 3.0,  7.0/120.0,       0.0,       0.0 },
		{  1.0/20.0,        0.0,       0.0,       0.0 }
	}; 

	const double integral_20[7][4] = {
		{  1.0/30.0,        0.0,       0.0,       0.0 },
		{ -1.0/ 3.0, 11.0/ 40.0,       0.0,       0.0 },
		{ -7.0/10.0, 11.0/ 60.0, 11.0/60.0,       0.0 },
		{ -2.0/ 3.0,  1.0/  8.0,  1.0/ 5.0, 1.0/120.0 }, 
		{ -7.0/10.0, -3.0/ 20.0,  1.0/60.0,       0.0 },
		{ -1.0/ 3.0, -7.0/120.0,       0.0,       0.0 },
		{  1.0/30.0,        0.0,       0.0,       0.0 }
	}; 

	switch (type) {
	case CBSplineKernel::integral_11:
		return integral_11[3+skip][delta]; 
	case CBSplineKernel::integral_02:
		swapped = !swapped; 
	case CBSplineKernel::integral_20:
		if (swapped) 
			return integral_20[3-skip][delta]; 
		else
			return integral_20[3+skip][delta]; 
	default:
		assert(0 && "unknown integral type specified"); 
	}
}

double CBSplineKernel::mult_int_from_table(int s1, int s2, int range, EIntegralType type) const
{
	const int hr = get_active_halfrange(); 
	const int max_skip = (size() + 1) & ~1; 
	bool swapped = false; 
	if (s2 < s1) {
		swapped = true; 
		swap(s1, s2); 
	}
	
	const int delta = s2 - s1; 
	if ( delta >= size() ) 
		return 0.0;
	
	int skip = 0; 
	const int dlow = hr - s1; 
	if (dlow > 0) {
		skip = s2 - hr;
		if (skip > 0) 
			skip = 0; 
	} else {
		const int dhigh = hr + s2 - range; 
		if (dhigh > 0) 
			skip = hr + s1 - range; 
		if (skip < 0) 
			skip = 0; 
	}
	if (abs(skip) >= max_skip) {
		cvdebug()<< "skip because abs(skip = " << skip << ")>=" << size() << " delta="<< delta<<"\n";  
		return 0.0; 
	}
	
	if (skip) 
		cvinfo() << "("<<s1<<"," << s2 << "@" << swapped <<") skip= " << skip << ", delta=" << delta <<"\n"; 
	return read_table(skip, delta, swapped, type); 
}

double CBSplineKernel::read_table(int skip, int delta, bool swapped, EIntegralType type) const
{
	assert(0 && "read_table needs to be implemented together with get_mult_int"); 
}

double CBSplineKernel3::get_mult_int(int s1, int s2, int range, EIntegralType type) const
{
	return mult_int_from_table(s1, s2, range, type); 
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
		weight[0] = 1 - x;
		weight[2] = 1 - 3.0 * x;
		weight[1] = 3 * x - 2;
	}break;
	case 3: {
		weight[3] =  1.0;
		weight[0] = -1.0;
		weight[2] = -3.0;
		weight[1] =  3.0;
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
		return 3.0*x-2.0;
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
			return -(2 * x * x + 240.0/96.0 - 480.0/96 * ax); 
		}
		if (ax <= 2.5) {
			double h = 2.5 - ax;
			return 0.5 * h * h;
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
	case 3:{
		weight[0] =  x - 1.0 / 2.0;
		const double t0 = 1;
		const double t1 = - 4.0 * x ;
		weight[1] = t1 + t0;
		weight[3] = t1 - t0;
		weight[4] = weight[0] + t0;
		weight[2] = - weight[0] - weight[1] - weight[3] - weight[4];
	}break;
	default:
		fill(weight.begin(), weight.end(), 0.0);
	}
}

double CBSplineKernel4::read_table(int skip, int delta, bool swapped, EIntegralType type) const
{
	const double integral_11[11][5] = {
		/*-5*/{ 1.0/32265.0,          0,              0.0,                 0.0, 0.0 },
		/*-4*/{ 19.0/336.0,       169.0/161280.0,              0.0,                 0.0, 0.0 },
		/*-3*/{ 35.0/144.0,       4771.0/80640.0,  17.0/53760.0,                0.0, 0.0 },
		/*-2*/{ 433.0/1008.0,     -1447.0/16128.0, -17.0/180.0,  -209.0/161280.0, 0.0 },
		/*-1*/{ 15678.0/32256.0, -1699.0/53760.0, -6103.0/32256.0, -1189.0/53760.0,  -1.0/10080 },
		/* 0*/{ 35.0/72.0,        -11.0/360.0, -17.0/90.0, -59.0/2520.0, -1.0/5040}, 
		/* 1*/{ 15678.0/32256.0, -1699.0/53760.0, -6103.0/32256.0, -1189.0/53760.0,  -1.0/10080 },
		/* 2*/{ 433.0/1008.0,     -1447.0/16128.0, -17.0/180.0,  -209.0/161280.0, 0.0 },
		/* 3*/{ 35.0/144.0,       4771.0/80640.0,  17.0/53760.0,                0.0, 0.0 },
		/* 4*/{ 19.0/336.0,       169.0/161280.0,              0.0,                 0.0, 0.0 },
		/* 5*/{ 1.0/32265.0,          0,              0.0,                 0.0, 0.0 }
	}; 


	const double integral_20[11][5] = {
		{       1.0/43008.0,                    0.0,        0.0,  0.0,       0.0 },
		{     551.0/16128.0,       0.0,       0.0,   0.0,          0.0 },
		{    -35.0/   144.0,       0.0,       0.0,   0.0,          0.0 },
		{  -2797.0/  5376.0,  11.0/40.0,       0.0,   0.0,         0.0 },
		{  -6679.0/ 13739.0,  11.0/60.0, 13.0/60.0,   0.0,         0.0 },
		{    -35.0/    72.0, 1789.0/63000.0, 1299.0/7000.0, 59.0/2625.0, 5207.0/28476000.0 }, 
		{  -6679.0/ 13739.0,  11.0/60.0, 13.0/60.0,   0.0,         0.0 },
		{  -2797.0/  5376.0,  11.0/40.0,       0.0,   0.0,         0.0 },
		{    -35.0/   144.0,       0.0,       0.0,   0.0,          0.0 },
		{     551.0/16128.0,       0.0,       0.0,   0.0,          0.0 },
		{      1.0/ 43008.0,       0.0,       0.0,  0.0,        0.0 }
	}; 

	
	switch (type) {
	case CBSplineKernel::integral_11:
		return integral_11[5 + skip][delta]; 
	case CBSplineKernel::integral_02:
		swapped = !swapped; 
	case CBSplineKernel::integral_20:
		if (swapped) 
			return integral_20[5 - skip][delta]; 
		else 
			return integral_20[5 + skip][delta]; 
	default:
		assert(0 && "unknown integral type specified"); 
	}
}

double CBSplineKernel4::get_mult_int(int s1, int s2, int range, EIntegralType type) const
{
	return mult_int_from_table(s1, s2, range, type); 
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


template <>
struct bspline<5, 0> {
	static double apply (double x)
	{

		const double ax = fabs(x);
		if (ax < 1.0) {
			const double x2 = ax*ax;
			const double x4 = x2 * x2;
			const double x5 = x4 * ax;
			return -(5 * x5 - 15 * x4  + 30 * x2 - 33)/60;
		}
		if (ax < 2.0) {
			double h = 2.0 - ax;
			double h2 = h * h;
			return -(5 * h2 * h * h2 - 5 * h2 * h2 - 10 * h2 * h  - 10 * h2 - 5 * h - 1) / 120.0;
		}

		if (ax < 3.0) {
			const double h = 3.0 - ax;
			const double h2 =  h * h;
			return h2 * h2 * h / 120.0;
		}
		return 0.0;
	}
};

template <>
struct bspline<5, 1> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax < 1.0) {
			const double x2 = x * x;
			const double x4 = x2 * x2;
			return ( x > 0 ? -1 : 1) * ( 5.0/12.0 * x4 - ax * (x2 - 1.0));
		}
		if (ax < 2.0) {
			double h = 2.0 - ax;
			double h2 = h * h;
			return ( x > 0 ? 1 : -1) *(5 * h2  * h2 - 4 * h * h2 - 6 * h2  - 4 * h - 1 ) / 24.0;
		}
		if (ax < 3.0) {
			const double h = 3.0 - ax;
			const double h2 =  h * h;
			return ( x > 0 ? -1 : 1) *  h2 * h2 / 24.0;
		}
		return 0.0;
	}
};


template <>
struct bspline<5, 2> {
	static double apply(double x)
	{
		double ax=fabs(x);
		if (ax < 1.0) {
			const double x2 = x * x;
			return -( 5.0/3.0 * x2 * ax - 3 * x2 + 1);
		}
		if (ax < 2.0) {
			double h = 2.0 - ax;
			double h2 = h * h;
			return -(5 * h2  * h - 3  * h2 - 3 * h  - 1 ) / 6.0;
		}
		if (ax < 3.0) {
			const double h = 3.0 - ax;
			const double h2 =  h * h;
			return   h2 * h / 6.0;
		}
		return 0.0;
	}
};


template <>
struct bspline<5, 3> {
	static double apply(double x)
	{
		const double f = x > 0  ? -1 : 1;
		const double ax=fabs(x);
		if (ax < 1.0) {
			const double x2 = x * x;
			return f * ( 5.0 * x2 - 6 * ax);
		}
		if (ax < 2.0) {
			double h = 2.0 - ax;
			double h2 = h * h;
			return -f * ( 5 * h2 - 2.0  * h - 1) / 2.0;
		}
		if (ax < 3.0) {
			const double h = 3.0 - ax;
			const double h2 =  h * h;
			return f * h2 / 2.0;
		}
		return 0.0;
	}
};


template <>
struct bspline<5, 4> {
	static double apply(double x)
	{
		const double f = x > 0  ? -1 : 1;
		const double ax=fabs(x);
		if (ax < 1.0) {
			const double x2 = x * x;
			return f * ( 5.0 * x2 - 6 * ax);
		}
		if (ax < 2.0) {
			double h = 2.0 - ax;
			double h2 = h * h;
			return -f * ( 5 * h2 - 2.0  * h - 1) / 2.0;
		}
		if (ax < 3.0) {
			const double h = 3.0 - ax;
			const double h2 =  h * h;
			return f * h2 / 2.0;
		}
		return 0.0;

	}
};

#if 0

template <>
struct bspline<5, 5> {
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
#endif

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
	case 3: {
		weight[5] = (1.0 / 2.0) * x * x;

		const double h2 = 2.0 * x - 1.0;
		const double w2 = x * x - x;
		const double h2w2 = 2 * h2 * w2;
		const double h2h22 = 2 * h2 * h2;
		const double t = 12 * h2;

		weight[0] = h2 / 2.0  - weight[5];

		const double t0 = h2 / 2.0;
		const double xm = x - 0.5;
		const double t1 = -( 10 * w2 -1) / 2.0;

		weight[2] = t0 + t1;
		weight[3] = t0 - t1;

		const double tt0 =  - t / 16.0;
		const double tt1 = (10 * w2 + 1) / 4.0;

		weight[1] = tt0 + tt1;
		weight[4] = tt0 - tt1;
	}break;

	default:
		fill(weight.begin(), weight.end(), 0.0);
	}
}

double CBSplineKernel5::get_weight_at(double x, int degree) const
{
	switch (degree) {
	case 0: return bspline<5,0>::apply(x);
	case 1: return bspline<5,1>::apply(x);
	case 2: return bspline<5,2>::apply(x);
	case 3: return bspline<5,3>::apply(x);
	case 4: return bspline<5,4>::apply(x);
	default:
		THROW(invalid_argument, "B-Spline5:derivative degree "
		      <<  degree << " not supported" );
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


struct F2DKernelIntegrator: public FUnary {
	F2DKernelIntegrator(const CBSplineKernel& spline, double s1, double s2, int deg1, int deg2):
		_M_spline(spline), _M_s1(s1), _M_s2(s2), _M_deg1(deg1), _M_deg2(deg2)
		{
		}
	virtual double operator() (double x) const {
		return _M_spline.get_weight_at(x - _M_s1, _M_deg1) *
			_M_spline.get_weight_at(x - _M_s2, _M_deg2);
	}
private:
	const CBSplineKernel& _M_spline;
	double _M_s1, _M_s2, _M_deg1, _M_deg2;
};


double  EXPORT_CORE integrate2(const CBSplineKernel& spline, double s1, double s2, int deg1, int deg2, double n, double x0, double L)
{
	double sum = 0.0;
	x0 /= n;
	L  /= n;

	// evaluate interval to integrate over
	double start_int = s1 - spline.get_nonzero_radius();
	double end_int = s1 + spline.get_nonzero_radius();
	if (start_int < s2 - spline.get_nonzero_radius())
		start_int = s2 - spline.get_nonzero_radius();
	if (start_int < x0)
		start_int = x0;
	if (end_int > s2 + spline.get_nonzero_radius())
		end_int = s2 + spline.get_nonzero_radius();
	if (end_int > L)
		end_int = L;

	// Simpson formula
	if (end_int <= start_int)
		return sum;
	const size_t intervals = size_t(8 * (end_int - start_int));

	sum = simpson( start_int, end_int, intervals, F2DKernelIntegrator(spline, s1, s2, deg1, deg2));
	cvdebug() << "integrate(" << start_int << ", " << end_int << ")[" << intervals <<"]="<< sum << "*" << n <<"\n"; 
	return sum * n;
}


NS_MIA_END
