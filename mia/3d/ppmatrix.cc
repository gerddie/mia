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
#include <mia/3d/ppmatrix.hh>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif


NS_MIA_BEGIN

class C3DPPDivcurlMatrixImpl {
public: 
	C3DPPDivcurlMatrixImpl(const C3DBounds& size, const C3DFVector& range, 
			       const CBSplineKernel& kernel, double wdiv, double wrot); 

	template <typename Field>
	double multiply(const Field& coefficients) const; 
	double evaluate(const C3DFVectorfield& coefficients, gsl::DoubleVector& gradient) const; 
	double evaluate(const T3DDatafield<C3DDVector>& coefficients, gsl::DoubleVector& gradient) const; 
	void reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
		   double wdiv, double wrot); 
	C3DBounds _M_size; 
private: 
	double _M_wdiv; 
	double _M_wrot; 
	EInterpolation _M_type; 
	C3DFVector _M_range; 
	size_t _M_nodes; 
	
	struct SMatrixCell {
		double v11; 
		double v12; 
		double v13; 
		double v22; 
		double v23; 
		double v33; 
		size_t i; 
		size_t j;
	}; 
	vector<SMatrixCell> _M_P; 
}; 


C3DPPDivcurlMatrix::C3DPPDivcurlMatrix(const C3DBounds& size, const C3DFVector& range, 
				       const CBSplineKernel& kernel, double wdiv, double wrot):
	impl(new C3DPPDivcurlMatrixImpl(size, range, kernel, wdiv, wrot))
{
	TRACE_FUNCTION; 
}
	
C3DPPDivcurlMatrix::~C3DPPDivcurlMatrix()
{
	TRACE_FUNCTION; 
	delete impl; 
}

double C3DPPDivcurlMatrix::operator * (const C3DFVectorfield& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}

double C3DPPDivcurlMatrix::operator * (const T3DDatafield<C3DDVector>& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}
double C3DPPDivcurlMatrix::evaluate(const T3DDatafield<C3DDVector>& coefficients, gsl::DoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	return impl->evaluate(coefficients, gradient); 
}

double C3DPPDivcurlMatrix::evaluate(const C3DFVectorfield& coefficients, gsl::DoubleVector& gradient) const
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


C3DPPDivcurlMatrixImpl::C3DPPDivcurlMatrixImpl(const C3DBounds& size, const C3DFVector& range, 
					       const CBSplineKernel& kernel,
					       double wdiv, double wrot)
{
	reset(size, range, kernel,  wdiv,  wrot); 
}						

void C3DPPDivcurlMatrixImpl::reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
				   double wdiv, double wrot)
{
	if (  _M_size == size && wdiv == _M_wdiv && wrot == _M_wrot && _M_range == range && kernel.get_type()  == _M_type)
		return; 
	_M_size = size;
	_M_wdiv = wdiv; 
	_M_wrot = wrot; 
	_M_range = range; 
	_M_type = kernel.get_type(); 
	_M_nodes = size.x*size.y*size.z; 
	_M_P.clear(); 

	C3DFVector h((size.x-1)/range.x,(size.y-1)/range.y, (size.z-1)/range.z);
	int nz = _M_size.z; 
	int ny = _M_size.y; 
	int nx = _M_size.x; 
	int kernel_range = kernel.size()+1; 

	CIntegralCache rc22(kernel); 
	CIntegralCache rc21(kernel); 
	CIntegralCache rc01(kernel); 
	CIntegralCache rc10(kernel); 

	CIntegralCache rc00(kernel); 
	CIntegralCache rc11(kernel); 

	const double wsum = wdiv + wrot; 
	const double wdelta = 2.0 * (wdiv - wrot); 
	
	for (int a = 0, i=0; a < nz; ++a) {
		for (int l = 0; l < ny; ++l) {
			for (int k = 0; k < nx; ++k, ++i) {
				for (int b = max(0,a - kernel_range); b < min(a + kernel_range, nz); ++b) {
					for (int n = max(0,l - kernel_range); n < min(l + kernel_range, ny); ++n) {
						for (int m = max(0,k - kernel_range); m < min(k + kernel_range,nx); ++m) {
							double r01x = h.x *  rc01.get( k, m, 0, 1,size.x); 
							double r01y = h.y *  rc01.get( l, n, 0, 1,size.y); 
							double r01z = h.z *  rc01.get( a, b, 0, 1,size.z); 

							// i think this was symmetric ...
							double r10x = h.x *  rc10.get( k, m, 0, 1,size.x); 
							double r10y = h.y *  rc10.get( l, n, 0, 1,size.y); 

							double r22x = h.x * rc22.get(k, m, 2, 2, size.x); 
							double r22y = h.y * rc22.get(l, n, 2, 2, size.y); 
							double r22z = h.z * rc22.get(a, b, 2, 2, size.z); 
							
							double r21x = h.x * rc21.get( k, m, 2, 1, size.x); 
							double r21y = h.y * rc21.get( l, n, 2, 1, size.y); 
							double r21z = h.z * rc21.get( a, b, 2, 1, size.z); 
							
							
							double r00x = h.x * rc00.get( k, m, 0, 0, size.x); 
							double r00y = h.y * rc00.get( l, n, 0, 0, size.y); 
							double r00z = h.z * rc00.get( a, b, 0, 0, size.z); 
							
							double r11x = h.x * rc11.get(  k, m, 1, 1, size.x); 
							double r11y = h.y * rc11.get(  l, n, 1, 1, size.y); 
							double r11z = h.z * rc11.get(  a, b, 1, 1, size.z); 
							
							SMatrixCell cell; 
							
							const double r220000 = r22x * r00y * r00z; 
							const double r002200 = r00x * r22y * r00z; 
							const double r000022 = r00x * r00y * r22z; 
							const double r111100 = r11x * r11y * r00z; 
							const double r110011 = r11x * r00y * r11z; 
							const double r001111 = r00x * r11y * r11z; 

							bool zero = true; 
							cell.v11 = 
								r220000 * wdiv +
								r111100 * wsum + 
								r110011 * wsum + 
								r002200 * wrot + 
								r000022 * wrot + 
								r001111 * 2.0 * wrot; 
							zero &= cell.v11 == 0.0; 

							cell.v22 = 
								r220000 * wrot +
								r111100 * wsum + 
								r110011 * 2* wrot + 
								r002200 * wdiv + 
								r000022 * wrot + 
								r001111 * wsum; 
							zero &= cell.v22 == 0.0; 
							
							cell.v33 = 
								r220000 * wrot +
								r111100 * 2* wrot + 
								r110011 * wsum + 
								r002200 * wrot + 
								r000022 * wdiv + 
								r001111 * wsum; 
							zero &= cell.v33 == 0.0; 
							
							if (wdelta != 0.0) {
								cell.v12 = wdelta * 
									(r21x * r01y * r00z + 
									 r01x * r21y * r00z + 
									 r10x * r01y * r11z); 
								zero &= cell.v12 == 0.0; 

								cell.v13 = wdelta * 
									(r21x * r00y * r01z + 
									 r10x * r11y * r01z + 
									 r01x * r00y * r21z); 
								zero &= cell.v13 == 0.0; 

								cell.v23 = wdelta * ( 
									r11x * r10y * r01z + 
									r00x * r21y * r01z + 
									r00x * r01y * r21z); 
								zero &= cell.v23 == 0.0; 
							}
								
							if (!zero) 
								_M_P.push_back(cell); 
						}
					}
				}
			}
		}
	}
}

