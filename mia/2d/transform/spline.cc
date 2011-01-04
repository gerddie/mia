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
#include <mia/2d/transform/spline.hh>
#include <mia/2d/transformfactory.hh>

#include <boost/lambda/lambda.hpp>


NS_MIA_BEGIN
using namespace std;
using namespace boost::lambda;


C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range, PBSplineKernel kernel):
	_M_range(range),
	_M_target_c_rate(1,1),
	_M_kernel(kernel),
	_M_shift(0), 
	_M_enlarge(0), 
	_M_scale(1.0, 1.0),
	_M_interpolator_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y), 
	_M_grid_valid(false)

{
	
	_M_shift = _M_kernel->get_active_halfrange() - 1; 
	_M_enlarge = 2*_M_shift; 
	
	TRACE_FUNCTION;
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
}

C2DSplineTransformation::C2DSplineTransformation(const C2DSplineTransformation& org):
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
	_M_grid_valid(false)
{
}

C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range, PBSplineKernel kernel, 
						 const C2DFVector& c_rate):
	_M_range(range),
	_M_target_c_rate(c_rate),
	_M_kernel(kernel),
	_M_interpolator_valid(false),
	_M_x_weights(_M_range.x), 
	_M_x_indices(_M_range.x), 
	_M_y_weights(_M_range.y),
	_M_y_indices(_M_range.y),
	_M_grid_valid(false)
{
	TRACE_FUNCTION;
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
	assert(c_rate.x >= 1.0);
	assert(c_rate.y >= 1.0);
	assert(kernel); 

	
	_M_shift = _M_kernel->get_active_halfrange() - 1; 
	_M_enlarge = 2*_M_shift; 

	C2DBounds csize(size_t((range.x + c_rate.x - 1) / c_rate.x) + _M_enlarge,
			size_t((range.y + c_rate.y - 1) / c_rate.y) + _M_enlarge);
	_M_coefficients = C2DFVectorfield(csize);
	reinit();
}

void C2DSplineTransformation::set_coefficients(const C2DFVectorfield& field)
{
	TRACE_FUNCTION;
	_M_coefficients = field;
	_M_interpolator_valid = false;

	_M_target_c_rate.x = float(_M_range.x) / (field.get_size().x - _M_enlarge);
	_M_target_c_rate.y = float(_M_range.y) / (field.get_size().y - _M_enlarge);
}

void C2DSplineTransformation::reinit() const
{
	TRACE_FUNCTION;
	if (!_M_interpolator_valid) {
		TRACE("C2DSplineTransformation::reinit applies");
		_M_scale.x = float(_M_coefficients.get_size().x - 1 - _M_enlarge) / (_M_range.x - 1);
		_M_scale.y = float(_M_coefficients.get_size().y - 1 - _M_enlarge) / (_M_range.y - 1);
		_M_inv_scale.x = float(_M_range.x - 1) / (_M_coefficients.get_size().x - 1 - _M_enlarge);
		_M_inv_scale.y = float(_M_range.y - 1) / (_M_coefficients.get_size().y - 1 - _M_enlarge);
		_M_interpolator_valid = true;

	}
}

C2DFVector C2DSplineTransformation::interpolate(const C2DFVector& x) const 
{
	std::vector<double> xweights(_M_kernel->size()); 
	std::vector<double> yweights(_M_kernel->size()); 
	size_t startx = _M_kernel->get_start_idx_and_value_weights(x.x, xweights); 
	size_t y = _M_kernel->get_start_idx_and_value_weights(x.y, yweights); 
	
	C2DFVector result; 
	for(auto wy = yweights.begin(); y < _M_coefficients.get_size().y; ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		for(auto wx = xweights.begin(), cx = _M_coefficients.begin_at(startx,y); 
		    x < _M_coefficients.get_size().x; ++x, ++wx)  {
			h += *wx * *cx; 
		}
		result += h * *wy; 
	}
	return result; 
}

