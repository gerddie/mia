/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
 *
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
 */

#include <cstdlib>
#include <cassert>
#include <limits>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <sstream>
#include <mia/3d/transform/spline.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/3dimageio.hh>

#include <boost/lambda/lambda.hpp>


NS_MIA_BEGIN
using namespace std;
using namespace boost::lambda;


C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PBSplineKernel kernel):
	_M_range(range),
	_M_target_c_rate(1,1,1),
	_M_kernel(kernel),
	_M_scale(1.0, 1.0, 1.0),
	_M_scales_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y), 
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z), 
	_M_grid_valid(false)
{
	TRACE_FUNCTION;
	unsigned int s = _M_kernel->get_active_halfrange() - 1; 
	_M_shift = C3DBounds(s,s,s); 
	// add one for convenience: on the right side a zero padding is needed to 
	// avoid memory access failures, because interpolatimng at the right boundary 
	// us isually done with a zero weight on the last component, but the 
	// code doesn't check if we want to access this 
	// Quation is, what is more expensive 
	_M_enlarge = 2 * _M_shift;
	
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
	assert(_M_range.z > 0);
}

C3DSplineTransformation::C3DSplineTransformation(const C3DSplineTransformation& org):
   	_M_range(org._M_range),
	_M_target_c_rate(org._M_target_c_rate),
	_M_coefficients( org._M_coefficients),
	_M_kernel(org._M_kernel),
	_M_shift(org._M_shift),
	_M_enlarge(org._M_enlarge),
	_M_scales_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y),
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z),
	_M_grid_valid(false)
{
	TRACE_FUNCTION;
}

C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PBSplineKernel kernel, 
						 const C3DFVector& c_rate):
	_M_range(range),
	_M_target_c_rate(c_rate),
	_M_kernel(kernel),
	_M_scales_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y),
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z),
	_M_grid_valid(false)
{
	TRACE_FUNCTION;

	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
	assert(_M_range.z > 0);
	assert(c_rate.x >= 1.0);
	assert(c_rate.y >= 1.0);
	assert(c_rate.z >= 1.0);
	assert(kernel); 
	
	unsigned int s = _M_kernel->get_active_halfrange() - 1; 
	_M_shift = C3DBounds(s,s,s); 
	_M_enlarge = 2 * _M_shift;

	C3DBounds csize(C3DFVector(range - C3DBounds::_1 ) / c_rate); 
	if (csize.x < 2)
		csize.x = 2; 
	if (csize.y < 2)
		csize.y = 2; 
	if (csize.z < 2)
		csize.z = 2; 
	csize += _M_enlarge;
	_M_coefficients = C3DFVectorfield(csize);
	reinit();
}

void C3DSplineTransformation::set_coefficients(const C3DFVectorfield& field)
{
	TRACE_FUNCTION;
	_M_scales_valid = (_M_coefficients.get_size() == field.get_size());
	_M_coefficients = field;
//	_M_target_c_rate =  C3DFVector(_M_range) / C3DFVector(field.get_size() - _M_enlarge);
}

void C3DSplineTransformation::set_coefficients_and_prefilter(const C3DFVectorfield& field)
{
	TRACE_FUNCTION;
	C3DFVectorfield help1(field.get_size());
	C3DFVectorfield help2(field.get_size());
	vector<C3DFVector> buffer(field.get_size().x); 
	for(size_t z = 0; z < field.get_size().z; ++z)
		for(size_t y = 0; y < field.get_size().y; ++y) {
			field.get_data_line_x(y, z, buffer); 
			_M_kernel->filter_line(buffer); 
			help1.put_data_line_x(y, z, buffer); 
		}
	
	buffer.resize(field.get_size().y); 
	for(size_t z = 0; z < field.get_size().z; ++z)
		for(size_t x = 0; x < field.get_size().x; ++x) {
			help1.get_data_line_y(x, z, buffer); 
			_M_kernel->filter_line(buffer); 
			help2.put_data_line_y(x, z, buffer); 
		}
	
	buffer.resize(field.get_size().z); 
	for(size_t y = 0; y < field.get_size().y; ++y)
		for(size_t x = 0; x < field.get_size().x; ++x) {
			help2.get_data_line_z(x, y, buffer); 
			_M_kernel->filter_line(buffer); 
			help1.put_data_line_z(x, y, buffer); 
		}
	
	set_coefficients(help1); 
}


