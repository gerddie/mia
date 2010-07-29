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
#include <mia/core/scale1d.hh>
#include <mia/2d/transform/spline.hh>
#include <mia/2d/transformfactory.hh>

#include <boost/lambda/lambda.hpp>


NS_MIA_BEGIN
using namespace std;
using namespace boost::lambda;


C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range,
						 P2DInterpolatorFactory ipf):
	_M_range(range),
	_M_ipf(ipf),
	_M_scale(1.0, 1.0),
	_M_interpolator_valid(false)
{
	TRACE_FUNCTION;
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
}

C2DSplineTransformation::C2DSplineTransformation(const C2DSplineTransformation& org):
	_M_range(org._M_range),
	_M_coefficients( org._M_coefficients),
	_M_ipf(org._M_ipf),
	_M_interpolator_valid(false)
{

}

C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range, P2DInterpolatorFactory ipf, const C2DFVector& c_rate):
	_M_range(range),
	_M_ipf(ipf),
	_M_interpolator_valid(false)
{
	TRACE_FUNCTION;
	assert(_M_range.x > 0);
	assert(_M_range.y > 0);
	assert(c_rate.x >= 1.0);
	assert(c_rate.y >= 1.0);

	C2DBounds csize(size_t((range.x + c_rate.x - 1) / c_rate.x),
			size_t((range.y + c_rate.y - 1) / c_rate.y));
	_M_coefficients = C2DFVectorfield(csize);
	reinit();
}

void C2DSplineTransformation::set_coefficients(const C2DFVectorfield& field)
{
	TRACE_FUNCTION;
	_M_coefficients = field;
	_M_interpolator_valid = false;
}

void C2DSplineTransformation::reinit() const
{
	TRACE_FUNCTION;
	if (!_M_interpolator_valid) {
		TRACE("C2DSplineTransformation::reinit applies");
		_M_scale.x = float(_M_coefficients.get_size().x - 1) / (_M_range.x - 1);
		_M_scale.y = float(_M_coefficients.get_size().y - 1) / (_M_range.y - 1);
		_M_inv_scale.x = float(_M_range.x - 1) / (_M_coefficients.get_size().x - 1);
		_M_inv_scale.y = float(_M_range.y - 1) / (_M_coefficients.get_size().y - 1);
		_M_interpolator.reset(_M_ipf->create(_M_coefficients));
		_M_interpolator_valid = true;
		cvdebug() << "Scale: "<< _M_scale << "InvScale = " << _M_inv_scale << "\n";
	}
}

C2DFVector C2DSplineTransformation::apply ( const C2DFVector& x) const
{
	assert(_M_interpolator_valid && _M_interpolator);
	return (*_M_interpolator)(scale(x) );
}

C2DTransformation *C2DSplineTransformation::clone()const
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
	assert(_M_interpolator_valid && _M_interpolator);
	return x - (*_M_interpolator)(scale(x));
}

C2DFVector C2DSplineTransformation::scale( const C2DFVector& x) const
{
	assert(_M_interpolator_valid);
	return x * _M_scale;
}

const C2DBounds& C2DSplineTransformation::get_size() const
{
	return _M_range;
}