C2DFVector C2DSplineTransformation::apply(const C2DFVector& x) const
{
	assert(_M_interpolator_valid);
	const C2DFVector s = scale(x); 
	const C2DFVector result = interpolate(scale(x));
	return result; 
}

C2DTransformation *C2DSplineTransformation::do_clone()const
{
	return new C2DSplineTransformation(*this);
}

C2DTransformation *C2DSplineTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C2DSplineTransformation(*this);
}


C2DFVector C2DSplineTransformation::operator () (const C2DFVector& x) const
{
	return x - apply(x);
}

C2DFVector C2DSplineTransformation::scale( const C2DFVector& x) const
{
	assert(_M_interpolator_valid);
	return x * _M_scale + C2DFVector(_M_shift,_M_shift);
}

const C2DBounds& C2DSplineTransformation::get_size() const
{
	return _M_range;
}

gsl::DoubleVector C2DSplineTransformation::get_parameters() const
{
	TRACE_FUNCTION;
	gsl::DoubleVector result(_M_coefficients.size() * 2);
	for(auto f = _M_coefficients.begin(), r = result.begin(); f != _M_coefficients.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
	return result;
}

void C2DSplineTransformation::set_parameters(const gsl::DoubleVector& params)
{
	TRACE_FUNCTION;
	assert(2 * _M_coefficients.size() == params.size());
	for(auto f = _M_coefficients.begin(), r = params.begin(); f != _M_coefficients.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
	}
	_M_interpolator_valid = false; 
}

bool C2DSplineTransformation::save(const std::string& /*filename*/,
				   const std::string& /*type*/) const
{
	TRACE_FUNCTION;
	assert(!"not implemented");
	return false;
}


bool C2DSplineTransformation::refine()
{
	TRACE_FUNCTION;
	C2DBounds csize(size_t((_M_range.x + _M_target_c_rate.x - 1) / _M_target_c_rate.x) + _M_enlarge,
			size_t((_M_range.y + _M_target_c_rate.y - 1) / _M_target_c_rate.y) + _M_enlarge);
	
	// no refinement necessary? 
	if (csize.x <= _M_coefficients.get_size().x || 
	    csize.y <= _M_coefficients.get_size().y) 
		return false; 

	// now interpolate the new coefficients 
	// \todo this should be done faster by a filter 
	reinit();
	C2DFVectorfield coeffs(csize);
	C2DFVector dx((float)(_M_coefficients.get_size().x - 1 - _M_enlarge) / (float)(csize.x - 1 - _M_enlarge),
		      (float)(_M_coefficients.get_size().y - 1 - _M_enlarge) / (float)(csize.y - 1 - _M_enlarge));

	C2DFVectorfield::iterator ic = coeffs.begin();

	for (size_t y = 0; y < csize.y; ++y)
		for (size_t x = 0; x < csize.x; ++x, ++ic) {
			*ic = interpolate(C2DFVector(dx.x * (float(x) - _M_shift)+ _M_shift, 
						     dx.y * (float(y) - _M_shift)+ _M_shift));
		}

	set_coefficients(coeffs);
	reinit();
	_M_grid_valid = false; 
	return true; 
}


P2DTransformation C2DSplineTransformation::upscale(const C2DBounds& size) const
{
	TRACE_FUNCTION;
	C2DFVector mx(((float)size.x - 1.0)/ ((float)_M_range.x - 1.0),
		      ((float)size.y - 1.0)/ ((float)_M_range.y - 1.0));

	C2DSplineTransformation *help = new C2DSplineTransformation(size, _M_kernel);
	C2DFVectorfield new_coefs(_M_coefficients.get_size()); 
	
	transform(_M_coefficients.begin(), _M_coefficients.end(), new_coefs.begin(), mx * _1 );
	help->set_coefficients(new_coefs); 
	help->_M_target_c_rate = _M_target_c_rate; 
	return P2DTransformation(help);
}

void C2DSplineTransformation::add(const C2DTransformation& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == get_size());

	reinit();
	a.reinit();

	C2DFVectorfield::iterator i = _M_coefficients.begin();

	for (size_t y = 0; y < _M_coefficients.get_size().y; ++y)  {
		for (size_t x = 0; x < _M_coefficients.get_size().x; ++x, ++i)  {
			C2DFVector v = C2DFVector(x - _M_shift, y - _M_shift) * _M_inv_scale;
			C2DFVector u = a(v);
			*i = v + apply(u) - u;
		}
	}

	_M_interpolator_valid = false;
}

