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
#include <limits>
#include <mia/core/unaryfunction.hh>
#include <mia/core/interpolator.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/simpson.hh>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#define USE_FASTFLOOR

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

CBSplineKernel::SCache::SCache(size_t s, int cs1, int cs2):
	x(numeric_limits<double>::quiet_NaN()), 
	start_idx(-1000), 
	weights(s), 
	index(s), 
	csize1(cs1), 
	csize2(cs2), 
	is_mirrored(true)
{
}

CBSplineKernel::CBSplineKernel(size_t degree, double shift, EInterpolation type):
	m_half_degree(degree >> 1),
	m_shift(shift),
	m_support_size(degree + 1), 
	m_type(type)
{
}

CBSplineKernel::~CBSplineKernel()
{
}

EInterpolation CBSplineKernel::get_type() const
{
	return m_type; 
}

void CBSplineKernel::operator () (double x, std::vector<double>& weight, std::vector<int>& index)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_weights(x - ix, weight);
}

void CBSplineKernel::operator () (double x, SCache& cache) const
{
	if (x == cache.x)
		return; 
	int start_idx  = get_start_idx_and_value_weights(x, cache.weights); 
	cache.x = x; 
	if (start_idx == cache.start_idx) 
		return; 
	cache.start_idx = start_idx; 

	fill_index(start_idx, cache.index); 
	cache.is_mirrored = mirror_boundary_conditions(cache.index, cache.csize1, cache.csize2); 
}


#ifdef USE_FASTFLOOR
// code taken from http://www.stereopsis.com/FPU.html
// Michael Herf
const double _double2fixmagic = 68719476736.0*1.5;     //2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
const int    _shiftamt        = 16;                    //16.16 fixed point representation,

#if BIGENDIAN_
	#define iexp_				0
	#define iman_				1
#else
	#define iexp_				1
	#define iman_				0
#endif //BigEndian_

// ================================================================================================
// Real2Int
// ================================================================================================
inline int fastfloor(double val)
{
	union {
		double dval; 
		int ival[2]; 
	} v; 
	v.dval = val + _double2fixmagic;
	return v.ival[iman_] >> _shiftamt; 
}
#else
#define fastfloor floor
#endif

void CBSplineKernel::fill_index(int i, std::vector<int>& index) const 
{
	auto ib = index.begin(); 
	const auto ie = index.end(); 
	while (ib != ie) {
		*ib = i; 
		++ib; 
		++i; 
	}
}

int CBSplineKernel::get_indices(double x, std::vector<int>& index) const
{
	const int ix = fastfloor(x + m_shift);
	fill_index(ix - (int)m_half_degree, index); 
	return ix;
}

double CBSplineKernel::get_weight_at(double /*x*/, int degree) const
{
	THROW(invalid_argument, "B-Spline: derivative degree "
	      <<  degree << " not supported" );
}

const std::vector<double>& CBSplineKernel::get_poles() const
{
	return m_poles;
}

void CBSplineKernel::add_pole(double x)
{
	m_poles.push_back(x);
}

void CBSplineKernel::derivative(double x, std::vector<double>& weight, std::vector<int>& index)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight);
}

void CBSplineKernel::derivative(double x, std::vector<double>& weight, std::vector<int>& index, int degree)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight, degree);
}

double CBSplineKernel::get_nonzero_radius() const
{
	return m_support_size / 2.0;
}

int CBSplineKernel::get_active_halfrange() const   
{
	return (m_support_size  + 1) / 2;
}

int CBSplineKernel::get_start_idx_and_value_weights(double x, std::vector<double>& weights) const
{
	const int result = fastfloor(x + m_shift);
	get_weights(x - result, weights); 
	return result - (int)m_half_degree; 
}

int CBSplineKernel::get_start_idx_and_derivative_weights(double x, std::vector<double>& weights) const
{
	const int result = fastfloor(x + m_shift);
	get_derivative_weights(x - result, weights); 
	return result - (int)m_half_degree; 
}