template <typename Field>
double C3DPPDivcurlMatrixImpl::multiply(const Field& coefficients) const
{
	assert(coefficients.size() == _M_nodes); 

	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	double result_4 = 0.0; 
	double result_5 = 0.0; 
	double result_6 = 0.0; 

	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		result_1 += ci.x * cj.x * p->v11; 
		result_2 += ci.x * cj.y * p->v12; 
		result_3 += ci.y * cj.y * p->v22; 
		result_4 += ci.x * cj.z * p->v13; 
		result_5 += ci.y * cj.z * p->v23; 
		result_6 += ci.z * cj.z * p->v33; 

	}
	return result_1 + result_2 + result_3 + 
		result_4 + result_5 + result_6; 
}


double C3DPPDivcurlMatrixImpl::evaluate(const T3DDatafield<C3DDVector>& coefficients, 
					gsl::DoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 3); 
	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	double result_4 = 0.0; 
	double result_5 = 0.0; 
	double result_6 = 0.0; 

	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		const double cjxpv11 = cj.x * p->v11; 
		const double cjypv22 = cj.y * p->v22; 
		const double cjzpv33 = cj.z * p->v33; 

		const double cjypv12 = cj.y * p->v12; 
		const double cjxpv13 = cj.x * p->v13; 
		const double cjzpv23 = cj.z * p->v23; 

		result_1 += ci.x * cjxpv11; 
		result_2 += ci.x * cjypv12; 
		result_3 += ci.y * cjypv22; 
		result_4 += ci.z * cjxpv13; 
		result_5 += ci.y * cjzpv23; 
		result_6 += ci.z * cjzpv33; 

		gradient[3*p->i    ] += 2 * cjxpv11 + cjypv12 + cj.z * p->v13; 
		gradient[3*p->i + 1] += 2 * cjypv22 + cjzpv23 + cj.x * p->v12; 
		gradient[3*p->i + 2] += 2 * cjzpv33 + cjxpv13 + cj.y * p->v23; 

	}

	return result_1 + result_2 + result_3 + 
		result_4 + result_5 + result_6; 
}

double C3DPPDivcurlMatrixImpl::evaluate(const C3DFVectorfield& coefficients, 
					gsl::DoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 3); 
	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	double result_4 = 0.0; 
	double result_5 = 0.0; 
	double result_6 = 0.0; 

	for (auto p = _M_P.begin(); p != _M_P.end();++p) {
		auto ci = coefficients[p->i]; 
		auto cj = coefficients[p->j]; 
		
		const double cjxpv11 = cj.x * p->v11; 
		const double cjypv22 = cj.y * p->v22; 
		const double cjzpv33 = cj.z * p->v33; 

		const double cjypv12 = cj.y * p->v12; 
		const double cjxpv13 = cj.x * p->v13; 
		const double cjzpv23 = cj.z * p->v23; 

		result_1 += ci.x * cjxpv11; 
		result_2 += ci.x * cjypv12; 
		result_3 += ci.y * cjypv22; 
		result_4 += ci.z * cjxpv13; 
		result_5 += ci.y * cjzpv23; 
		result_6 += ci.z * cjzpv33; 

		gradient[3*p->i    ] += 2 * cjxpv11 + cjypv12 + cj.z * p->v13; 
		gradient[3*p->i + 1] += 2 * cjypv22 + cjzpv23 + cj.x * p->v12; 
		gradient[3*p->i + 2] += 2 * cjzpv33 + cjxpv13 + cj.y * p->v23; 

	}

	return result_1 + result_2 + result_3 + 
		result_4 + result_5 + result_6; 

}

const C3DBounds& C3DPPDivcurlMatrix::get_size() const
{
	TRACE_FUNCTION; 
	return impl->_M_size; 
}

void C3DPPDivcurlMatrix::reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
			       double wdiv, double wrot)
{
	TRACE_FUNCTION; 
	impl->reset(size, range, kernel, wdiv, wrot); 
}


NS_MIA_END
