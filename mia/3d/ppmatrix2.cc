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

#define VSTREAM_DOMAIN "ppmatrix-3d" 

#include <cmath>
#include <mia/3d/ppmatrix2.hh>
#include <mia/3d/3DDatafield.cxx>

#include <emmintrin.h>


NS_MIA_BEGIN

class C3DPPDivcurlMatrix2Impl {
public: 
	C3DPPDivcurlMatrix2Impl(const C3DBounds& size, const C3DFVector& range, 
			       const CBSplineKernel& kernel, double wdiv, double wrot); 

	template <typename Field>
	double multiply(const Field& coefficients) const; 
	double evaluate(const C3DFVectorfield& coefficients, CDoubleVector& gradient) const; 
	double evaluate(const T3DDatafield<C3DDVector>& coefficients, CDoubleVector& gradient) const; 
	void reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
		   double wdiv, double wrot); 
	C3DBounds _M_size; 
private: 
	double _M_wdiv; 
	double _M_wrot; 
	EInterpolation _M_type; 
	C3DFVector _M_range; 
	size_t _M_nodes; 
	int _M_ksize; 
	
	struct SMatrixCell {
		double vxx; 
		double vxz; 

		double vxy; 
		double vyy; 

		double vyz; 
		double vzz; 
		SMatrixCell(); 
	}; 
	T3DDatafield<SMatrixCell> _M_P; 
}; 


C3DPPDivcurlMatrix2Impl::SMatrixCell::SMatrixCell()
{
	memset(this, 0, sizeof(*this)); 
}


C3DPPDivcurlMatrix2::C3DPPDivcurlMatrix2(const C3DBounds& size, const C3DFVector& range, 
				       const CBSplineKernel& kernel, double wdiv, double wrot):
	impl(new C3DPPDivcurlMatrix2Impl(size, range, kernel, wdiv, wrot))
{
	TRACE_FUNCTION; 
}
	
C3DPPDivcurlMatrix2::~C3DPPDivcurlMatrix2()
{
	TRACE_FUNCTION; 
	delete impl; 
}

double C3DPPDivcurlMatrix2::operator * (const C3DFVectorfield& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}

double C3DPPDivcurlMatrix2::operator * (const T3DDatafield<C3DDVector>& coefficients) const
{
	TRACE_FUNCTION; 
	return impl->multiply(coefficients); 
}
double C3DPPDivcurlMatrix2::evaluate(const T3DDatafield<C3DDVector>& coefficients, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	return impl->evaluate(coefficients, gradient); 
}

double C3DPPDivcurlMatrix2::evaluate(const C3DFVectorfield& coefficients, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	return impl->evaluate(coefficients, gradient); 
}


/**\todo helper class to evaluate values only once, should be re-done and moved to the spline kernel */
class CIntegralCache2 {
public: 
	CIntegralCache2(const CBSplineKernel& kernel, int deg1, int deg2); 
	double get(int delta) const; 
private: 
	const CBSplineKernel& m_kernel; 
	int m_shift; 
	int m_deg1; 
	int m_deg2; 
	mutable vector<bool> m_valid; 
	mutable vector<double> m_values; 
}; 

CIntegralCache2::CIntegralCache2(const CBSplineKernel& kernel, int deg1, int deg2):
	m_kernel(kernel), 
	m_shift(kernel.size()),
	m_deg1(deg1), 
	m_deg2(deg2), 
	m_valid(2*m_shift + 1), 
	m_values(2*m_shift + 1)
	
{
	fill(m_valid.begin(), m_valid.end(), false); 
}

double CIntegralCache2::get(int delta) const
{
	delta +=  m_shift; 
	if (delta < 0 || delta >= (int)m_valid.size()) 
		return 0; 

	double result = 0; 
	if (m_valid[delta]) 
		result = m_values[delta]; 
	else {
		double result = integrate2(m_kernel, m_shift, delta, m_deg1, m_deg2, 1, 0,  2*m_shift); 
		m_values[delta] = result; 
		m_valid[delta] = true; 
	}
	return result; 
}


C3DPPDivcurlMatrix2Impl::C3DPPDivcurlMatrix2Impl(const C3DBounds& size, const C3DFVector& range, 
					       const CBSplineKernel& kernel,
					       double wdiv, double wrot)
{
	reset(size, range, kernel,  wdiv,  wrot); 
}						