CBSplineKernel0::CBSplineKernel0():
	CBSplineKernel(0, 0.5, ip_bspline0)
{
}
	
void CBSplineKernel0::get_weights(double /*x*/, std::vector<double>& weight)const
{
	assert(weight.size() > 0); 
	weight[0] = 1; 
}

void CBSplineKernel0::get_derivative_weights(double /*x*/, std::vector<double>& /*weight*/) const
{
	assert(false && "get_derivative_weights is not defined for the Haar spline"); 
	throw runtime_error("CBSplineKernel0::get_derivative_weights: not supported for Haar spline"); 
}

double CBSplineKernel0::get_weight_at(double x, int degree) const
{
	if (degree != 0) {
		THROW(invalid_argument, "CBSplineKernel0::get_weight_at: degree " <<  degree << 
		      "not supported for Haar spline"); 
	}
	return abs(x) < 0.5 ? 1.0 : 0.0; 
}
void CBSplineKernel0::get_derivative_weights(double /*x*/, std::vector<double>& weight, int degree) const
{
	if (degree == 0)
		weight[0] = 1.0; 
	else {
		THROW(invalid_argument, "CBSplineKernel0::get_derivative_weights: degree " <<  degree << 
		      "not supported for Haar spline"); 
	}
}

CBSplineKernel2::CBSplineKernel2():
	CBSplineKernel(2, 0.5, ip_bspline2)
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
	CBSplineKernel(3, 0.0, ip_bspline3)
{
	add_pole(sqrt(3.0) - 2.0);
}

#ifdef __SSE2__
typedef double v2df __attribute__ ((vector_size (16)));
// getthis stupid compiler to allocate the memory and set the values 
// once and for all. 
const double oneby6[2] __attribute__((aligned(16))) = { 1.0/6.0,  1.0/6.0 };
#endif

void CBSplineKernel3::get_weights(double x, std::vector<double>&  weight)const
{
	const double xm1 = 1 - x; 
#ifdef __SSE2__
	v2df X =_mm_set_pd(x, xm1); 

	const v2df OB6 =  __builtin_ia32_loadupd(oneby6);
	const v2df X2  = X * X; 
	const v2df XB6 = X * OB6; 
	const v2df W03 = X2 * XB6; 
	v2df W30 = W03; 
	W30 = _mm_shuffle_pd(W30, W30, 0x1); 

	const v2df W12 = X - W03 - W03 + W30; 
	
	_mm_storel_pd(&weight[0], W03); 
	_mm_storeh_pd(&weight[3], W03); 
	_mm_storeu_pd(&weight[1], W12); 
#else

	const double onebysix = 1.0/6.0; 
	weight[3] = onebysix * x * x * x;
	weight[0] = onebysix  * xm1 * xm1 * xm1;
	weight[2] = x - 2.0 * weight[3]  + weight[0] ;
	weight[1] = xm1 - 2 * weight[0]  + weight[3];
#endif
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
	CBSplineKernel(3, 0.0, ip_omoms3)
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
	CBSplineKernel(4, 0.5, ip_bspline4)
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



CBSplineKernel5::CBSplineKernel5():
	CBSplineKernel(5, 0.0, ip_bspline5)
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
		//const double h2w2 = 2 * h2 * w2;
		//const double h2h22 = 2 * h2 * h2;
		const double t = 12 * h2;

		weight[0] = h2 / 2.0  - weight[5];

		const double t0 = h2 / 2.0;
		//const double xm = x - 0.5;
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
		m_spline(spline), m_s1(s1), m_s2(s2), m_deg1(deg1), m_deg2(deg2)
		{
		}
	virtual double operator() (double x) const {
		return m_spline.get_weight_at(x - m_s1, m_deg1) *
			m_spline.get_weight_at(x - m_s2, m_deg2);
	}
private:
	const CBSplineKernel& m_spline;
	double m_s1, m_s2, m_deg1, m_deg2;
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
	return sum * n;
}


NS_MIA_END