C3DBounds C3DSplineTransformation::get_enlarge() const
{
	TRACE_FUNCTION;
	return _M_enlarge; 
}

void C3DSplineTransformation::reinit() const
{
	TRACE_FUNCTION;
	if (!_M_scales_valid) {
		TRACE("C3DSplineTransformation::reinit applies");
		_M_scale = C3DFVector(_M_coefficients.get_size() - C3DBounds::_1 - _M_enlarge) / 
			C3DFVector(_M_range - C3DBounds::_1);
		
		_M_inv_scale = C3DFVector::_1 / _M_scale; 

		_M_mx = get_derivative_row(_M_range.x, _M_coefficients.get_size().x, _M_scale.x); 
		_M_my = get_derivative_row(_M_range.y, _M_coefficients.get_size().y, _M_scale.y); 
		_M_mz = get_derivative_row(_M_range.z, _M_coefficients.get_size().z, _M_scale.z); 

		cvdebug() << "C3DSplineTransformation:"
			  << " _M_coefficients.get_size() = " << _M_coefficients.get_size()
			  << " _M_range= " << _M_range 
			  << "  _M_scale = " << _M_scale << "\n"; 
		_M_scales_valid = true;
	}
}

C3DFVector C3DSplineTransformation::interpolate(const C3DFVector& x) const 
{
	TRACE_FUNCTION;
	vector<double> xweights(_M_kernel->size()); 
	vector<double> yweights(_M_kernel->size()); 
	vector<double> zweights(_M_kernel->size()); 
	
	C3DBounds start(_M_kernel->get_start_idx_and_value_weights(x.x, xweights), 
			_M_kernel->get_start_idx_and_value_weights(x.y, yweights), 
			_M_kernel->get_start_idx_and_value_weights(x.z, zweights)); 
	
	return sum(start, xweights, yweights, zweights);
}

C3DFVector C3DSplineTransformation::apply(const C3DFVector& x) const
{
	TRACE_FUNCTION;
	assert(_M_scales_valid);
	return interpolate(scale(x));
}

C3DTransformation *C3DSplineTransformation::do_clone()const
{
	TRACE_FUNCTION;
	return new C3DSplineTransformation(*this);
}

C3DTransformation *C3DSplineTransformation::invert() const
{
	TRACE_FUNCTION;
	assert(0 && "not implemented"); 
	return new C3DSplineTransformation(*this);
}


C3DFVector C3DSplineTransformation::operator () (const C3DFVector& x) const
{
	TRACE_FUNCTION;
	return x - apply(x);
}

C3DFVector C3DSplineTransformation::scale( const C3DFVector& x) const
{
	TRACE_FUNCTION;
	assert(_M_scales_valid);
	return x * _M_scale + C3DFVector(_M_shift);
}

const C3DBounds& C3DSplineTransformation::get_size() const
{
	TRACE_FUNCTION;
	return _M_range;
}

CDoubleVector C3DSplineTransformation::get_parameters() const
{
	TRACE_FUNCTION;
	CDoubleVector result(_M_coefficients.size() * 3);
	auto r = result.begin();
	for(auto f = _M_coefficients.begin(); f != _M_coefficients.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
		*r++ = f->z;
	}
	return result;
}

void C3DSplineTransformation::set_parameters(const CDoubleVector& params)
{
	TRACE_FUNCTION;
	assert(3 * _M_coefficients.size() == params.size());
	auto r = params.begin();
	for(auto f = _M_coefficients.begin(); f != _M_coefficients.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
		f->z = *r++;
	}
}

bool C3DSplineTransformation::save(const std::string& /*filename*/,
				   const std::string& /*type*/) const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	assert(!"not implemented");
	return false;
}


