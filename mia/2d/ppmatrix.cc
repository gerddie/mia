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


#include <cmath>
#include <mia/2d/ppmatrix.hh>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif


NS_MIA_BEGIN

class C2DPPDivcurlMatrixImpl {
public: 
	C2DPPDivcurlMatrixImpl(const C2DBounds& size, const C2DFVector& range, 
			       const CBSplineKernel& kernel, double wd, double wr); 

	template <typename Field>
	double multiply(const Field& coefficients) const; 
	double evaluate(const C2DFVectorfield& coefficients, gsl::DoubleVector& gradient) const; 
	double evaluate(const T2DDatafield<C2DDVector>& coefficients, gsl::DoubleVector& gradient) const; 
	void reset(const C2DBounds& size, const C2DFVector& range, const CBSplineKernel& kernel, 
		   double wd, double wr); 
	C2DBounds _M_size; 
private: 
	double _M_wd; 
	double _M_wr; 
	EInterpolation _M_type; 
	C2DFVector _M_range; 
	size_t _M_nodes; 
	
	struct SMatrixCell {
		C2DDVector v; 
		double v12; 
		size_t i; 
		size_t j; 
	}; 
	vector<SMatrixCell> _M_P; 
}; 


C2DPPDivcurlMatrix::C2DPPDivcurlMatrix(const C2DBounds& size, const C2DFVector& range, 
				       const CBSplineKernel& kernel, double wd, double wr):
	impl(new C2DPPDivcurlMatrixImpl(size, range, kernel, wd, wr))
{
	TRACE_FUNCTION; 
}
	
C2DPPDivcurlMatrix::~C2DPPDivcurlMatrix()
{
	TRACE_FUNCTION; 
	delete impl; 
}

double C2DPPDivcurlMatrix::operator * (const C2DFVectorfield& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}

double C2DPPDivcurlMatrix::operator * (const T2DDatafield<C2DDVector>& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}
double C2DPPDivcurlMatrix::evaluate(const T2DDatafield<C2DDVector>& coefficients, gsl::DoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	return impl->evaluate(coefficients, gradient); 
}

double C2DPPDivcurlMatrix::evaluate(const C2DFVectorfield& coefficients, gsl::DoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	return impl->evaluate(coefficients, gradient); 
}


/**\todo helper class to evaluate values only once, should be re-done and moved to the spline kernel */
class CIntegralCache {
public: 
	CIntegralCache(const CBSplineKernel& kernel); 
	double get(int s1, int s2, int deg1, int deg2, int range) const; 
private: 
	const CBSplineKernel& _M_kernel; 
	int _M_max_skip; 
	int _M_row_length; 
	int _M_hr; 
	mutable map<int,double> _M_values; 
}; 

CIntegralCache::CIntegralCache(const CBSplineKernel& kernel):
	_M_kernel(kernel), 
	_M_max_skip((kernel.size() + 1) & ~1), 
	_M_row_length(kernel.size()), 
	_M_hr(kernel.get_active_halfrange())
	
{
}

double CIntegralCache::get(int s1, int s2, int deg1, int deg2, int range) const
{
	bool swapped = false; 
	if (s2 < s1) {
		swapped = true; 
		swap(s1, s2); 
	}
	
	int delta = s2 - s1; 
	if ( delta >= _M_row_length ) 
		return 0.0;
	
	int skip = 0; 
	const int dlow = _M_hr - s1; 
	if (dlow > 0) {
		skip = s2 - _M_hr;
		if (skip > 0) 
			skip = 0; 
	} else {
		const int dhigh = _M_hr + s2 - range; 
		if (dhigh > 0) 
			skip = _M_hr + s1 - range; 
		if (skip < 0) 
			skip = 0; 
	}
	if (abs(skip) >= _M_max_skip) {
		cvdebug()<< "skip because abs(skip = " << skip << ")>=" << _M_row_length << " delta="<< delta<<"\n";  
		return 0.0; 
	}
	
	if (swapped) {
		delta = - delta;
		swap(s1, s2); 
	}
	
	int index = skip * 2*_M_row_length + delta + _M_row_length;
	auto pv = _M_values.find(index);
	if (pv != _M_values.end())
		return pv->second; 
	double result = _M_values[index] = integrate2(_M_kernel, s1, s2, deg1, deg2, 1, 0,  range); 
	return result; 
}


