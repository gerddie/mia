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
		double vxx; 
		double vxy; 
		double vxz; 
		double vyy; 
		double vyz; 
		double vzz; 
		size_t i; 
		size_t j;
		double sort_help; 
		SMatrixCell(); 
	}; 
	vector<SMatrixCell> _M_P; 
}; 


C3DPPDivcurlMatrixImpl::SMatrixCell::SMatrixCell()
{
	memset(this, 0, sizeof(*this)); 
}


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
	void print_index_range()const;
private: 
	const CBSplineKernel& _M_kernel; 
	int _M_max_skip; 
	int _M_row_length; 
	int _M_hr; 
	int _M_shift; 
	mutable vector<int> _M_index_map; 
	mutable vector<double> _M_values; 
}; 

CIntegralCache::CIntegralCache(const CBSplineKernel& kernel):
	_M_kernel(kernel), 
	_M_max_skip((kernel.size() + 1) & ~1), 
	_M_row_length(kernel.size()), 
	_M_hr(kernel.get_active_halfrange()), 
	_M_shift(kernel.size() * kernel.size()), 
	_M_index_map(2*_M_shift, -1)

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
	if ( delta >= _M_row_length ) {
		return 0.0;
	}
	
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
	
	int index = skip * 2*_M_row_length + delta + _M_row_length + _M_shift;
	int ref = _M_index_map[index]; 
	if (ref >= 0) 
		return _M_values[ref]; 
	else {
		double result = integrate2(_M_kernel, s1, s2, deg1, deg2, 1, 0,  range); 
		_M_index_map[index] = _M_values.size(); 
		_M_values.push_back(result); 
		return result; 
	}
	
}

void CIntegralCache::print_index_range()const
{
	//cvmsg() << "CIntegralCache index range: [" << min_index << ", " << max_index << "\n"; 
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
	if (  _M_size == size && wdiv == _M_wdiv && wrot == _M_wrot &&
	      _M_range == range && kernel.get_type()  == _M_type)
		return; 
	
	_M_size = size;
	_M_wdiv = wdiv;
	_M_wrot = wrot;
	_M_range = range;
	_M_type = kernel.get_type();
	_M_nodes = size.x*size.y*size.z;
	_M_P.clear();

	C3DFVector h((size.x-1)/range.x,
		     (size.y-1)/range.y, 
		     (size.z-1)/range.z);
	const double scale = pow(h.x * h.y * h.z, 1.0/3.0); 

	cvinfo() << "scale = " << scale << "\n"; 
	const int nx = _M_size.x;
	const int ny = _M_size.y;
	const int nz = _M_size.z;
	const size_t nxy = _M_size.x *_M_size.y; 

	int ksize = kernel.size(); 
	cvinfo() << "ksize = " <<ksize  << "\n"; 

	CIntegralCache rc00(kernel); 
	CIntegralCache rc11(kernel); 
	CIntegralCache rc22(kernel); 
	CIntegralCache rc21(kernel); 
	CIntegralCache rc12(kernel); 
	CIntegralCache rc01(kernel); 
	CIntegralCache rc10(kernel); 

	const double wsum = wdiv + wrot; 
	const double wdelta = 2*(wdiv - wrot); 
	
	for (int zi = 0; zi < nz; ++zi) {
		for (int zj = max(0,zi - ksize); zj < min(zi + ksize, nz); ++zj) {
			const double r00z = rc00.get( zi, zj, 0, 0, size.z); 
			const double r01z = rc01.get( zi, zj, 0, 1, size.z); 
			const double r11z = rc11.get( zi, zj, 1, 1, size.z); 
			const double r12z = rc12.get( zi, zj, 1, 2, size.z); 
			const double r22z = rc22.get( zi, zj, 2, 2, size.z); 


			for (int yi = 0; yi < ny; ++yi) {
				for (int yj = max(0,yi - ksize); yj < min(yi + ksize, ny); ++yj) {
					const double r00y = rc00.get( yi, yj, 0, 0, size.y); 
					const double r01y = rc01.get( yi, yj, 0, 1, size.y); 
					const double r10y = rc10.get( yi, yj, 1, 0, size.y); 
					const double r11y = rc11.get( yi, yj, 1, 1, size.y); 
					const double r12y = rc12.get( yi, yj, 1, 2, size.y); 
					const double r21y = rc21.get( yi, yj, 2, 1, size.y); 
					const double r22y = rc22.get( yi, yj, 2, 2, size.y); 

					
					for (int xi = 0; xi < nx; ++xi) {
						for (int xj = max(0,xi - ksize); xj < min(xi + ksize,nx); ++xj) {
							const double r00x = rc00.get( xi, xj, 0, 0, size.x); 
							const double r01x = rc01.get( xi, xj, 0, 1, size.x); 
							const double r10x = rc10.get( xi, xj, 1, 0, size.x); 
							const double r11x = rc11.get( xi, xj, 1, 1, size.x); 
							const double r21x = rc21.get( xi, xj, 2, 1, size.x); 
							const double r22x = rc22.get( xi, xj, 2, 2, size.x); 




							SMatrixCell cell; 
							
							const double r200200 = r22x * r00y * r00z; 
							const double r020020 = r00x * r22y * r00z; 
							const double r002002 = r00x * r00y * r22z; 
							const double r110110 = r11x * r11y * r00z; 
							const double r101101 = r11x * r00y * r11z; 
							const double r011011 = r00x * r11y * r11z; 

							bool zero = true; 
							cell.vxx = scale * (
								r002002 * wrot + 
								r011011 * 2.0 * wrot + 
								r020020 * wrot + 
								r101101 * wsum + 
								r110110 * wsum + 
								r200200 * wdiv); 

							zero &= cell.vxx == 0.0; 

							cell.vyy = scale * (
								r002002 * wrot + 
								r011011 * wsum + 
								r020020 * wdiv + 
								r101101 * 2* wrot + 
								r110110 * wsum + 
								r200200 * wrot); 
							zero &= cell.vyy == 0.0; 
							
							cell.vzz = scale * (
								r002002 * wdiv + 
								r011011 * wsum + 
								r020020 * wrot +
								r101101 * wsum +  
								r110110 * wrot * 2 + 
								r200200 * wrot); 

							zero &= cell.vzz == 0.0; 
							
							if (wdelta != 0.0) {
								cell.vxy = scale * wdelta * 
									(r21x * r01y * r00z + 
									 r10x * r12y * r00z + 
									 r01x * r10y * r11z); 
								zero &= cell.vxy == 0.0; 

								cell.vxz = scale * wdelta * 
									(r21x * r00y * r01z + 
									 r10x * r11y * r01z + 
									 r10x * r00y * r12z); 
								zero &= cell.vxz == 0.0;

								cell.vyz = scale * wdelta * (
									r11x * r10y * r01z + 
									r00x * r21y * r01z + 
									r00x * r10y * r12z);
								zero &= cell.vyz == 0.0;

							}
							
								
							if (!zero) {
								
								cell.i = xi + nx * yi + nxy * zi;
								cell.j = xj + nx * yj + nxy * zj;
								_M_P.push_back(cell);
								cvdebug() <<"cell = [" 
									  << cell.i << "x" << cell.j 
									  << "] (" << cell.vxx
									  << ", " << cell.vyy
									  << ", " << cell.vzz
									  << ", " << cell.vxy
									  << ", " << cell.vxz
									  << ", " << cell.vyz
									  << "\n"; 

							}
						}
					}
				}
			}
		}
	}
	cvmsg() << "P-matrix has " << _M_P.size() << " entries\n"; 
	cvmsg() << "size:" << _M_size << ", ksize=" << ksize << "\n"; 
	rc00.print_index_range(); 
	rc11.print_index_range(); 
	rc22.print_index_range(); 
	rc21.print_index_range();  
	rc12.print_index_range(); 
	rc01.print_index_range(); 
	rc10.print_index_range(); 
}