bool C3DSplineTransformation::refine()
{
	TRACE_FUNCTION;
	C3DBounds csize = C3DBounds((C3DFVector(_M_range) + _M_target_c_rate - C3DFVector::_1)/ _M_target_c_rate)
		+ _M_enlarge; 

	cvdebug() << "Refine from " << _M_coefficients.get_size() << " to " << csize << "\n"; 
	// no refinement necessary? 
	if (csize.x <= _M_coefficients.get_size().x && 
	    csize.y <= _M_coefficients.get_size().y && 
	    csize.z <= _M_coefficients.get_size().z) 
		return false; 

	// now interpolate the new coefficients 
	// \todo this should be done faster by a filter 
	reinit();
	T3DConvoluteInterpolator<C3DFVector> interp(_M_coefficients, _M_kernel);

	C3DFVectorfield coeffs(csize);
	C3DFVector dx(C3DFVector(_M_coefficients.get_size() - C3DBounds::_1 - _M_enlarge) / 
		      C3DFVector(csize -  C3DBounds::_1 - _M_enlarge)); 

	auto ic = coeffs.begin();

	cvdebug() << "refine scale = " << dx << "\n"; 

	C3DFVector fshift(_M_shift); 
	for (size_t z = 0; z < csize.z; ++z)
		for (size_t y = 0; y < csize.y; ++y)
			for (size_t x = 0; x < csize.x; ++x, ++ic) {
				C3DFVector X(x,y,z); 
				*ic = interp(dx * (X - fshift) + fshift); 
			}
	
	set_coefficients(coeffs);
	reinit();
	_M_grid_valid = false; 
	return true; 
}


P3DTransformation C3DSplineTransformation::do_upscale(const C3DBounds& size) const
{
	TRACE_FUNCTION;

	C3DFVector mx(C3DFVector(size) / C3DFVector(_M_range));

	C3DSplineTransformation *help = new C3DSplineTransformation(size, _M_kernel);
	C3DFVectorfield new_coefs(_M_coefficients.get_size()); 
	
	transform(_M_coefficients.begin(), _M_coefficients.end(), new_coefs.begin(), mx * _1 );
	help->set_coefficients(new_coefs); 
	help->_M_target_c_rate = _M_target_c_rate; 
	return P3DTransformation(help);
}

void C3DSplineTransformation::add(const C3DTransformation& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == get_size());

	reinit();
	a.reinit();
	
	C3DFVectorfield new_coef(_M_coefficients.get_size()); 
	auto i = new_coef.begin();

	for (size_t z = 0; z < _M_coefficients.get_size().z; ++z)  {
		for (size_t y = 0; y < _M_coefficients.get_size().y; ++y)  {
			for (size_t x = 0; x < _M_coefficients.get_size().x; ++x, ++i)  {
				C3DFVector v = (C3DFVector(x, y, z) - C3DFVector(_M_shift)) * _M_inv_scale;
				C3DFVector u = a(v);
				*i = v + apply(u) - u;
			}
		}
	}
	_M_coefficients = new_coef; 
}

size_t C3DSplineTransformation::degrees_of_freedom() const
{
	TRACE_FUNCTION;
	return _M_coefficients.size() * 3;
}

void C3DSplineTransformation::update(float step, const C3DFVectorfield& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == _M_coefficients.get_size());
	auto ci = _M_coefficients.begin();
	auto ce = _M_coefficients.end();
	auto ai = a.begin();

	while( ci != ce ) {
		*ci++ += C3DFVector(step * *ai++);
	}
}

C3DFVector C3DSplineTransformation::sum(const C3DBounds& start, 
					const vector<double>& xweights, 
					const vector<double>& yweights, 
					const vector<double>& zweights) const 
{
//	TRACE_FUNCTION;
	C3DFVector result; 
	size_t z = start.z; 
	for(auto wz = zweights.begin(); z < _M_coefficients.get_size().z && 
		    wz != zweights.end(); ++z, ++wz)  {
		C3DFVector hy; 
		size_t y = start.y; 
		for(auto wy = yweights.begin(); y < _M_coefficients.get_size().y && 
			    wy != yweights.end(); ++y, ++wy)  {
			C3DFVector hx; 
			size_t x = start.x; 
			auto wx = xweights.begin();
			for(auto cx = _M_coefficients.begin_at(start.x,y,z); 
			    x < _M_coefficients.get_size().x && 
				    wx != xweights.end(); ++x, ++wx, ++cx)  {
				hx += *wx * *cx; 
			}
			hy += hx * *wy; 
		}
		result += hy * *wz; 
	}
	return C3DFVector(result); 
}
					