gsl::DoubleVector C2DSplineTransformation::get_parameters() const
{
	gsl::DoubleVector result(_M_coefficients.size() * 2);
	for(auto f = _M_coefficients.begin(), r = result.begin(); f != _M_coefficients.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
	return result;
}

void C2DSplineTransformation::set_parameters(const gsl::DoubleVector& params)
{
	assert(2 * _M_coefficients.size() == params.size());
	for(auto f = _M_coefficients.begin(), r = params.begin(); f != _M_coefficients.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
	}
}

bool C2DSplineTransformation::save(const std::string& /*filename*/,
				   const std::string& /*type*/) const
{
	TRACE_FUNCTION;
	assert(!"not implemented");
	return false;
}


C2DSplineTransformation *C2DSplineTransformation::refine(const C2DBounds& coeff_size) const
{
	TRACE_FUNCTION;
	C2DSplineTransformation *result =  new C2DSplineTransformation(_M_range, _M_ipf);
	reinit();

	C2DFVectorfield coeffs(coeff_size);

	C2DFVector dx((float)(_M_coefficients.get_size().x - 1) / (float)(coeff_size.x - 1),
		      (float)(_M_coefficients.get_size().y - 1) / (float)(coeff_size.y - 1) );

	C2DFVectorfield::iterator ic = coeffs.begin();

	for (size_t y = 0; y < coeff_size.y; ++y)
		for (size_t x = 0; x < coeff_size.x; ++x, ++ic) {
			*ic = (*_M_interpolator)(C2DFVector(dx.x * x, dx.y * y));
		}

	result->set_coefficients(coeffs);
	result->reinit();
	return result;
}


P2DTransformation C2DSplineTransformation::upscale(const C2DBounds& size) const
{
	TRACE_FUNCTION;
	C2DFVector mx((float)size.x / (float)_M_range.x     ,
		      (float)size.y / (float)_M_range.y);

	C2DBounds cmx((unsigned int)(mx.x * _M_coefficients.get_size().x),
		      (unsigned int)(mx.y * _M_coefficients.get_size().y));

	C2DSplineTransformation help(size, _M_ipf);
	help._M_coefficients = C2DFVectorfield(_M_coefficients.get_size());
	transform(_M_coefficients.begin(), _M_coefficients.end(),
		  help._M_coefficients.begin(), mx * _1 );


	return P2DTransformation(help.refine(cmx));
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
			C2DFVector v = C2DFVector(x,y) * _M_inv_scale;
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

C2DFMatrix C2DSplineTransformation::derivative_at(int x, int y) const
{
	TRACE_FUNCTION;
	assert(_M_interpolator_valid);
	const C2DFVector l = scale(C2DFVector(x,y));
	C2DFMatrix d = _M_interpolator->derivative_at(l);
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

C2DSplineTransformation::const_iterator C2DSplineTransformation::begin() const
{
	reinit();
	return const_iterator(*this, C2DBounds(0,0));
}

C2DSplineTransformation::const_iterator C2DSplineTransformation::end() const
{
	return const_iterator(*this, get_size());
}

void C2DSplineTransformation::translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	TRACE_FUNCTION;
	// downscale the field
	assert(params.size() == _M_coefficients.size() * 2);

	P1DInterpolatorFactory ipf(create_1dinterpolation_factory(ip_bspline4));
	C1DScalar scaler(ipf);

	C2DFVectorfield tmp(C2DBounds(gradient.get_size().x, _M_coefficients.get_size().y));

	vector<C2DFVector> in_buffer;
	vector<C2DFVector> out_buffer(_M_coefficients.get_size().y);

	for (size_t i = 0; i < gradient.get_size().x; ++i) {
		gradient.get_data_line_y(i, in_buffer);
		scaler(in_buffer, out_buffer);
		tmp.put_data_line_y(i, out_buffer);
	}
	out_buffer.resize(_M_coefficients.get_size().x);

	auto r = params.begin();

	for (size_t i = 0; i < tmp.get_size().y; ++i) {
		tmp.get_data_line_x(i, in_buffer);
		scaler(in_buffer, out_buffer);
		for(auto x = out_buffer.begin(); x != out_buffer.end(); ++x) {
			*r++ = x->x;
			*r++ = x->y;
		}
	}
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
			C2DFMatrix j = _M_interpolator->derivative_at(lx);
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
			C2DFMatrix J =  _M_interpolator->derivative_at(lx);
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


const C2DBounds& C2DSplineTransformation::get_coeff_size() const
{
	return _M_coefficients.get_size();
}

C2DSplineTransformation::const_iterator::const_iterator(
     const C2DSplineTransformation::const_iterator&  other):
	_M_trans(other._M_trans),
	_M_pos(other._M_pos),
	_M_value(other._M_value),
	_M_value_valid(other._M_value_valid)
{
}

C2DSplineTransformation::const_iterator::const_iterator(const C2DSplineTransformation& trans,
							const C2DBounds& pos):
	_M_trans(trans),
	_M_pos(pos),
	_M_value(0,0),
	_M_value_valid(false)
{
}


C2DSplineTransformation::const_iterator&
C2DSplineTransformation::const_iterator::operator ++()
{
	++_M_pos.x;
	if (_M_pos.x == _M_trans.get_size().x) {
		if (_M_pos.y < _M_trans.get_size().y) {
			++_M_pos.y;
			if (_M_pos.y < _M_trans.get_size().y)
				_M_pos.x = 0;
		}
	}
	_M_value_valid = false;
	return *this;
}


C2DSplineTransformation::const_iterator
C2DSplineTransformation::const_iterator::operator ++(int)
{
	C2DSplineTransformation::const_iterator old = *this;
	++(*this);
	return old;
}

void C2DSplineTransformation::const_iterator::update_value() const
{

	if (_M_pos != _M_trans.get_size())  {
		_M_value = C2DFVector(_M_pos) - _M_trans.apply(C2DFVector(_M_pos));
		_M_value_valid = true;
	}else {
		assert(!"C2DSplineTransformation::const_iterator: try to access outside range");
		throw logic_error("C2DSplineTransformation::const_iterator: try to access outside range");
	}
}

const C2DFVector& C2DSplineTransformation::const_iterator::operator *() const
{
	if (!_M_value_valid)
		update_value();
	return _M_value;
}

const C2DFVector *C2DSplineTransformation::const_iterator::operator ->() const
{
	if (!_M_value_valid)
		update_value();
	return &_M_value;
}

double C2DSplineTransformation::get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C2DPPDivcurlMatrix(_M_coefficients.get_size(), _M_range, 
							       *_M_ipf->get_kernel(), wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), _M_range, 
					 *_M_ipf->get_kernel(), wd, wr); 
	
	// this will throw ifthe interpolator is not of the right type
	const T2DConvoluteInterpolator<C2DFVector>& interp = 
		dynamic_cast<const T2DConvoluteInterpolator<C2DFVector>&>(*_M_interpolator); 
	
	return _M_divcurl_matrix->evaluate(interp.get_coefficients(), gradient); 
}

double C2DSplineTransformation::get_divcurl_cost(double wd, double wr) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!_M_divcurl_matrix) 
		_M_divcurl_matrix.reset(new C2DPPDivcurlMatrix(_M_coefficients.get_size(), _M_range, 
							       *_M_ipf->get_kernel(), wd, wr)); 
	else 
		_M_divcurl_matrix->reset(_M_coefficients.get_size(), _M_range, 
					 *_M_ipf->get_kernel(), wd, wr); 
	
	// this will throw ifthe interpolator is not of the right type
	const T2DConvoluteInterpolator<C2DFVector>& interp = 
		dynamic_cast<const T2DConvoluteInterpolator<C2DFVector>&>(*_M_interpolator); 
	
	return *_M_divcurl_matrix * interp.get_coefficients(); 
}


class C2DSplineTransformCreator: public C2DTransformCreator {
public:
	C2DSplineTransformCreator(EInterpolation ip, const C2DFVector& rates);
	virtual P2DTransformation do_create(const C2DBounds& size) const;
private:
	P2DInterpolatorFactory _M_ipf;
	C2DFVector _M_rates;
};

C2DSplineTransformCreator::C2DSplineTransformCreator(EInterpolation ip, const C2DFVector& rates):
	_M_rates(rates)
{
	_M_ipf.reset(create_2dinterpolation_factory(ip));
}


P2DTransformation C2DSplineTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DSplineTransformation(size, _M_ipf, _M_rates));
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