size_t C2DSplineTransformation::degrees_of_freedom() const
{
	return _M_coefficients.size() * 2;
}

P2DImage C2DSplineTransformation::apply(const C2DImage& image,
					const C2DInterpolatorFactory& ipf) const
{
	TRACE_FUNCTION;
	return transform2d(image, ipf, *this);
}

void C2DSplineTransformation::update(float step, const C2DFVectorfield& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == _M_coefficients.get_size());
	C2DFVectorfield::iterator ci = _M_coefficients.begin();
	C2DFVectorfield::iterator ce = _M_coefficients.end();
	C2DFVectorfield::const_iterator ai = a.begin();

	while( ci != ce ) {
		*ci++ += step * *ai++;
	}
	_M_interpolator_valid = false;
}

C2DFMatrix C2DSplineTransformation::derivative_at(const C2DFVector& x) const
{
	std::vector<double> xweights(_M_kernel->size()); 
	std::vector<double> yweights(_M_kernel->size()); 
	size_t startx = _M_kernel->get_start_idx_and_value_weights(x.x, xweights); 
	size_t y = _M_kernel->get_start_idx_and_derivative_weights(x.y, yweights); 
	
	C2DFMatrix result; 
	for(auto wy = yweights.begin(); y < _M_coefficients.get_size().y; ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		for(auto wx = xweights.begin(), cx = _M_coefficients.begin_at(startx,y); 
		    x < _M_coefficients.get_size().x; ++x, ++wx)  {
			h += *wx * *cx; 
		}
		result.x += h * *wy; 
	}
	startx = _M_kernel->get_start_idx_and_derivative_weights(x.x, xweights); 
	y = _M_kernel->get_start_idx_and_value_weights(x.y, yweights); 

	for(auto wy = yweights.begin(); y < _M_coefficients.get_size().y; ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		for(auto wx = xweights.begin(), cx = _M_coefficients.begin_at(startx,y); 
		    x < _M_coefficients.get_size().x; ++x, ++wx)  {
			h += *wx * *cx; 
		}
		result.y += h * *wy; 
	}
	return result; 
	
}

C2DFMatrix C2DSplineTransformation::derivative_at(int x, int y) const
{
	TRACE_FUNCTION;
	assert(_M_interpolator_valid);
	const C2DFVector l = scale(C2DFVector(x,y));
	C2DFMatrix d = derivative_at(l);
	cvinfo() << C2DFVector(x,y) << ":" << l << " = [" <<  d.x << d.y << "]\n"; 
	d.x.x = 1.0f - d.x.x * _M_scale.x;
	d.x.y =      - d.x.y * _M_scale.x;
	d.y.x =      - d.y.x * _M_scale.y;
	d.y.y = 1.0f - d.y.y * _M_scale.y;
	return d;
}

void C2DSplineTransformation::set_identity()
{
	TRACE_FUNCTION;
	fill(_M_coefficients.begin(), _M_coefficients.end(), C2DFVector(0,0));
	_M_interpolator_valid = false;
}

float C2DSplineTransformation::get_max_transform() const
{
	TRACE_FUNCTION;
	// assuming the maximum spline coef is equal
	C2DFVectorfield::const_iterator i = _M_coefficients.begin();
	C2DFVectorfield::const_iterator e = _M_coefficients.end();

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

void C2DSplineTransformation::init_grid()const
{
	TRACE_FUNCTION; 
	if (!_M_grid_valid) {
		cvdebug() << "really run C2DSplineTransformation::init_grid\n"; 
		// pre-evaluateof fixed-grid coefficients
		size_t n_elms = _M_kernel->size(); 
		std::vector<int> indices(n_elms); 
		std::vector<double> weights(n_elms); 
		const C2DBounds& csize = _M_coefficients.get_size(); 
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

		_M_grid_valid = true; 
	}
}

C2DTransformation::const_iterator C2DSplineTransformation::begin() const
{
	reinit();
	init_grid(); 
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), get_size(), *this));
}