C3DFMatrix C3DSplineTransformation::derivative_at(const C3DFVector& v) const
{
//	TRACE_FUNCTION;
	
	if (v.x < 0 || v.y < 0 || v.z < 0 ||
	    v.x > _M_range.x - 1 ||  v.y > _M_range.y - 1  || v.z > _M_range.z -1) {
		return C3DFMatrix::_1; 
	}

	vector<double> xweights(_M_kernel->size()); 
	vector<double> yweights(_M_kernel->size()); 
	vector<double> zweights(_M_kernel->size()); 
	vector<double> xdweights(_M_kernel->size()); 
	vector<double> ydweights(_M_kernel->size()); 
	vector<double> zdweights(_M_kernel->size()); 

	const C3DFVector x = scale(v);

	C3DBounds start(_M_kernel->get_start_idx_and_value_weights(x.x, xweights), 
			_M_kernel->get_start_idx_and_value_weights(x.y, yweights), 
			_M_kernel->get_start_idx_and_value_weights(x.z, zweights)); 

	_M_kernel->get_start_idx_and_derivative_weights(x.x, xdweights); 
	_M_kernel->get_start_idx_and_derivative_weights(x.y, ydweights); 
	_M_kernel->get_start_idx_and_derivative_weights(x.z, zdweights); 

	C3DFMatrix result(sum(start, xdweights, yweights,  zweights) * (-_M_scale.x), 
			  sum(start, xweights,  ydweights, zweights) * (-_M_scale.y), 
			  sum(start, xweights,  yweights, zdweights) * (-_M_scale.z)); 
	result.x.x += 1.0; 
	result.y.y += 1.0; 
	result.z.z += 1.0; 
	return result; 
}

C3DFMatrix C3DSplineTransformation::derivative_at(int x, int y, int z) const
{
	TRACE_FUNCTION;
	return derivative_at(C3DFVector(x,y,z));
}

void C3DSplineTransformation::set_identity()
{
	TRACE_FUNCTION;
	fill(_M_coefficients.begin(), _M_coefficients.end(), C3DFVector());
}

float C3DSplineTransformation::get_max_transform() const
{
	TRACE_FUNCTION;
	// assuming the maximum spline coef is equal
	auto i = _M_coefficients.begin();
	auto e = _M_coefficients.end();

	assert(i != e);
	float value = i->norm2();

	++i;
	while (i != e) {
		float v = i->norm2();
		if (value < v)
			value = v;
		++i;
	}
	return sqrt(value);


}

void C3DSplineTransformation::init_grid()const
{
	TRACE_FUNCTION; 
	reinit();
	if (!_M_grid_valid) {
		cvdebug() << "really run C3DSplineTransformation::init_grid\n"; 
		// pre-evaluateof fixed-grid coefficients
		size_t n_elms = _M_kernel->size(); 
		for (size_t i = 0; i < _M_range.x; ++i) {
			_M_x_weights[i].resize(n_elms); 
			_M_x_indices[i] = _M_kernel->get_start_idx_and_value_weights(i * _M_scale.x + _M_shift.x, 
										     _M_x_weights[i]); 
		}
		for (size_t i = 0; i < _M_range.y; ++i){
			_M_y_weights[i].resize(n_elms); 
			_M_y_indices[i] = _M_kernel->get_start_idx_and_value_weights(i * _M_scale.y + _M_shift.y, 
										     _M_y_weights[i]); 
		}
		for (size_t i = 0; i < _M_range.z; ++i){
			_M_z_weights[i].resize(n_elms); 
			_M_z_indices[i] = _M_kernel->get_start_idx_and_value_weights(i * _M_scale.z + _M_shift.z, 
										     _M_z_weights[i]); 
		}
		_M_grid_valid = true; 
	}
}

