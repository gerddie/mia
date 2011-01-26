/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/3d/transform/spline.hh>
#include <mia/3d/transformfactory.hh>

#include <boost/lambda/lambda.hpp>


NS_MIA_BEGIN
using namespace std;
using namespace boost::lambda;


C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PBSplineKernel kernel):
	_M_range(range),
	_M_target_c_rate(1,1,1),
	_M_kernel(kernel),
	_M_shift(0), 
	_M_enlarge(0), 
	_M_scale(1.0, 1.0, 1.0),
	_M_interpolator_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y), 
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z), 
	_M_grid_valid(false)

{
	
	_M_shift = _M_kernel->get_active_halfrange() - 1; 
	_M_enlarge = 2*_M_shift; 
	
	TRACE_FUNCTION;
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
	_M_interpolator_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y),
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z),
	_M_grid_valid(false)
{
}

C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PBSplineKernel kernel, 
						 const C3DFVector& c_rate):
	_M_range(range),
	_M_target_c_rate(c_rate),
	_M_kernel(kernel),
	_M_interpolator_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y),
	_M_z_weights(_M_range.z),
	_M_z_indices(_M_range.z),
	_M_grid_valid(false)
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
	assert(_M_range.z > 0);
	assert(c_rate.x >= 1.0);
	assert(c_rate.y >= 1.0);
	assert(c_rate.z >= 1.0);
	assert(kernel); 

	
	_M_shift = _M_kernel->get_active_halfrange() - 1; 
	_M_enlarge = 2*_M_shift; 

	C3DBounds csize(size_t((range.x + c_rate.x - 1) / c_rate.x) + _M_enlarge,
			size_t((range.y + c_rate.y - 1) / c_rate.y) + _M_enlarge,
			size_t((range.z + c_rate.z - 1) / c_rate.z) + _M_enlarge);
	_M_coefficients = C3DFVectorfield(csize);
	reinit();
}

void C3DSplineTransformation::set_coefficients(const C3DFVectorfield& field)
{
	TRACE_FUNCTION;
	_M_coefficients = field;
	_M_interpolator_valid = false;

	_M_target_c_rate.x = float(_M_range.x) / (field.get_size().x - _M_enlarge);
	_M_target_c_rate.y = float(_M_range.y) / (field.get_size().y - _M_enlarge);
	_M_target_c_rate.z = float(_M_range.z) / (field.get_size().z - _M_enlarge);
}

void C3DSplineTransformation::reinit() const
{
	TRACE_FUNCTION;
	if (!_M_interpolator_valid) {
		TRACE("C3DSplineTransformation::reinit applies");
		_M_scale.x = float(_M_coefficients.get_size().x - 1 - _M_enlarge) / (_M_range.x - 1);
		_M_scale.y = float(_M_coefficients.get_size().y - 1 - _M_enlarge) / (_M_range.y - 1);
		_M_scale.z = float(_M_coefficients.get_size().z - 1 - _M_enlarge) / (_M_range.z - 1);
		_M_inv_scale.x = float(_M_range.x - 1) / (_M_coefficients.get_size().x - 1 - _M_enlarge);
		_M_inv_scale.y = float(_M_range.y - 1) / (_M_coefficients.get_size().y - 1 - _M_enlarge);
		_M_inv_scale.z = float(_M_range.z - 1) / (_M_coefficients.get_size().z - 1 - _M_enlarge);

		_M_mx = get_derivative_row(_M_range.x, _M_coefficients.get_size().x, _M_scale.x); 
		_M_my = get_derivative_row(_M_range.y, _M_coefficients.get_size().y, _M_scale.y); 
		_M_mz = get_derivative_row(_M_range.z, _M_coefficients.get_size().z, _M_scale.z); 
		_M_interpolator_valid = true;
	}
}