C2DPPDivcurlMatrixImpl::C2DPPDivcurlMatrixImpl(const C2DBounds& size, const C2DFVector& range, 
					       const CBSplineKernel& kernel,
					       double wd, double wr)
{
	reset(size, range, kernel,  wd,  wr); 
}						

void C2DPPDivcurlMatrixImpl::reset(const C2DBounds& size, const C2DFVector& range, const CBSplineKernel& kernel, 
				   double wd, double wr)
{
	if (  _M_size == size && wd == _M_wd && wr == _M_wr && _M_range == range && kernel.get_type()  == _M_type)
		return; 
	_M_size = size;
	_M_wd = wd; 
	_M_wr = wr; 
	_M_range = range; 
	_M_type = kernel.get_type(); 
	_M_nodes = size.x*size.y; 
	_M_P.clear(); 

	C2DFVector h1((size.x-1)/range.x,(size.y-1)/range.y);
	double global_scale = 1.0 / (h1.x * h1.y); 

	C2DFVector h2(h1.x*h1.x, h1.y*h1.y);
	C2DFVector h3(h1.x*h2.x, h1.y*h2.y);
	C2DFVector h4(h1.x*h3.x, h1.y*h3.y);
	

	int ny = _M_size.y; 
	int nx = _M_size.x; 
	int kernel_range = kernel.size(); 
	
	CIntegralCache rc22(kernel); 
	CIntegralCache rc21(kernel); 
	CIntegralCache rc01(kernel); 
	CIntegralCache rc12(kernel); 
	CIntegralCache rc10(kernel); 

	CIntegralCache rc00(kernel); 
	CIntegralCache rc11(kernel); 

	
	for (int l = 0, i=0; l < ny; ++l) {
		for (int n = max(0,l - kernel_range); n < min(l + kernel_range, ny); ++n) {
			double r00y =        rc00.get( l, n, 0, 0, size.y); 
			double r01y = h1.y * rc01.get( l, n, 0, 1,size.y); 
			double r11y = h2.y * rc11.get( l, n, 1, 1, size.y); 
			double r21y = h3.y * rc21.get( l, n, 2, 1, size.y); 
			double r22y = h4.y * rc22.get( l, n, 2, 2, size.y); 

			for (int k = 0; k < nx; ++k) {
				for (int m = max(0,k - kernel_range); m < min(k + kernel_range,nx); ++m) {
					
					double r00x =        rc00.get( k, m, 0, 0, size.x); 
					double r01x = h1.x * rc01.get( k, m, 0, 1,size.x); 
					double r11x = h2.x * rc11.get( k, m, 1, 1, size.x); 
					double r21x = h3.x * rc21.get( k, m, 2, 1, size.x); 
					double r22x = h4.x * rc22.get( k, m, 2, 2, size.x); 

					
					SMatrixCell cell; 
					cell.v.x = global_scale * (
						wd * (r22x * r00y + r11x * r11y) + 
						wr * (r11x * r11y + r00x * r22y)); 
					
					cell.v12 = 2 * global_scale * ( wd - wr ) * ( r21x * r01y + r01x * r21y) ; 
					
					cell.v.y = global_scale * (
						wd * (r00x * r22y + r11x * r11y) + 
						wr * (r22x * r00y + r11x * r11y));
					cell.i = k + l * nx; 
					cell.j = m + n * nx;  
					if (cell.v.x != 0.0 || cell.v12 != 0.0 ||cell.v.y != 0.0) 
						_M_P.push_back(cell); 
				}
			}
		}
	}
}