C3DTransformation::const_iterator C3DSplineTransformation::begin() const
{
	TRACE_FUNCTION;
	init_grid(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this));
}

C3DTransformation::const_iterator C3DSplineTransformation::end() const
{
	TRACE_FUNCTION;
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this));
}

/*
   This function evaluates the weights of the derivative  of the spline transformation 
   w.r.t. the coefficients. 
*/

C3DSplineTransformation::CSplineDerivativeRow 
C3DSplineTransformation::get_derivative_row(size_t nin, size_t nout, double scale) const 
{
	TRACE_FUNCTION;
	CSplineDerivativeRow result; 
	for(size_t o = 0; o < nout; ++o) {
		CSplineDerivativeRow::value_type v; 
		v.first = -1; 
		double sum_y = 0.0; 
		for(size_t i = 0; i < nin; ++i) {
			double x = i * scale + _M_shift.x - o; 
			if (std::fabs(x) <  _M_kernel->get_nonzero_radius()) {
				if (v.first < 0)
					v.first = i; 
				double y = _M_kernel->get_weight_at(x, 0); 
				sum_y += y; 
				if (y > 0) 
					v.second.push_back(y); 
				else 
					break; 
			}
		}
		result.push_back(v); 
	}
	return result; 
}

void C3DSplineTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	TRACE_FUNCTION;

	assert(params.size() == _M_coefficients.size() * 3);
	assert(gradient.get_size() == _M_range); 
	reinit(); 
	
	C3DFVectorfield tmp(C3DBounds(gradient.get_size().x, 
				      gradient.get_size().y, 
				      _M_coefficients.get_size().z));
	
	vector<C3DFVector> in_buffer(gradient.get_size().z); 
	vector<C3DFVector> out_buffer(_M_coefficients.get_size().z);
	
	for (size_t iy = 0; iy < gradient.get_size().y; ++iy) {
		for (size_t ix = 0; ix < gradient.get_size().x; ++ix) {
			gradient.get_data_line_z(ix, iy, in_buffer);
			for(size_t i = 0; i < _M_coefficients.get_size().z; ++i) {
				const CSplineDerivativeRow::value_type& myrow = _M_mz[i]; 
				out_buffer[i] = inner_product(myrow.second.begin(), myrow.second.end(), 
							      in_buffer.begin() + myrow.first, C3DFVector());
			}
			tmp.put_data_line_z(ix, iy, out_buffer);
		}
	}

	C3DFVectorfield tmp2(C3DBounds(gradient.get_size().x, 
				       _M_coefficients.get_size().y, 
				       _M_coefficients.get_size().z));

	in_buffer.resize(gradient.get_size().y); 
	out_buffer.resize(_M_coefficients.get_size().y);

	for (size_t iz = 0; iz < _M_coefficients.get_size().z; ++iz) {
		for (size_t ix = 0; ix < gradient.get_size().x; ++ix) {
			tmp.get_data_line_y(ix, iz, in_buffer);
			for(size_t i = 0; i < _M_coefficients.get_size().y; ++i) {
				const CSplineDerivativeRow::value_type& myrow = _M_my[i]; 
				out_buffer[i] = inner_product(myrow.second.begin(), myrow.second.end(), 
							      in_buffer.begin() + myrow.first, C3DFVector());
			}
			tmp2.put_data_line_y(ix, iz, out_buffer);
		}
	}
	
	in_buffer.resize(gradient.get_size().x); 
	// x convolution and copy to output
	auto r = params.begin(); 
	for (size_t iz = 0; iz < _M_coefficients.get_size().z; ++iz) {
		for (size_t iy = 0; iy < _M_coefficients.get_size().y; ++iy) {
			tmp2.get_data_line_x(iy, iz, in_buffer);
			for(size_t x = 0; x < _M_coefficients.get_size().x; ++x, r+=3) {
				const CSplineDerivativeRow::value_type& mxrow = _M_mx[x]; 
				const C3DFVector v = inner_product(mxrow.second.begin(), mxrow.second.end(), 
								   in_buffer.begin() + mxrow.first, C3DFVector());
				r[0] = -v.x;
				r[1] = -v.y;
				r[2] = -v.z;
			}
		}
	}
}