C2DTransformation::const_iterator C2DSplineTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this));
}

struct FCopyX {
	double operator() (const C2DFVector& x) const {
		return x.x; 
	}
}; 

struct FCopyY {
	double operator() (const C2DFVector& x) const {
		return x.y; 
	}
}; 

// quadratic extrapolation 
static double extrapolate(double x, double ym, double y0, double yp) 
{
	const double c = y0; 
	const double b = 0.5 * (yp - ym); 
	const double a = ym + b - c; 
	return a * x * x + b * x + c; 
}

void C2DSplineTransformation::run_downscaler(C1DScalarFixed& scaler, vector<double>& out_buffer) const 
{
	const int size = out_buffer.size() - _M_shift; 
	scaler.run(); 
	copy(scaler.output_begin(), scaler.output_end(), out_buffer.begin() + _M_shift); 
	
	// continue the gradient a both sides with quatratic extrapolation 
	for(int j = 0; j < _M_shift; ++j) {
		out_buffer[_M_shift - j - 1] = extrapolate(-2, out_buffer[_M_shift - j], 
							   out_buffer[_M_shift - j + 1], 
							   out_buffer[_M_shift - j + 2]); 
		out_buffer[size + j] = extrapolate(2, out_buffer[size + j - 3], 
						   out_buffer[size + j - 2], out_buffer[size + j - 1]); 
	}
}

void C2DSplineTransformation::convolute(vector<C2DFVector>& output, 
		      const vector<C2DFVector>& input,  const vector<double>& kernel) const 
{
	const int khsize = kernel.size() / 2; 
	for(size_t o = 0; o < output.size(); ++o) {
		int istart = (o * (input.size() - 1)) /(output.size() - _M_enlarge- 1)  - khsize; 
		auto kb = kernel.begin(); 
		auto ke = kernel.end(); 
		if (istart < 0) {
			ke += istart;
			istart = 0; 
		}
		int iend = (int)input.size() - istart - kernel.size(); 
		if (iend < 0)
			kb -= iend; 

		cvdebug() <<"[" << istart << "," << iend << "] "; 
		output[o] = C2DFVector(); 
		for(; kb != ke; ++istart, ++kb) {
			output[o] += *kb * input[istart]; 
		}
		cverb << o << " = " << output[o] << "\n"; 
	}
}


void C2DSplineTransformation::translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	TRACE_FUNCTION;
	assert(params.size() == _M_coefficients.size() * 2);
	assert(gradient.get_size() == _M_range); 

