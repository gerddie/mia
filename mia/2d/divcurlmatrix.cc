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

#include <cassert>
#include <iomanip>

#include <mia/2d/divcurlmatrix.hh>


NS_MIA_BEGIN
using namespace std; 

struct C2DDivCurlMatrixImpl {
	C2DDivCurlMatrixImpl(const CBSplineKernel* kernel); 

	double multiply(const C2DFVectorfield& coefficients) const; 
	C2DFVectorfield multiply_for_gradient(const C2DFVectorfield& coefficients) const; 
	int get_index(int n1, int n2, int size) const; 
	double value_at(const C2DFVectorfield& coefficients, size_t m, size_t n)const; 
	C2DFVector derivative_at(const C2DFVectorfield& coefficients, size_t m, size_t n)const; 
private: 

	int ksize;
	int hsupport_size; 
	vector<double> b3; 
	vector<double> b2; 
	vector<double> b1; 
	vector<double> b0; 

	vector<double> r20; 
	vector<double> r11; 
	vector<double> r02; 
	
}; 




C2DDivCurlMatrix::C2DDivCurlMatrix(const CBSplineKernel* kernel):
	impl(new C2DDivCurlMatrixImpl(kernel))
{
	
}

C2DDivCurlMatrix::~C2DDivCurlMatrix() 
{
	delete impl; 
}

double C2DDivCurlMatrix::multiply(const C2DFVectorfield& coefficients) const
{
	return impl->multiply(coefficients); 
}

C2DFVectorfield C2DDivCurlMatrix::multiply_for_gradient(const C2DFVectorfield& coefficients) const
{
	return impl->multiply_for_gradient(coefficients); 
}

double C2DDivCurlMatrix::value_at(const C2DFVectorfield& coefficients, size_t m, size_t n)const
{
	return impl->value_at(coefficients, m, n); 
}

C2DFVector C2DDivCurlMatrix::derivative_at(const C2DFVectorfield& coefficients, size_t m, size_t n) const
{
	return impl->derivative_at(coefficients, m, n); 
}

C2DDivCurlMatrixImpl::C2DDivCurlMatrixImpl(const CBSplineKernel* kernel)
{
	assert(kernel); 

	ksize = kernel->size(); 
	hsupport_size = (kernel->size() - 1) / 2; 
	
	b0.resize(ksize); 
	b1.resize(ksize); 
	b2.resize(ksize); 
	b3.resize(ksize); 

	cvdebug() << "Kernel size = " << ksize << "\n"; 

	kernel->get_derivative_weights(0.0, b0, 0);
	kernel->get_derivative_weights(0.0, b1, 1);
	kernel->get_derivative_weights(0.0, b2, 2);
	kernel->get_derivative_weights(0.0, b3, 3);

#if 0
	r20.resize(ksize); 
	r11.resize(ksize); 
	r02.resize(ksize); 

	for (size_t i = 0; i < ksize; ++i) {
		r20[i] = integrate2(*kernel, ksize, ksize + i, 2, 0, 1, 0, 3 * ksize); 
		r11[i] = integrate2(*kernel, ksize, ksize + i, 1, 1, 1, 0, 3 * ksize); 
		r02[i] = integrate2(*kernel, ksize, ksize + i, 0, 2, 1, 0, 3 * ksize); 
	}
#endif
}

double C2DDivCurlMatrixImpl::value_at(const C2DFVectorfield& coefficients, size_t m, size_t n)const
{
	// we can use < ksize-1, because the splines are evaluated centered around
	// 0 thereby setting value [ksize-1] = 0
	double v = 0.0;
	for(int l = -hsupport_size, wl=0; wl < ksize; ++l, ++wl) {
		const size_t nl = l + n; 
		if (nl >= coefficients.get_size().y) 
			continue; 
		for(int k = -hsupport_size, wk=0; wk < ksize; ++k,++wk) {
			const size_t km = k + m;
			if (km < coefficients.get_size().x) {
				const C2DFVector& cmn = coefficients(km,nl); 
				v +=  cmn.x * (b2[wk] * b0[wl]  + b1[wk] * b1[wl]); 
			       	v +=  cmn.y * (b0[wk] * b2[wl]  + b1[wk] * b1[wl]); 
			}
		}
		
	}
	return v; 	
}

C2DFVector C2DDivCurlMatrixImpl::derivative_at(const C2DFVectorfield& coefficients, size_t m, size_t n)const
{
	cvdebug() << m << ", " << n << "\n";
	C2DFVector v(0.0, 0.0);
	for(int l = -hsupport_size, wl=0; wl < ksize; ++l, ++wl) {
		const size_t nl = l + n;
		if (nl >= coefficients.get_size().y)
			continue;
		for(int k = -hsupport_size, wk=0; wk < ksize; ++k,++wk) {
			const size_t km = k + m;
			if (km < coefficients.get_size().x) {
				const C2DFVector& cmn = coefficients(km,nl);
				v.x +=  cmn.x * (b3[wk] * b0[wl]  + b2[wk] * b1[wl]) +
					cmn.y * (b1[wk] * b2[wl]  + b2[wk] * b1[wl]);

				v.y +=  cmn.x * (b2[wk] * b1[wl]  + b1[wk] * b2[wl]) +
					cmn.y * (b0[wk] * b3[wl]  + b1[wk] * b2[wl]);
			}
		}
	}
	return 2.0 * value_at(coefficients, m, n) * v;
}

/*
  The spline approximation of this evaluation generally breaks if the function 
  has an artificial non-zero second order derivative because of the cut-off at the 
  boundary. 
  In non-rigid image registration this is usually nota big deal, since here the cost function
  is normally also close to zero, 
*/

double C2DDivCurlMatrixImpl::multiply(const C2DFVectorfield& coefficients) const
{
	double sum = 0.0; 

	for(size_t n = 0; n < coefficients.get_size().y; ++n) {
		for(size_t m = 0; m < coefficients.get_size().x; ++m) {
			double v = value_at(coefficients, m, n); 
			sum += v*v; 
		}
	}
	return sum; 
}

/*
  It seems that this gradient is _very_ approximate compared to the alaystic function 
*/


C2DFVectorfield C2DDivCurlMatrixImpl::multiply_for_gradient(const C2DFVectorfield& coefficients) const
{

	// far from effective 
	C2DFVectorfield result(coefficients.get_size()); 
	C2DFVectorfield::iterator r   = result.begin();
	C2DFVectorfield::const_iterator cmn = coefficients.begin();
	for(size_t n = 0; n < coefficients.get_size().y; ++n) {
		for(size_t m = 0; m < coefficients.get_size().x; ++m, ++cmn, ++r) {
			*r = derivative_at(coefficients, m, n); 
		}

	}
	return result; 
}

NS_MIA_END
