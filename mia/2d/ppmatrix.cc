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

NS_MIA_BEGIN

class C2DPPDivcurlMatrixImpl {
public: 
	C2DPPDivcurlMatrixImpl(const C2DBounds& size, const C2DFVector& range, 
			       const CBSplineKernel& kernel, double wd, double wr); 

	template <typename Field>
	double multiply(const Field& coefficients) const; 
	C2DBounds _M_size; 
	double evaluate(const T2DDatafield<C2DDVector>& coefficients, gsl::DoubleVector& gradient) const; 
private: 

	size_t _M_nodes; 
	struct SMatrixCell {
		double v11; 
		double v12; 
		double v22; 
		size_t i; 
		size_t j; 
	}; 
	vector<SMatrixCell> _M_P; 
}; 


C2DPPDivcurlMatrix::C2DPPDivcurlMatrix(const C2DBounds& size, const C2DFVector& range, 
				       const CBSplineKernel& kernel, double wd, double wr):
	impl(new C2DPPDivcurlMatrixImpl(size, range, kernel, wd, wr))
{
}
	
C2DPPDivcurlMatrix::~C2DPPDivcurlMatrix()
{
	delete impl; 
}

double C2DPPDivcurlMatrix::operator * (const C2DFVectorfield& coefficients) const
{
	return impl->multiply(coefficients); 
}

double C2DPPDivcurlMatrix::operator * (const T2DDatafield<C2DDVector>& coefficients) const
{
	return impl->multiply(coefficients); 
}
double C2DPPDivcurlMatrix::evaluate(const T2DDatafield<C2DDVector>& coefficients, gsl::DoubleVector& gradient) const
{
	return impl->evaluate(coefficients, gradient); 
}


/** helper class to evaluate value sonly once, should be re-done */
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
					       double wd, double wr):
	_M_size(size),
	_M_nodes(size.x*size.y)
{
	C2DFVector h((size.x-1)/range.x,(size.y-1)/range.y);
	cvinfo() << "size=" << size 
		 << " range=" << range
		 << " h=" << h << "\n"; 
	int ny = _M_size.y; 
	int nx = _M_size.x; 
	int kernel_range = kernel.size()+1; 
	
	CIntegralCache rc22(kernel); 
	CIntegralCache rc21(kernel); 
	CIntegralCache rc01(kernel); 
	CIntegralCache rc00(kernel); 
	CIntegralCache rc11(kernel); 

	
	for (int l = 0, i=0; l < ny; ++l) {
		for (int k = 0; k < nx; ++k, ++i) {
			for (int n = max(0,l - kernel_range); n < min(l + kernel_range, ny); ++n) {
				for (int m = max(0,k - kernel_range); m < min(k + kernel_range,nx); ++m) {
					double r01x =  rc01.get( k, m, 0, 1,size.x); 
					double r01y =  rc01.get( l, n, 0, 1,size.y); 
					
					double r22x = h.x * rc22.get(k, m, 2, 2, size.x); 
					double r22y = h.y * rc22.get(l, n, 2, 2, size.y); 
					
					double r21x = h.x * rc21.get( k, m, 2, 1, size.x); 
					double r21y = h.y * rc21.get( l, n, 2, 1, size.y); 
					
					
					double r00x = h.x * rc00.get( k, m, 0, 0, size.x); 
					double r00y = h.y * rc00.get( l, n, 0, 0, size.y); 
					
					double r11x = h.x * rc11.get(  k, m, 1, 1, size.x); 
					double r11y = h.y * rc11.get(  l, n, 1, 1, size.y); 
					
					SMatrixCell cell; 
					cell.v11 = 
					wd * (r22x * r00y + r11x * r11y) + 
					wr * (r11x * r11y + r00x * r22y); 
					
					cell.v12 = 2 *  (
						wd * ( r21x * r01y + r01x * r21y) -
						wr * ( r01x * r21y + r21x * r01y )); 
					
					cell.v22 = 
					wd * (r00x * r22y + r11x * r11y) + 
					wr * (r22x * r00y + r11x * r11y);
					cell.i = i; 
					cell.j = m + n * nx;  
					if (cell.v11 != 0.0 || cell.v12 != 0.0 ||cell.v22 != 0.0) 
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
		
		result_1 += ci.x * cj.x * p->v11; 
		result_2 += ci.x * cj.y * p->v12; 
		result_3 += ci.y * cj.y * p->v22; 
	}

	return result_1 + result_2 + result_3; 
}

double C2DPPDivcurlMatrixImpl::evaluate(const T2DDatafield<C2DDVector>& coefficients, 
					gsl::DoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 2); 

	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 

		gradient[2*p->i    ] +=                 2 * cj.x * p->v11 + cj.y * p->v12; 
		gradient[2*p->i + 1] += cj.x * p->v12 + 2 * cj.y * p->v22; 

		

		result_1 += ci.x * cj.x * p->v11; 
		result_2 += ci.x * cj.y * p->v12; 
		result_3 += ci.y * cj.y * p->v22; 
	}

	return result_1 + result_2 + result_3; 
}

const C2DBounds& C2DPPDivcurlMatrix::get_size() const
{
	return impl->_M_size; 
}

NS_MIA_END