#if 1
	// y_matrix todo: pre-calculate 
	vector<vector<float> > my(_M_coefficients.get_size().y, vector<float>(gradient.get_size().y)); 
	for(size_t i = 0; i < gradient.get_size().y; ++i) {
		double x = i * _M_scale.y + _M_shift; 
		for(size_t o = 0; o < _M_coefficients.get_size().y; ++o) {
			my[o][i] = _M_kernel->get_weight_at(x - o, 0); 
		}
	}

	for(size_t o = 0; o < _M_coefficients.get_size().y; ++o) {
		cvdebug() << o << ":" << my[o] << "\n"; 
	}


	C2DFVectorfield tmp(C2DBounds(gradient.get_size().x, _M_coefficients.get_size().y));
	vector<C2DFVector> in_buffer(gradient.get_size().y); 
	vector<C2DFVector> out_buffer(_M_coefficients.get_size().y); 
	
	for (size_t ix = 0; ix < gradient.get_size().x; ++ix) {
		gradient.get_data_line_y(ix, in_buffer);
		for(size_t i = 0; i < _M_coefficients.get_size().y; ++i) {
			out_buffer[i] = inner_product(in_buffer.begin(), in_buffer.end(), 
						      my[i].begin(), C2DFVector());
						   
		}
		tmp.put_data_line_y(ix, out_buffer);
	}


	// x_matrix  todo: pre-calculate 
	vector<vector<float> > mx(_M_coefficients.get_size().x, vector<float>(gradient.get_size().x)); 
	for(size_t i = 0; i < gradient.get_size().x; ++i) {
		double x = i * _M_scale.x + _M_shift; 
		for(size_t o = 0; o < _M_coefficients.get_size().x; ++o) {
			mx[o][i] = _M_kernel->get_weight_at(x - o, 0); 
		}
	}
	for(size_t o = 0; o < _M_coefficients.get_size().x; ++o) {
		cvdebug() << o << ":" << mx[o] << "\n"; 
	}


	in_buffer.resize(gradient.get_size().x); 
	out_buffer.resize(_M_coefficients.get_size().x); 


	// x convolution and copy to output
	auto r = params.begin(); 
	for (size_t iy = 0; iy < _M_coefficients.get_size().y; ++iy) {
		tmp.get_data_line_x(iy, in_buffer);
		
		for(size_t i = 0; i < _M_coefficients.get_size().x; ++i) {
			out_buffer[i] = inner_product(in_buffer.begin(), in_buffer.end(), 
						      mx[i].begin(), C2DFVector());
						   
		}
		cvdebug() << "Y:" << iy << ":" << out_buffer << "\n"; 
		for(auto v = out_buffer.begin();  v != out_buffer.end(); ++v, r+=2) {
			cvdebug() << *v << "\n"; 
			r[0] = -v->x; 
			r[1] = -v->y; 
		}
	}
#else // this is the slow road 
	auto r = params.begin(); 
	for (size_t yo = 0; yo < _M_coefficients.get_size().y; ++yo)
		for (size_t xo = 0; xo < _M_coefficients.get_size().x; ++xo, r+=2) {
			auto ig = gradient.begin(); 
			for (size_t yi = 0; yi < gradient.get_size().y; ++yi) 
				for (size_t xi = 0; xi < gradient.get_size().x; ++xi, ++ig) {
					C2DFVector x = scale(C2DFVector(xi,yi)); 
					double v = _M_ipf->get_kernel()->get_weight_at(x.x - xo, 0) * 
						_M_ipf->get_kernel()->get_weight_at(x.y - yo, 0); 
					r[0] -= v * ig->x; 
					r[1] -= v * ig->y; 
				}
		}
	
			

#endif
}