void C3DPPDivcurlMatrix2Impl::reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
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
	_M_P = T3DDatafield<SMatrixCell>(C3DBounds(2 * kernel.size()+1, 
						   2 * kernel.size()+1, 
						   2 * kernel.size()+1)); 

	C3DFVector h1((size.x-1)/range.x,
		      (size.y-1)/range.y, 
		      (size.z-1)/range.z);
	
	C3DFVector h2(h1.x * h1.x, h1.y * h1.y, h1.z * h1.z); 
	C3DFVector h3(h1.x * h2.x, h1.y * h2.y, h1.z * h2.z); 
	C3DFVector h4(h1.x * h3.x, h1.y * h3.y, h1.z * h3.z); 

	const double global_scale = 1.0 / (h1.x * h1.y * h1.z); 

	cvdebug() << "scale = " << global_scale << "\n"; 
	cvdebug() << "h1 = " << h1 << "\n"; 
	cvdebug() << "h2 = " << h2 << "\n"; 
	cvdebug() << "h3 = " << h3 << "\n"; 
	cvdebug() << "h4 = " << h4 << "\n"; 

	
	_M_ksize = kernel.size(); 
	cvinfo() << "ksize = " <<_M_ksize  << "\n"; 

	CIntegralCache2 rc00(kernel, 0, 0); 
	CIntegralCache2 rc11(kernel, 1, 1); 
	CIntegralCache2 rc22(kernel, 2, 2); 
	CIntegralCache2 rc21(kernel, 2, 1); 
	CIntegralCache2 rc12(kernel, 1, 2); 
	CIntegralCache2 rc01(kernel, 0, 1); 
	CIntegralCache2 rc10(kernel, 1, 0); 

	const double wsum = wdiv + wrot; 
	const double wdelta = 2*(wdiv - wrot); 

	auto p = _M_P.begin(); 
	for (int z = -_M_ksize; z <= _M_ksize; ++z)  {
		const double r00z =        rc00.get(z); 
		const double r01z = h1.z * rc01.get(z); 
		const double r11z = h2.z * rc11.get(z); 
		const double r12z = h3.z * rc12.get(z); 
		const double r22z = h4.z * rc22.get(z); 
		for (int y = -_M_ksize; y <= _M_ksize; ++y)  {
			const double r00y =        rc00.get(y); 
			const double r01y = h1.y * rc01.get(y); 
			const double r10y = h1.y * rc10.get(y); 
			const double r11y = h2.y * rc11.get(y); 
			const double r12y = h3.y * rc12.get(y); 
			const double r21y = h3.y * rc21.get(y); 
			const double r22y = h4.y * rc22.get(y); 
			for (int x = -_M_ksize; x <= _M_ksize; ++x, ++p) {
				const double r00x =        rc00.get(x); 
				const double r01x = h1.x * rc01.get(x); 
				const double r10x = h1.x * rc10.get(x); 
				const double r11x = h2.x * rc11.get(x); 
				const double r21x = h3.x * rc21.get(x); 
				const double r22x = h4.x * rc22.get(x); 
				
				SMatrixCell cell; 
				
				const double r200200 = r22x * r00y * r00z; 
				const double r020020 = r00x * r22y * r00z; 
				const double r002002 = r00x * r00y * r22z; 
				const double r110110 = r11x * r11y * r00z; 
				const double r101101 = r11x * r00y * r11z; 
				const double r011011 = r00x * r11y * r11z; 
				
				bool zero = true; 
				cell.vxx = global_scale * (
					r002002 * wrot + 
					r011011 * 2.0 * wrot + 
					r020020 * wrot + 
					r101101 * wsum + 
					r110110 * wsum + 
					r200200 * wdiv); 
				
				zero &= cell.vxx == 0.0; 
				
				cell.vyy = global_scale * (
					r002002 * wrot + 
					r011011 * wsum + 
					r020020 * wdiv + 
					r101101 * 2* wrot + 
					r110110 * wsum + 
					r200200 * wrot); 
				zero &= cell.vyy == 0.0; 
				
				cell.vzz = global_scale * (
					r002002 * wdiv + 
					r011011 * wsum + 
					r020020 * wrot +
					r101101 * wsum +  
					r110110 * wrot * 2 + 
					r200200 * wrot); 
				
				zero &= cell.vzz == 0.0; 
				
				if (wdelta != 0.0) {
					cell.vxy = global_scale * wdelta * 
						(r21x * r01y * r00z + 
						 r10x * r12y * r00z + 
						 r01x * r10y * r11z); 
					zero &= cell.vxy == 0.0; 
					
					cell.vxz = global_scale * wdelta * 
						(r21x * r00y * r01z + 
						 r10x * r11y * r01z + 
						 r10x * r00y * r12z); 
					zero &= cell.vxz == 0.0;
					
					cell.vyz = global_scale * wdelta * (
						r11x * r10y * r01z + 
						r00x * r21y * r01z + 
						r00x * r10y * r12z);
					zero &= cell.vyz == 0.0;
					
				}
				*p = cell; 
			}
		}
	}
	cvdebug() << "P-matrix has " << _M_P.size() << " entries\n"; 
	cvdebug() << "size:" << _M_size << ", _M_ksize=" << _M_ksize << "\n"; 
}