template <typename Field>
double C2DPPDivcurlMatrixImpl::multiply(const Field& coefficients) const
{
	assert(coefficients.size() == _M_nodes); 

	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		result_1 += ci.x * cj.x * p->v.x; 
		result_2 += ci.x * cj.y * p->v12; 
		result_3 += ci.y * cj.y * p->v.y; 
	}
	return result_1 + result_2 + result_3; 
}


double C2DPPDivcurlMatrixImpl::evaluate(const T2DDatafield<C2DDVector>& coefficients, 
					gsl::DoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 2); 
#if defined(__SSE2__)
	register __m128d result_a = {0.0, 0.0}; 
	register __m128d result_b = result_a; 
	
	auto p = _M_P.begin(); 
	auto pe = _M_P.end(); 

	
	while (p != pe) {
		const __m128d ci = _mm_loadu_pd(&coefficients[p->i].x); 
		const __m128d cj = _mm_loadu_pd(&coefficients[p->j].x); 
		const __m128d pv = _mm_loadu_pd(&p->v.x);
		__m128d g = _mm_loadu_pd(&gradient[2*p->i]); 
		const __m128d pv12 = _mm_set1_pd(p->v12); 
		
		__m128d cjpv = cj * pv; 
		__m128d cjpv12 = cj * pv12; 
		cjpv12 = _mm_shuffle_pd(cjpv12, cjpv12, 0x1); 
		
		result_a = result_a + ci * cjpv; 
		cjpv = cjpv + cjpv; 
		g = g + cjpv12; 
		result_b = _mm_add_sd(result_b, _mm_mul_sd(ci, cjpv12)); 
		g = g + cjpv; 
		
		_mm_storeu_pd(&gradient[2*p->i], g); 
		++p; 
	}
	double result_12[2]; 
	result_a = _mm_add_sd(result_a, result_b); 
	_mm_storeu_pd(result_12, result_a); 
	return result_12[0] + result_12[1]; 
#else
	register double result_1 = 0.0; 
	register double result_2 = 0.0; 
	register double result_3 = 0.0; 
	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		const double cjxpv11 = cj.x * p->v.x; 
		const double cjypv22 = cj.y * p->v.y; 

		const double cjxpv12 = cj.x * p->v12; 
		const double cjypv12 = cj.y * p->v12; 


		gradient[2*p->i    ] += cjxpv11 + cjxpv11 + cjypv12; 
		gradient[2*p->i + 1] += cjypv22 + cjypv22 + cjxpv12; 

		result_1 += ci.x * cjxpv11; 
		result_2 += ci.x * cjypv12; 
		result_3 += ci.y * cjypv22; 
	}

	return result_1 + result_2 + result_3; 
#endif
}

double C2DPPDivcurlMatrixImpl::evaluate(const C2DFVectorfield& coefficients, 
					gsl::DoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 2); 
	register double result_1 = 0.0; 
	register double result_2 = 0.0; 
	register double result_3 = 0.0; 
	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		const double cjxpv11 = cj.x * p->v.x; 
		const double cjypv22 = cj.y * p->v.y; 

		const double cjxpv12 = cj.x * p->v12; 
		const double cjypv12 = cj.y * p->v12; 


		gradient[2*p->i    ] += cjxpv11 + cjxpv11 + cjypv12; 
		gradient[2*p->i + 1] += cjypv22 + cjypv22 + cjxpv12; 

		result_1 += ci.x * cjxpv11; 
		result_2 += ci.x * cjypv12; 
		result_3 += ci.y * cjypv22; 
	}

	return result_1 + result_2 + result_3; 
}

const C2DBounds& C2DPPDivcurlMatrix::get_size() const
{
	TRACE_FUNCTION; 
	return impl->_M_size; 
}

void C2DPPDivcurlMatrix::reset(const C2DBounds& size, const C2DFVector& range, const CBSplineKernel& kernel, 
			       double wd, double wr)
{
	TRACE_FUNCTION; 
	impl->reset(size, range, kernel, wd, wr); 
}


NS_MIA_END