float  C2DSplineTransformation::pertuberate(C2DFVectorfield& v) const
{
	TRACE_FUNCTION;
	reinit();
	C2DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	C2DFVector lx_max(0,0);
	for (size_t y = 0; y < v.get_size().y; ++y)
		for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
			C2DFVector lx(x,y);
			C2DFMatrix j = derivative_at(lx);
			j.x.x = j.x.x * _M_scale.x;
			j.x.y = j.x.y * _M_scale.x;
			j.y.x = j.y.x * _M_scale.y;
			j.y.y = j.y.y * _M_scale.y;

			const C2DFVector u = j *  *iv;
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

float C2DSplineTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_coeff_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();
	delta *= 0.5;

	C2DFVector lx_min(0,0);
	for(size_t y = 1; y < v.get_size().y - 1; ++y) {
		C2DFVectorfield::const_iterator iv = v.begin_at(1,y);
		for(size_t x = 1; x < v.get_size().x - 1; ++x, ++iv) {
			const C2DFVector lx(x,y);
			C2DFMatrix J =  derivative_at(lx);
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
	cvdebug() << lx_min << "\n";
	return j_min * _M_scale.y * _M_scale.x;
}

C2DFVector C2DSplineTransformation::on_grid(const C2DBounds& x) const
{
	assert(x.x < _M_range.x); 
	assert(x.y < _M_range.y); 
	assert(_M_grid_valid); 
	assert(_M_interpolator_valid);

	C2DFVector result; 
	for(size_t y = 0; y < _M_y_weights[x.y].size(); ++y) {
		C2DFVector hr; 
		auto ic = _M_coefficients.begin_at(0, _M_y_indices[x.y][y]); 
		for(size_t ix = 0; ix < _M_x_weights[x.x].size(); ++ix) {
			hr += _M_x_weights[x.x][ix] * ic[_M_x_indices[x.x][ix]]; 
		}
		result += _M_y_weights[x.y][y] * hr; 
	}
	return result; 
}

const C2DBounds& C2DSplineTransformation::get_coeff_size() const
{
	return _M_coefficients.get_size();
}


C2DSplineTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						      const C2DSplineTransformation& trans):
	C2DTransformation::iterator_impl(pos,size), 
	_M_trans(trans), 
	_M_value_valid(false)
{
}
 
C2DTransformation::iterator_impl * C2DSplineTransformation::iterator_impl::clone() const
{
	return new C2DSplineTransformation::iterator_impl(get_pos(), get_size(), _M_trans); 
}

const C2DFVector&  C2DSplineTransformation::iterator_impl::do_get_value()const
{
	if (!_M_value_valid) {
		_M_value = C2DFVector(get_pos()) - _M_trans.on_grid(get_pos());
		_M_value_valid = true; 
	}
	return _M_value; 
}

void C2DSplineTransformation::iterator_impl::do_x_increment()
{
	_M_value_valid = false; 
}
void C2DSplineTransformation::iterator_impl::do_y_increment()
{
	_M_value_valid = false; 
}


double C2DSplineTransformation::get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C2DPPDivcurlMatrix(_M_coefficients.get_size(), _M_range, 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), _M_range, 
					 *_M_kernel, wd, wr); 
	
	
	return _M_divcurl_matrix->evaluate(_M_coefficients, gradient); 
}

double C2DSplineTransformation::get_divcurl_cost(double wd, double wr) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C2DPPDivcurlMatrix(_M_coefficients.get_size(), _M_range, 
							       *_M_kernel, wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), _M_range, 
					 *_M_kernel, wd, wr); 
	

	return *_M_divcurl_matrix * _M_coefficients; 
}


class C2DSplineTransformCreator: public C2DTransformCreator {
public:
	C2DSplineTransformCreator(EInterpolation ip, const C2DFVector& rates);
	virtual P2DTransformation do_create(const C2DBounds& size) const;
private:
	PBSplineKernel _M_kernel;
	C2DFVector _M_rates;
};

C2DSplineTransformCreator::C2DSplineTransformCreator(EInterpolation ip, const C2DFVector& rates):
	_M_rates(rates)
{

	switch (ip){
	case ip_bspline2:
		_M_kernel.reset(new CBSplineKernel2());
		break;
	case ip_bspline3:
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


P2DTransformation C2DSplineTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DSplineTransformation(size, _M_kernel, _M_rates));
}


class C2DSplineTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DSplineTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
private:
	EInterpolation _M_ip;
	float _M_rate;
};

C2DSplineTransformCreatorPlugin::C2DSplineTransformCreatorPlugin():
	C2DTransformCreatorPlugin("spline"),
	_M_ip(ip_bspline3),
	_M_rate(10)
{
	add_parameter("interp",
            new CDictParameter<EInterpolation>(_M_ip, GInterpolatorTable, "image interpolator"));
	add_parameter("rate",
		      new CFloatParameter(_M_rate, 1, numeric_limits<float>::max(), false,
					  "isotropic coefficient rate in pixels"));
}

C2DSplineTransformCreatorPlugin::ProductPtr
C2DSplineTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C2DSplineTransformCreator(_M_ip, C2DFVector(_M_rate, _M_rate)));
}

bool C2DSplineTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DSplineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create spline transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSplineTransformCreatorPlugin();
}

NS_MIA_END