template <typename Field>
double C3DPPDivcurlMatrix2Impl::multiply(const Field& coefficients) const
{
	assert(coefficients.size() == _M_nodes); 

	register double result_1 = 0.0; 
	register double result_2 = 0.0; 
	register double result_3 = 0.0; 
	register double result_4 = 0.0; 
	register double result_5 = 0.0; 
	register double result_6 = 0.0; 

	const int nx = _M_size.x;
	const int ny = _M_size.y;
	const int nz = _M_size.z;

	auto ci = coefficients.begin(); 

	if (_M_wdiv == _M_wrot) {
		for (int zi = 0; zi < nz; ++zi) {
			for (int yi = 0; yi < ny; ++yi) {
				for (int xi = 0; xi < nx; ++xi, ++ci) {
					for (int zj = max(0,zi - _M_ksize); zj < min(zi + _M_ksize, nz); ++zj) {
						int dz = zi - zj + _M_ksize; 
						for (int yj = max(0,yi - _M_ksize); yj < min(yi + _M_ksize, ny); ++yj) {							int dy = yi - yj + _M_ksize;
							int xstart = max(0,xi - _M_ksize); 
							auto cj = coefficients.begin_at(xstart, yj, zj); 
							auto p = _M_P.begin_at(xi - xstart + _M_ksize, dy, dz);
							for (int xj = xstart; xj < min(xi + _M_ksize,nx); --p, ++xj, ++cj) {
								result_1 += ci->x * cj->x * p->vxx; 
								result_2 += ci->y * cj->y * p->vyy; 
								result_3 += ci->z * cj->z * p->vzz; 
							}
						}
					}
				}
			}
		}
	}else{
		for (int zi = 0; zi < nz; ++zi) {
			for (int yi = 0; yi < ny; ++yi) {
				for (int xi = 0; xi < nx; ++xi, ++ci) {
					for (int zj = max(0,zi - _M_ksize); zj < min(zi + _M_ksize, nz); ++zj) {
						const int dz = zi - zj + _M_ksize; 
						for (int yj = max(0,yi - _M_ksize); yj < min(yi + _M_ksize, ny); ++yj) {
							const int dy = yi - yj + _M_ksize;
							const int xstart = max(0,xi - _M_ksize);
							const int xend = min(xi + _M_ksize,nx); 
							auto cj = coefficients.begin_at(xstart, yj, zj); 
							
							auto p = _M_P.begin_at(xi - xstart + _M_ksize, dy, dz);

							for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {

								
								result_1 += ci->x * cj->x * p->vxx; 
								result_2 += ci->y * cj->y * p->vyy; 
								result_3 += ci->z * cj->z * p->vzz; 
								
								result_4 += ci->x * cj->y * p->vxy; 
								result_5 += ci->x * cj->z * p->vxz; 
								result_6 += ci->y * cj->z * p->vyz;
							}
						}
					}
				}
			}
		}
		
	}
	return result_1 + result_2 + result_3 + 
		result_4 + result_5 + result_6; 
}