template <typename Field>
double C3DPPDivcurlMatrixImpl::multiply(const Field& coefficients) const
{
	assert(coefficients.size() == _M_nodes); 

	register double result_1 = 0.0; 
	register double result_2 = 0.0; 
	register double result_3 = 0.0; 
	register double result_4 = 0.0; 
	register double result_5 = 0.0; 
	register double result_6 = 0.0; 

	if (_M_wdiv == _M_wrot) {
		for (auto p = _M_P.begin(); p != _M_P.end();++p) {
			auto ci = coefficients[p->i]; 
			auto cj = coefficients[p->j]; 
			result_1 += ci.x * cj.x * p->vxx; 
			result_2 += ci.y * cj.y * p->vyy; 
			result_3 += ci.z * cj.z * p->vzz; 
		}
		return result_1 + result_2 + result_3; 
	}else{
		for (auto p = _M_P.begin(); p != _M_P.end();++p) {
			auto ci = coefficients[p->i]; 
			auto cj = coefficients[p->j]; 
			
			result_1 += ci.x * cj.x * p->vxx; 
			result_2 += ci.y * cj.y * p->vyy; 
			result_3 += ci.z * cj.z * p->vzz; 
			
			result_4 += ci.x * cj.y * p->vxy; 
			result_5 += ci.x * cj.z * p->vxz; 
			result_6 += ci.y * cj.z * p->vyz; 
			
		}
		return result_1 + result_2 + result_3 + 
			result_4 + result_5 + result_6; 
	}
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
		
		const double cjxpvxx = cj.x * p->vxx; 
		const double cjypvyy = cj.y * p->vyy; 
		const double cjzpvzz = cj.z * p->vzz; 

		const double cjypvxy = cj.y * p->vxy; 
		const double cjxpvxz = cj.x * p->vxz; 
		const double cjzpvyz = cj.z * p->vyz; 

		result_1 += ci.x * cjxpvxx; 
		result_2 += ci.x * cjypvxy; 
		result_3 += ci.y * cjypvyy; 
		result_4 += ci.z * cjxpvxz; 
		result_5 += ci.y * cjzpvyz; 
		result_6 += ci.z * cjzpvzz; 

		gradient[3*p->i    ] += 2 * cjxpvxx + cjypvxy + cj.z * p->vxz; 
		gradient[3*p->i + 1] += 2 * cjypvyy + cjzpvyz + cj.x * p->vxy; 
		gradient[3*p->i + 2] += 2 * cjzpvzz + cjxpvxz + cj.y * p->vyz; 

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
		
		const double cjxpvxx = cj.x * p->vxx; 
		const double cjypvyy = cj.y * p->vyy; 
		const double cjzpvzz = cj.z * p->vzz; 

		const double cjypvxy = cj.y * p->vxy; 
		const double cjxpvxz = cj.x * p->vxz; 
		const double cjzpvyz = cj.z * p->vyz; 

		result_1 += ci.x * cjxpvxx; 
		result_2 += ci.x * cjypvxy; 
		result_3 += ci.y * cjypvyy; 
		result_4 += ci.z * cjxpvxz; 
		result_5 += ci.y * cjzpvyz; 
		result_6 += ci.z * cjzpvzz; 

		gradient[3*p->i    ] += 2 * cjxpvxx + cjypvxy + cj.z * p->vxz; 
		gradient[3*p->i + 1] += 2 * cjypvyy + cjzpvyz + cj.x * p->vxy; 
		gradient[3*p->i + 2] += 2 * cjzpvzz + cjxpvxz + cj.y * p->vyz; 

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