float  C3DSplineTransformation::pertuberate(C3DFVectorfield& v) const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED; 

	reinit();
	C3DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	C3DFVector lx_max;
	for (size_t z = 0; z < v.get_size().z; ++z)
		for (size_t y = 0; y < v.get_size().y; ++y)
			for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
				C3DFVector lx(x,y,z);
				C3DFMatrix j = derivative_at(lx);
				j.x.x = j.x.x * _M_scale.x;
				j.x.y = j.x.y * _M_scale.x;
				j.y.x = j.y.x * _M_scale.y;
				j.y.y = j.y.y * _M_scale.y;
				
				const C3DFVector u = j *  *iv;
				*iv -= u;
				float gamma = iv->norm2();
				if (gamma > max_gamma) {
					lx_max = lx;
					max_gamma = gamma;
				}
			}
	return sqrt(max_gamma);
}

float C3DSplineTransformation::get_jacobian(const C3DFVectorfield& v, float delta) const
{
	FUNCTION_NOT_TESTED;
	assert(v.get_size() == get_coeff_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();
	delta *= 0.5;

	C3DFVector lx_min;
	for(size_t z = 1; z < v.get_size().z - 1; ++z) {
		for(size_t y = 1; y < v.get_size().y - 1; ++y) {
			C3DFVectorfield::const_iterator iv = v.begin_at(1,y,z);
			for(size_t x = 1; x < v.get_size().x - 1; ++x, ++iv) {
				const C3DFVector lx(x,y,z);
				C3DFMatrix J =  derivative_at(lx);
				J.x.x = _M_inv_scale.x - J.x.x;
				J.x.y = - J.x.y;
				J.y.x = - J.y.x;
				J.y.y = _M_inv_scale.y - J.y.y;
				
				J.x -= delta * (iv[1]  - iv[-1]);
				J.y -= delta * (iv[dx] - iv[-dx]);
				
				const float j = J.x.x * J.y.y - J.x.y * J.y.x;
				if ( j_min > j) {
					j_min = j;
					lx_min = lx;
				}
			}
			
		}
	}
	return j_min * _M_scale.y * _M_scale.x;
}

C3DFVector C3DSplineTransformation::on_grid(const C3DBounds& x) const
{
//	TRACE_FUNCTION;
	// this is a bit expensive, but uses less memory 
	// one could evaluate the whole grid using convolution 
	// but this would require more memory
	assert(x.x < _M_range.x); 
	assert(x.y < _M_range.y); 
	assert(x.z < _M_range.z); 
	assert(_M_grid_valid); 

	C3DBounds start(_M_x_indices[x.x], _M_y_indices[x.y], _M_z_indices[x.z]); 
	return sum(start, _M_x_weights[x.x], _M_y_weights[x.y], _M_z_weights[x.z]);
}

const C3DBounds& C3DSplineTransformation::get_coeff_size() const
{
	TRACE_FUNCTION;
	return _M_coefficients.get_size();
}


C3DSplineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DSplineTransformation& trans):
	C3DTransformation::iterator_impl(pos,size), 
	_M_trans(trans), 
	_M_value_valid(false)
{
	TRACE_FUNCTION;
}
 
C3DTransformation::iterator_impl * C3DSplineTransformation::iterator_impl::clone() const
{
	TRACE_FUNCTION;
	return new C3DSplineTransformation::iterator_impl(get_pos(), get_size(), _M_trans); 
}

const C3DFVector&  C3DSplineTransformation::iterator_impl::do_get_value()const
{
//	TRACE_FUNCTION;
	if (!_M_value_valid) {
		_M_value = C3DFVector(get_pos()) - _M_trans.on_grid(get_pos());
		_M_value_valid = true; 
	}
	return _M_value; 
}