double C3DPPDivcurlMatrix2Impl::evaluate(const T3DDatafield<C3DDVector>& coefficients, 
					CDoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 3); 

	const int nx = _M_size.x;
	const int ny = _M_size.y;
	const int nz = _M_size.z;

	double result_1 = 0.0; 
	double result_2 = 0.0; 
	double result_3 = 0.0; 
	double result_4 = 0.0; 
	double result_5 = 0.0; 
	double result_6 = 0.0; 

	auto ci = coefficients.begin(); 	
	auto gi = gradient.begin(); 
	int i = 0; 
	for (int zi = 0; zi < nz; ++zi) {
		for (int yi = 0; yi < ny; ++yi) {
			for (int xi = 0; xi < nx; ++xi, ++ci, ++i, gi+=3) {
				C3DDVector g(0,0,0); 
				for (int zj = max(0,zi - _M_ksize); zj < min(zi + _M_ksize, nz); ++zj) {
					const int dz = zi - zj + _M_ksize; 
					for (int yj = max(0,yi - _M_ksize); yj < min(yi + _M_ksize, ny); ++yj) {
						const int dy = yi - yj + _M_ksize;
						const int xstart = max(0,xi - _M_ksize);
						const int xend = min(xi + _M_ksize,nx);
						auto p = _M_P.begin_at(xi - xstart + _M_ksize, dy, dz); 
						auto cj = coefficients.begin_at(xstart, yj, zj); 
						for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {
							
							const double cjxpvxx = cj->x * p->vxx; 
							const double cjypvyy = cj->y * p->vyy; 
							const double cjzpvzz = cj->z * p->vzz; 
							
							const double cjypvxy = cj->y * p->vxy; 
							const double cjxpvxz = cj->x * p->vxz; 
							const double cjzpvyz = cj->z * p->vyz; 
							
							result_1 += ci->x * cjxpvxx; 
							result_2 += ci->x * cjypvxy; 
							result_3 += ci->y * cjypvyy; 
							result_4 += ci->z * cjxpvxz; 
							result_5 += ci->y * cjzpvyz; 
							result_6 += ci->z * cjzpvzz; 
							
							g.x += 2 * cjxpvxx + cjypvxy + cj->z * p->vxz; 
							g.y += 2 * cjypvyy + cjzpvyz + cj->x * p->vxy; 
							g.z += 2 * cjzpvzz + cjxpvxz + cj->y * p->vyz; 
						}
					}
				}
				gi[0] = g.x; 
				gi[1] = g.y; 
				gi[2] = g.z; 
			}
		}
	}
	return result_1 + result_2 + result_3 + 
		result_4 + result_5 + result_6; 
}

