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
private: 

	size_t ksize; 
	vector<double> r20x; 
	vector<double> r11x; 
	vector<double> r02x; 
	vector<double> r20y; 
	vector<double> r11y; 
	vector<double> r02y; 
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

int C2DDivCurlMatrix::get_index(int n1, int n2, int size) const
{
	return impl->get_index(n1, n2, size);
}

C2DDivCurlMatrixImpl::C2DDivCurlMatrixImpl(const CBSplineKernel* kernel)
{
	assert(kernel); 

	ksize = 2*kernel->size() - 1; 
	size_t fsize = ksize * ksize;

	r20x.resize(fsize);
	r11x.resize(fsize);
	r02x.resize(fsize);
	r20y.resize(fsize);
	r11y.resize(fsize);
	r02y.resize(fsize); 


	double nx = 1.0; // field_size.x; 
	double ny = 1.0; // field_size.y; 

	size_t idx = 0; 
	for(size_t y = 0; y < ksize; ++y) {
		for(size_t x = 0; x < ksize; ++x, ++idx) {
			r20x[idx] = integrate2(*kernel, x, y, 2, 0, nx, 0, ksize-1);
			r11x[idx] = integrate2(*kernel, x, y, 1, 1, nx, 0, ksize-1);
			r02x[idx] = integrate2(*kernel, x, y, 0, 2, nx, 0, ksize-1);
			r20y[idx] = integrate2(*kernel, x, y, 2, 0, ny, 0, ksize-1);
			r11y[idx] = integrate2(*kernel, x, y, 1, 1, ny, 0, ksize-1);
			r02y[idx] = integrate2(*kernel, x, y, 0, 2, ny, 0, ksize-1);

			cvdebug() << "(r20x,r20y):" << setw(10) << r20x[idx] << ", " << setw(10) << r20y[idx]
				  << " (r02x,r02y):"<< setw(10) << r02x[idx] << ", " << setw(10) << r02y[idx] << "\n"; 
		}
	}
	
}

int C2DDivCurlMatrixImpl::get_index(int n1, int n2, int size) const
{
	if (n2 < n1)
		swap(n1, n2);
	
	int delta = n2 - n1;
	
	int hsize = (ksize - 1) / 2; 
	if (delta > hsize) 
		return -1; 
	

	int row = hsize;
	if (n1 < hsize) {
		row = n1;
		return ksize * row + n1 + delta; 
	} else {
		int m2 = size - n2 - 1; 
		if (m2 < hsize) {
			row = m2;
			return ksize * row + m2 + delta; 
		}
	}
	return ksize * row + hsize + delta; 
}

double C2DDivCurlMatrixImpl::multiply(const C2DFVectorfield& coefficients) const
{
	double sum = 0.0; 
	double sum11 = 0.0; 
	double sum12 = 0.0; 
	double sum22 = 0.0; 


	C2DFVectorfield::const_iterator cmn = coefficients.begin();
	for(size_t n = 0; n < coefficients.get_size().y; ++n) {
		for(size_t m = 0; m < coefficients.get_size().x; ++m, ++cmn) {
			C2DFVectorfield::const_iterator ckl = coefficients.begin();
			for(size_t l = 0; l < coefficients.get_size().y; ++l)
				for(size_t k = 0; k < coefficients.get_size().x; ++k, ++ckl) {

					int km = get_index(k, m, coefficients.get_size().x);
					int ln = get_index(l, n, coefficients.get_size().y);
					if (km < 0 || ln < 0 ) 
						continue; 
					const double r1111 = r11x[km] * r11y[ln]; 
					
					const double q11 =      r20x[km] * r20y[ln] + r1111;
					const double q12 = 2 * (r20x[km] * r11y[ln] + r11x[km] * r02y[ln]);
					const double q22 =      r02x[km] * r02y[ln] + r1111;

					sum += cmn->x * ckl->x * q11; 
					sum += cmn->x * ckl->y * q12; 
					sum += cmn->y * ckl->y * q22;
		
					sum11 += q11; 
					sum12 += q12; 
					sum22 += q22; 

				}
		}

	}
	cvinfo() << "sump11=" << sum11 << ", sump12=" << sum12 << ", sump22=" << sum22 << "\n";  
	return sum; 
}

C2DFVectorfield C2DDivCurlMatrixImpl::multiply_for_gradient(const C2DFVectorfield& coefficients) const
{

	C2DFVectorfield result(coefficients.get_size()); 
	
	C2DFVectorfield::iterator r   = result.begin();
	C2DFVectorfield::const_iterator cmn = coefficients.begin();
	for(size_t n = 0; n < coefficients.get_size().y; ++n) {
		for(size_t m = 0; m < coefficients.get_size().x; ++m, ++cmn, ++r) {
			*r = C2DFVector(0,0); 
			C2DFVectorfield::const_iterator ckl = coefficients.begin();
			for(size_t l = 0; l < coefficients.get_size().y; ++l)
				for(size_t k = 0; k < coefficients.get_size().x; ++k, ++ckl) {

					int km = get_index(k, m, coefficients.get_size().x);
					int ln = get_index(l, n, coefficients.get_size().y);
					if (km < 0 || ln < 0 ) 
						continue; 
					const double r1111 = r11x[km] * r11y[ln]; 
					
					const double q11 =      r20x[km] * r20y[ln] + r1111;
					const double q12 = 2 * (r20x[km] * r11y[ln] + r11x[km] * r02y[ln]);
					const double q22 =      r02x[km] * r02y[ln] + r1111;

					r->x += (n == l && m == k) ? 2 * ckl->x * q11 : ckl->x * q11; 
					r->x += ckl->y * q12;
					
					r->y += ckl->x * q12;
					r->y += (n == l && m == k) ? 2 * ckl->y * q11 : ckl->y * q22; 
				}
		}

	}
	return result; 
}

NS_MIA_END