void C3DSplineTransformation::iterator_impl::do_x_increment()
{
//	TRACE_FUNCTION;
	_M_value_valid = false; 
}
void C3DSplineTransformation::iterator_impl::do_y_increment()
{
//	TRACE_FUNCTION;
	_M_value_valid = false; 
}
void C3DSplineTransformation::iterator_impl::do_z_increment()
{
//	TRACE_FUNCTION;
	_M_value_valid = false; 
}

double C3DSplineTransformation::get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const
{
	TRACE_FUNCTION;
//	FUNCTION_NOT_TESTED;
	reinit(); 

	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C3DPPDivcurlMatrix2(_M_coefficients.get_size(), 
							       C3DFVector(_M_range), 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), C3DFVector(_M_range), 
					 *_M_kernel, wd, wr); 
	
	
	return _M_divcurl_matrix->evaluate(_M_coefficients, gradient); 
}

double C3DSplineTransformation::get_divcurl_cost(double wd, double wr) const
{
	TRACE_FUNCTION;

	reinit(); 
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C3DPPDivcurlMatrix2(_M_coefficients.get_size(), C3DFVector(_M_range), 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), C3DFVector(_M_range), 
					 *_M_kernel, wd, wr); 
	

	return *_M_divcurl_matrix * _M_coefficients; 
}


class C3DSplineTransformCreator: public C3DTransformCreator {
public:
	C3DSplineTransformCreator(EInterpolation ip, const C3DFVector& rates, bool debug);
	virtual P3DTransformation do_create(const C3DBounds& size) const;
private:
	PBSplineKernel _M_kernel;
	C3DFVector _M_rates;
	bool _M_debug; 
};

C3DSplineTransformCreator::C3DSplineTransformCreator(EInterpolation ip, const C3DFVector& rates, bool debug):
	_M_rates(rates), 
	_M_debug(debug)
{
	TRACE_FUNCTION;
	switch (ip){
	case ip_bspline2:
		_M_kernel.reset(new CBSplineKernel2());
		break;
	case ip_bspline3:
		_M_kernel.reset(new CBSplineKernel3());
		break;
	case ip_bspline4:
		_M_kernel.reset(new CBSplineKernel4());
		break;
	case ip_bspline5:
		_M_kernel.reset(new CBSplineKernel5());
		break;
	case ip_omoms3:
		_M_kernel.reset(new CBSplineKernelOMoms3());
	default:
		throw invalid_argument("Spline kernel type required"); 
	}
}


P3DTransformation C3DSplineTransformCreator::do_create(const C3DBounds& size) const
{
	TRACE_FUNCTION;

	assert(_M_kernel); 
	P3DTransformation result(new C3DSplineTransformation(size, _M_kernel, _M_rates));
	if (_M_debug) 
		result->set_debug(); 
	return result;
}


class C3DSplineTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	typedef C3DTransformCreatorPlugin::ProductPtr ProductPtr;

	C3DSplineTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
private:
	EInterpolation _M_ip;
	float _M_rate;
	bool _M_debug; 
};

C3DSplineTransformCreatorPlugin::C3DSplineTransformCreatorPlugin():
	C3DTransformCreatorPlugin("spline"),
	_M_ip(ip_bspline3),
	_M_rate(10), 
	_M_debug(false)
{
	add_parameter("interp",
            new CDictParameter<EInterpolation>(_M_ip, GInterpolatorTable, "image interpolator"));
	add_parameter("rate",
		      new CFloatParameter(_M_rate, 1, numeric_limits<float>::max(), false,
					  "isotropic coefficient rate in pixels"));
	add_parameter("debug",
		      new CBoolParameter(_M_debug, false, "enable additional debuging output"));

}

C3DSplineTransformCreatorPlugin::ProductPtr
C3DSplineTransformCreatorPlugin::do_create() const
{
	TRACE_FUNCTION;
	return ProductPtr(new C3DSplineTransformCreator(_M_ip, C3DFVector(_M_rate, _M_rate, _M_rate), _M_debug));
}

bool C3DSplineTransformCreatorPlugin::do_test() const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;

	return true;
}

const std::string C3DSplineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create spline transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSplineTransformCreatorPlugin();
}

NS_MIA_END