double C3DPPDivcurlMatrix2Impl::evaluate(const C3DFVectorfield& coefficients, 
					CDoubleVector& gradient) const
{
	assert(coefficients.size() == _M_nodes); 
	assert(gradient.size() == coefficients.size() * 3); 

	const int nx = _M_size.x;
	const int ny = _M_size.y;
	const int nz = _M_size.z;

	double __attribute__((aligned(16))) result[6]; 
	fill(result, result + 6, 0.0);
	int i = 0; 
	auto ci = coefficients.begin(); 	
	auto gi = gradient.begin(); 
	for (int zi = 0; zi < nz; ++zi) {
		for (int yi = 0; yi < ny; ++yi) {
			for (int xi = 0; xi < nx; ++xi, ++ci, ++i, gi+=3) {
				
				double __attribute__((aligned(16))) g[4]; 
				fill(g, g + 4, 0.0);
				for (int zj = max(0,zi - _M_ksize); zj < min(zi + _M_ksize, nz); ++zj) {
					const int dz = zi - zj + _M_ksize; 
					for (int yj = max(0,yi - _M_ksize); yj < min(yi + _M_ksize, ny); ++yj) {
						const int dy = yi - yj + _M_ksize;
						const int xstart = max(0,xi - _M_ksize);
						const int xend = min(xi + _M_ksize,nx);

						auto p = _M_P.begin_at(xi - xstart + _M_ksize, dy, dz);
						auto cj = coefficients.begin_at(xstart, yj, zj); 

						for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {
							const __m128d cj_x = _mm_set1_pd(cj->x); 
							const __m128d cj_y = _mm_set1_pd(cj->y); 
							const __m128d cj_z = _mm_set1_pd(cj->z); 
							const __m128d cj_zx = _mm_shuffle_pd(cj_z, cj_x, 
											     _MM_SHUFFLE2(0,0));
							
							const __m128d p_xx_xz = _mm_load_pd(&p->vxx); 
							const __m128d p_xy_yy = _mm_load_pd(&p->vxy); 
							const __m128d p_yz_zz = _mm_load_pd(&p->vyz); 
							const __m128d p_xz_xy = _mm_shuffle_pd(p_xx_xz, p_xy_yy, 
											   _MM_SHUFFLE2(0,1));
							
							__m128d gz01 = cj_zx * p_xz_xy;
							__m128d gz2X = _mm_mul_sd(cj_y, p_yz_zz);
							 
							const __m128d cj_xp_xx_xz = p_xx_xz * cj_x; 
							const __m128d cj_yp_xy_yy = p_xy_yy * cj_y; 
							const __m128d cj_zp_yz_zz = p_yz_zz * cj_z; 
							
							const __m128d cj_xyp_xx_xy = _mm_shuffle_pd(cj_xp_xx_xz, 
											     cj_yp_xy_yy, 
											     _MM_SHUFFLE2(0,0)); 
							
							const __m128d cj_yzp_yy_yz = _mm_shuffle_pd(cj_yp_xy_yy, 
											     cj_zp_yz_zz, 
											     _MM_SHUFFLE2(0,1)); 
							
							const __m128d cj_zxp_zz_xz = _mm_shuffle_pd(cj_zp_yz_zz, 
											     cj_xp_xx_xz, 
											     _MM_SHUFFLE2(1,1)); 

							const __m128d cix = _mm_set1_pd(ci->x); 
							const __m128d ciy = _mm_set1_pd(ci->y); 
							const __m128d ciz = _mm_set1_pd(ci->z); 

							__m128d r0 = _mm_load_pd(&result[0]);
							__m128d r1 = _mm_load_pd(&result[2]);
							__m128d r2 = _mm_load_pd(&result[4]);
							
							r0 += cix * cj_xyp_xx_xy; 
							r1 += ciy * cj_yzp_yy_yz; 
							r2 += ciz * cj_zxp_zz_xz; 
							
							_mm_store_pd(&result[0], r0); 
							_mm_store_pd(&result[2], r1); 
							_mm_store_pd(&result[4], r2); 

							const __m128d cjxy_xx_yy = _mm_shuffle_pd(cj_xp_xx_xz, 
												 cj_yp_xy_yy, 
												 _MM_SHUFFLE2(1,0));
							const __m128d cjyz_xy_yz = _mm_shuffle_pd(cj_yp_xy_yy, 
												 cj_zp_yz_zz,
												 _MM_SHUFFLE2(0,0));
							const __m128d two = _mm_set1_pd(2.0); 
							
							gz01 += two * cjxy_xx_yy + cjyz_xy_yz; 
							const __m128d cj_zp_zz = _mm_shuffle_pd(cj_zp_yz_zz, 
												cj_zp_yz_zz, 
												_MM_SHUFFLE2(1,1)); 
							
							const __m128d cj_xp_xz = _mm_shuffle_pd(cj_xp_xx_xz, 
												cj_xp_xx_xz, 
												_MM_SHUFFLE2(1,1)); 

							gz2X += _mm_add_sd(_mm_mul_sd(two, cj_zp_zz), cj_xp_xz);
							
							__m128d g0 = _mm_load_pd(&g[0]);
							__m128d g2 = _mm_load_pd(&g[2]);
							
							g0 += gz01; 
							g2 += gz2X; 
							
							
							 _mm_store_pd(&g[0], g0);
							 _mm_store_pd(&g[2], g2);
							
						}
					}
				}
				gi[0] = g[0]; 
				gi[1] = g[1]; 
				gi[2] = g[2];
			}
		}
	}
	return result[0] + result[1] + result[2] + 
		result[3] + result[4] + result[5]; 

}

const C3DBounds& C3DPPDivcurlMatrix2::get_size() const
{
	TRACE_FUNCTION; 
	return impl->_M_size; 
}

void C3DPPDivcurlMatrix2::reset(const C3DBounds& size, const C3DFVector& range, const CBSplineKernel& kernel, 
			       double wdiv, double wrot)
{
	TRACE_FUNCTION; 
	impl->reset(size, range, kernel, wdiv, wrot); 
}


NS_MIA_END