C3DFVector C3DSplineTransformation::interpolate(const C3DFVector& x) const 
{
	std::vector<double> xweights(_M_kernel->size()); 
	std::vector<double> yweights(_M_kernel->size()); 
	std::vector<double> zweights(_M_kernel->size()); 
	
	C3DBounds start(_M_kernel->get_start_idx_and_value_weights(x.x, xweights), 
			_M_kernel->get_start_idx_and_value_weights(x.y, yweights), 
			_M_kernel->get_start_idx_and_value_weights(x.z, zweights)); 
	
	return sum(start, xweights, yweights, zweights); 
}

C3DFVector C3DSplineTransformation::apply(const C3DFVector& x) const
{
	assert(_M_interpolator_valid);
	return interpolate(scale(x));
}

C3DTransformation *C3DSplineTransformation::do_clone()const
{
	FUNCTION_NOT_TESTED;
	return new C3DSplineTransformation(*this);
}

C3DTransformation *C3DSplineTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C3DSplineTransformation(*this);
}


C3DFVector C3DSplineTransformation::operator () (const C3DFVector& x) const
{
	return x - apply(x);
}

C3DFVector C3DSplineTransformation::scale( const C3DFVector& x) const
{
	assert(_M_interpolator_valid);
	return x * _M_scale + C3DFVector(_M_shift,_M_shift,_M_shift);
}

const C3DBounds& C3DSplineTransformation::get_size() const
{
	return _M_range;
}

gsl::DoubleVector C3DSplineTransformation::get_parameters() const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	gsl::DoubleVector result(_M_coefficients.size() * 3);
	auto r = result.begin();
	for(auto f = _M_coefficients.begin(); f != _M_coefficients.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
		*r++ = f->z;
	}
	return result;
}

void C3DSplineTransformation::set_parameters(const gsl::DoubleVector& params)
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	assert(3 * _M_coefficients.size() == params.size());
	auto r = params.begin();
	for(auto f = _M_coefficients.begin(); f != _M_coefficients.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
		f->z = *r++;
	}
	_M_interpolator_valid = false; 
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
	FUNCTION_NOT_TESTED;
	C3DBounds csize(size_t((_M_range.x + _M_target_c_rate.x - 1) / _M_target_c_rate.x) + _M_enlarge,
			size_t((_M_range.y + _M_target_c_rate.y - 1) / _M_target_c_rate.y) + _M_enlarge,
			size_t((_M_range.z + _M_target_c_rate.z - 1) / _M_target_c_rate.z) + _M_enlarge);
	
	// no refinement necessary? 
	if (csize.x <= _M_coefficients.get_size().x || 
	    csize.y <= _M_coefficients.get_size().y ||
	    csize.z <= _M_coefficients.get_size().z) 
		return false; 

	// now interpolate the new coefficients 
	// \todo this should be done faster by a filter 
	reinit();
	C3DFVectorfield coeffs(csize);
	C3DFVector dx((float)(_M_coefficients.get_size().x - 1 - _M_enlarge) / (float)(csize.x - 1 - _M_enlarge),
		      (float)(_M_coefficients.get_size().y - 1 - _M_enlarge) / (float)(csize.y - 1 - _M_enlarge), 
		      (float)(_M_coefficients.get_size().z - 1 - _M_enlarge) / (float)(csize.z - 1 - _M_enlarge));

	C3DFVectorfield::iterator ic = coeffs.begin();

	for (size_t z = 0; z < csize.z; ++z)
		for (size_t y = 0; y < csize.y; ++y)
			for (size_t x = 0; x < csize.x; ++x, ++ic) {
			*ic = interpolate(C3DFVector(dx.x * (float(x) - _M_shift)+ _M_shift, 
						     dx.y * (float(y) - _M_shift)+ _M_shift, 
						     dx.z * (float(z) - _M_shift)+ _M_shift));
		}

	set_coefficients(coeffs);
	reinit();
	_M_grid_valid = false; 
	return true; 
}


