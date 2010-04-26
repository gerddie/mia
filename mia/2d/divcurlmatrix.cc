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
	C2DDivCurlMatrixImpl(const C2DBounds& field_size, const CBSplineKernel* kernel); 

	double multiply(const C2DFVectorfield& coefficients) const; 
private: 
	vector<double> r20x; 
	vector<double> r11x; 
	vector<double> r02x; 
	vector<double> r20y; 
	vector<double> r11y; 
	vector<double> r02y; 
}; 




C2DDivCurlMatrix::C2DDivCurlMatrix(const C2DBounds& field_size, const CBSplineKernel* kernel):
	impl(new C2DDivCurlMatrixImpl(field_size, kernel))
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

C2DDivCurlMatrixImpl::C2DDivCurlMatrixImpl(const C2DBounds& field_size, const CBSplineKernel* kernel):
	r20x(field_size.x * field_size.x), 
	r11x(field_size.x * field_size.x), 
	r02x(field_size.x * field_size.x), 
	r20y(field_size.y * field_size.y), 
	r11y(field_size.y * field_size.y),
	r02y(field_size.y * field_size.y)
{
	assert(kernel); 

	const double Lx = field_size.x - 1; 
	const double Ly = field_size.y - 1; 
	double nx = 1.0; // field_size.x; 
	double ny = 1.0; // field_size.y; 

	size_t idx = 0; 
	for(size_t y = 0; y < field_size.x; ++y) {
		for(size_t x = 0; x < field_size.x; ++x, ++idx) {
			r20x[idx] = integrate2(*kernel, x, y, 2, 0, nx, 0, Lx);
			r11x[idx] = integrate2(*kernel, x, y, 1, 1, nx, 0, Lx);
			r02x[idx] = integrate2(*kernel, x, y, 0, 2, nx, 0, Lx);

		}
	}
	idx = 0; 
	for(size_t y = 0; y < field_size.y; ++y) {
		for(size_t x = 0; x < field_size.y; ++x, ++idx) {
			r20y[idx] = integrate2(*kernel, x, y, 2, 0, ny, 0, Ly);
			r11y[idx] = integrate2(*kernel, x, y, 1, 1, ny, 0, Ly);
			r02y[idx] = integrate2(*kernel, x, y, 0, 2, ny, 0, Ly);
		}
	}

}

double C2DDivCurlMatrixImpl::multiply(const C2DFVectorfield& coefficients) const
{
	double sum = 0.0; 

	C2DFVectorfield::const_iterator cmn = coefficients.begin();
	for(size_t n = 0; n < coefficients.get_size().y; ++n) {
		for(size_t m = 0; m < coefficients.get_size().x; ++m, ++cmn) {
			C2DFVectorfield::const_iterator ckl = coefficients.begin();
			for(size_t l = 0; l < coefficients.get_size().y; ++l)
				for(size_t k = 0; k < coefficients.get_size().x; ++k, ++ckl) {

					size_t km = k + coefficients.get_size().x * m;
					size_t ln = l + coefficients.get_size().y * n;
					const double q11 =      r20x[km] * r20y[ln] + r11x[km] * r11y[ln];
					const double q12 = 2 * (r20x[km] * r11y[ln] + r11x[km] * r02y[ln]);
					const double q22 =      r02x[km] * r02y[ln] + r11x[km] * r11y[ln];

					sum += cmn->x * ckl->x * q11; 
					sum += cmn->x * ckl->y * q12; 
					sum += cmn->y * ckl->y * q22;
		
				}
		}

	}
	return sum / coefficients.size(); 
}

NS_MIA_END