P3DTransformation C3DSplineTransformation::upscale(const C3DBounds& size) const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	C3DFVector mx(((float)size.x - 1.0)/ ((float)_M_range.x - 1.0),
		      ((float)size.y - 1.0)/ ((float)_M_range.y - 1.0), 
		      ((float)size.z - 1.0)/ ((float)_M_range.z - 1.0));

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
	FUNCTION_NOT_TESTED;
	assert(a.get_size() == get_size());

	reinit();
	a.reinit();
	
	C3DFVectorfield new_coef(_M_coefficients.get_size()); 
	C3DFVectorfield::iterator i = new_coef.begin();

	for (size_t z = 0; z < _M_coefficients.get_size().z; ++z)  {
		for (size_t y = 0; y < _M_coefficients.get_size().y; ++y)  {
			for (size_t x = 0; x < _M_coefficients.get_size().x; ++x, ++i)  {
				C3DFVector v = C3DFVector(x - _M_shift, y - _M_shift, z-_M_shift) * _M_inv_scale;
				C3DFVector u = a(v);
				*i = v + apply(u) - u;
			}
		}
	}
	_M_coefficients = new_coef; 
	
	_M_interpolator_valid = false;
}

size_t C3DSplineTransformation::degrees_of_freedom() const
{
	return _M_coefficients.size() * 3;
}

P3DImage C3DSplineTransformation::apply(const C3DImage& image,
					const C3DInterpolatorFactory& ipf) const
{
	TRACE_FUNCTION;
	return transform3d(image, ipf, *this);
}

void C3DSplineTransformation::update(float step, const C3DFVectorfield& a)
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	assert(a.get_size() == _M_coefficients.get_size());
	C3DFVectorfield::iterator ci = _M_coefficients.begin();
	C3DFVectorfield::iterator ce = _M_coefficients.end();
	C3DFVectorfield::const_iterator ai = a.begin();

	while( ci != ce ) {
		*ci++ += step * *ai++;
	}
	_M_interpolator_valid = false;
}

C3DFVector C3DSplineTransformation::sum(const C3DBounds& start, 
					const vector<double>& xweights, 
					const vector<double>& yweights, 
					const vector<double>& zweights) const 
{
	TRACE_FUNCTION;
	C3DFVector result; 
	size_t z = start.z; 
	for(auto wz = zweights.begin(); z < _M_coefficients.get_size().z && wz != zweights.end(); ++z, ++wz)  {
		C3DFVector hy; 
		size_t y = start.y; 
		for(auto wy = yweights.begin(); y < _M_coefficients.get_size().y && wy != yweights.end(); ++y, ++wy)  {
			C3DFVector hx; 
			size_t x = start.x; 
			auto wx = xweights.begin();
			for(auto cx = _M_coefficients.begin_at(start.x,y,z); 
			    x < _M_coefficients.get_size().x && wx != xweights.end(); ++x, ++wx, ++cx)  {
				hx += *wx * *cx; 
			}
			hy += hx * *wy; 
		}
		result += hy * *wz; 
	}
	return result; 
}
					

C3DFMatrix C3DSplineTransformation::derivative_at(const C3DFVector& v) const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	vector<double> xweights(_M_kernel->size()); 
	vector<double> yweights(_M_kernel->size()); 
	vector<double> zweights(_M_kernel->size()); 
	vector<double> xdweights(_M_kernel->size()); 
	vector<double> ydweights(_M_kernel->size()); 
	vector<double> zdweights(_M_kernel->size()); 

	const C3DFVector x = scale(v);
	C3DBounds start(_M_kernel->get_start_idx_and_value_weights(x.x, xweights), 
			_M_kernel->get_start_idx_and_value_weights(x.y, xweights), 
			_M_kernel->get_start_idx_and_value_weights(x.z, xweights)); 

	_M_kernel->get_start_idx_and_derivative_weights(x.x, xdweights); 
	_M_kernel->get_start_idx_and_derivative_weights(x.y, ydweights); 
	_M_kernel->get_start_idx_and_derivative_weights(x.z, zdweights); 

	C3DFMatrix result(sum(start, xdweights,  yweights,  zweights) * (-_M_scale.x), 
			  sum(start,  xweights, ydweights,  zweights) * (-_M_scale.y), 
			  sum(start,  xweights,  yweights, zdweights) * (-_M_scale.z)); 
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
	_M_interpolator_valid = false;
}

float C3DSplineTransformation::get_max_transform() const
{
	TRACE_FUNCTION;
	// assuming the maximum spline coef is equal
	C3DFVectorfield::const_iterator i = _M_coefficients.begin();
	C3DFVectorfield::const_iterator e = _M_coefficients.end();

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
	if (!_M_grid_valid) {
		cvdebug() << "really run C3DSplineTransformation::init_grid\n"; 
		// pre-evaluateof fixed-grid coefficients
		size_t n_elms = _M_kernel->size(); 
		std::vector<int> indices(n_elms); 
		std::vector<double> weights(n_elms); 
		const C3DBounds& csize = _M_coefficients.get_size(); 
		for (size_t i = 0; i < _M_range.x; ++i) {
			(*_M_kernel)(i * _M_scale.x + _M_shift, weights, indices); 
			_M_x_weights[i] = weights; 
			mirror_boundary_conditions(indices, csize.x, 2 * csize.x - 2);
			_M_x_indices[i] = indices; 
		}
		for (size_t i = 0; i < _M_range.y; ++i) {
			(*_M_kernel)(i * _M_scale.y +  _M_shift, weights, indices); 
			_M_y_weights[i] = weights; 
			mirror_boundary_conditions(indices, csize.y, 2 * csize.y - 2);
			_M_y_indices[i] = indices; 
		}

		for (size_t i = 0; i < _M_range.z; ++i) {
			(*_M_kernel)(i * _M_scale.z +  _M_shift, weights, indices); 
			_M_z_weights[i] = weights; 
			mirror_boundary_conditions(indices, csize.z, 2 * csize.z - 2);
			_M_z_indices[i] = indices; 
		}
		_M_grid_valid = true; 
	}
}

C3DTransformation::const_iterator C3DSplineTransformation::begin() const
{
	reinit();
	init_grid(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this));
}

C3DTransformation::const_iterator C3DSplineTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this));
}

struct FCopyX {
	double operator() (const C3DFVector& x) const {
		return x.x; 
	}
}; 

struct FCopyY {
	double operator() (const C3DFVector& x) const {
		return x.y; 
	}
}; 

C3DSplineTransformation::CSplineDerivativeRow 
C3DSplineTransformation::get_derivative_row(size_t nin, size_t nout, double scale) const 
{
	CSplineDerivativeRow result; 
	for(size_t o = 0; o < nout; ++o) {
		CSplineDerivativeRow::value_type v; 
		v.first = -1; 
		for(size_t i = 0; i < nin; ++i) {
			double x = i * scale + _M_shift - o; 
			if (std::fabs(x) <  _M_kernel->get_nonzero_radius()) {
				if (v.first < 0)
					v.first = i; 
				double y = _M_kernel->get_weight_at(x, 0); 
				if (y > 0) 
					v.second.push_back(y); 
				else 
					break; 
			}
		}
		cvdebug() << o << ":" << v.first << ":" << v.second << "\n"; 
		result.push_back(v); 
	}
	return result; 
}

void C3DSplineTransformation::translate(const C3DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	TRACE_FUNCTION;
	FUNCTION_NOT_TESTED;
	assert(params.size() == _M_coefficients.size() * 2);
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

	for (size_t iz = 0; iz < gradient.get_size().z; ++iz) {
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
			tmp.get_data_line_x(iy, iz, in_buffer);
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
	cvdebug() << lx_max << "\n";
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
	cvdebug() << lx_min << "\n";
	return j_min * _M_scale.y * _M_scale.x;
}

C3DFVector C3DSplineTransformation::on_grid(const C3DBounds& x) const
{
	assert(x.x < _M_range.x); 
	assert(x.y < _M_range.y); 
	assert(x.z < _M_range.z); 
	assert(_M_grid_valid); 

	auto & zweights =  _M_z_weights[x.z]; 
	auto & zindices =  _M_z_indices[x.z]; 
	auto & yweights =  _M_y_weights[x.y]; 
	auto & yindices =  _M_y_indices[x.y]; 
	auto & xweights =  _M_x_weights[x.x]; 
	auto & xindices =  _M_x_indices[x.x]; 

	C3DFVector result; 		
	for(size_t z = 0; z < zweights.size(); ++z) {
		C3DFVector hy; 
		for(size_t y = 0; y < yweights.size(); ++y) {
			C3DFVector hx; 
			auto ic = _M_coefficients.begin_at(0, yindices[y], zindices[z]); 
			for(size_t ix = 0; ix < xweights.size(); ++ix) {
				hx += xweights[ix] * ic[xindices[ix]]; 
			}
			hy += yweights[y] * hx; 
		}
		result += hy * zweights[z]; 
	}
	return result; 
}

const C3DBounds& C3DSplineTransformation::get_coeff_size() const
{
	return _M_coefficients.get_size();
}


C3DSplineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DSplineTransformation& trans):
	C3DTransformation::iterator_impl(pos,size), 
	_M_trans(trans), 
	_M_value_valid(false)
{
}
 
C3DTransformation::iterator_impl * C3DSplineTransformation::iterator_impl::clone() const
{
	return new C3DSplineTransformation::iterator_impl(get_pos(), get_size(), _M_trans); 
}

const C3DFVector&  C3DSplineTransformation::iterator_impl::do_get_value()const
{
	if (!_M_value_valid) {
		_M_value = C3DFVector(get_pos()) - _M_trans.on_grid(get_pos());
		_M_value_valid = true; 
	}
	return _M_value; 
}

void C3DSplineTransformation::iterator_impl::do_x_increment()
{
	_M_value_valid = false; 
}
void C3DSplineTransformation::iterator_impl::do_y_increment()
{
	_M_value_valid = false; 
}
void C3DSplineTransformation::iterator_impl::do_z_increment()
{
	_M_value_valid = false; 
}


double C3DSplineTransformation::get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const
{
	reinit(); 
	FUNCTION_NOT_TESTED; 	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C3DPPDivcurlMatrix(_M_coefficients.get_size(), 
							       C3DFVector(_M_range), 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), C3DFVector(_M_range), 
					 *_M_kernel, wd, wr); 
	
	
	return _M_divcurl_matrix->evaluate(_M_coefficients, gradient); 
}

double C3DSplineTransformation::get_divcurl_cost(double wd, double wr) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C3DPPDivcurlMatrix(_M_coefficients.get_size(), C3DFVector(_M_range), 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), C3DFVector(_M_range), 
					 *_M_kernel, wd, wr); 
	

	return *_M_divcurl_matrix * _M_coefficients; 
}


class C3DSplineTransformCreator: public C3DTransformCreator {
public:
	C3DSplineTransformCreator(EInterpolation ip, const C3DFVector& rates);
	virtual P3DTransformation do_create(const C3DBounds& size) const;
private:
	PBSplineKernel _M_kernel;
	C3DFVector _M_rates;
};

C3DSplineTransformCreator::C3DSplineTransformCreator(EInterpolation ip, const C3DFVector& rates):
	_M_rates(rates)
{

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
	assert(_M_kernel); 
	return P3DTransformation(new C3DSplineTransformation(size, _M_kernel, _M_rates));
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
};

C3DSplineTransformCreatorPlugin::C3DSplineTransformCreatorPlugin():
	C3DTransformCreatorPlugin("spline"),
	_M_ip(ip_bspline3),
	_M_rate(10)
{
	add_parameter("interp",
            new CDictParameter<EInterpolation>(_M_ip, GInterpolatorTable, "image interpolator"));
	add_parameter("rate",
		      new CFloatParameter(_M_rate, 1, numeric_limits<float>::max(), false,
					  "isotropic coefficient rate in pixels"));
}

C3DSplineTransformCreatorPlugin::ProductPtr
C3DSplineTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C3DSplineTransformCreator(_M_ip, C3DFVector(_M_rate, _M_rate, _M_rate)));
}

bool C3DSplineTransformCreatorPlugin::do_test() const
{
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

