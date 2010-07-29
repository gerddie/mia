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

#include <fstream>
#include <mia/core/msgstream.hh>
#include <mia/2d/transformfactory.hh>

#include <mia/2d/transform/affine.hh>


NS_MIA_BEGIN
using namespace boost::lambda;
using namespace std;

P2DImage C2DAffineTransformation::apply(const C2DImage& image,
					const C2DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C2DAffineTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform2d(image, ipf, *this);
}

C2DFVector C2DAffineTransformation::apply(const C2DFVector& x) const
{
	return transform(x);
}



C2DFVector C2DAffineTransformation::transform(const C2DFVector& x)const
{
	return C2DFVector(
		_M_t[0] * x.x + _M_t[1] * x.y + _M_t[2],
		_M_t[3] * x.x + _M_t[4] * x.y + _M_t[5]
			  );
}

C2DAffineTransformation::C2DAffineTransformation(const C2DBounds& size):
	_M_t(6,0.0f),
	_M_size(size)
{
	_M_t[0] = _M_t[4] = 1.0;
}

C2DAffineTransformation::C2DAffineTransformation(const C2DAffineTransformation& other):
	_M_t(other._M_t),
	_M_size(other._M_size)
{
}

C2DTransformation *C2DAffineTransformation::clone()const
{
	return new C2DAffineTransformation(*this);
}

C2DTransformation *C2DAffineTransformation::invert()const
{
	const double det = _M_t[0] * _M_t[4] - _M_t[1] * _M_t[3]; 
	if (fabs(det) < 1e-6) 
		THROW(invalid_argument, "C2DAffineTransformation::invert(): Matrix is singular"); 
	
	const double inv_det = 1.0 / det; 

	C2DAffineTransformation *result = new C2DAffineTransformation(*this);
	
	result->_M_t[0] = _M_t[4] * inv_det; 
	result->_M_t[1] = - _M_t[1] * inv_det; 
	result->_M_t[2] = (_M_t[1] * _M_t[5] - _M_t[2] * _M_t[4])* inv_det; 

	result->_M_t[3] = - _M_t[3] * inv_det; 
	result->_M_t[4] = _M_t[0] * inv_det; 
	result->_M_t[5] = (_M_t[2] * _M_t[3] - _M_t[0] * _M_t[5])* inv_det; 
		
	return result; 
}


C2DAffineTransformation::C2DAffineTransformation(const C2DBounds& size,
						 std::vector<double> transform):
	_M_t(transform),
	_M_size(size)
{
}

bool C2DAffineTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 2D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 6 ; ++i)
		file << _M_t[i] << " ";
	file << "\n";
	return file.good();
}

size_t C2DAffineTransformation::degrees_of_freedom() const
{
	return 6;
}

void C2DAffineTransformation::update(float /*step*/, const C2DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C2DAffineTransformation::scale(float x, float y)
{
	const double expx = exp(x);
	const double expy = exp(y);
	_M_t[0] *= expx;
	_M_t[1] *= expx;
	_M_t[2] *= expx;
	_M_t[3] *= expy;
	_M_t[4] *= expy;
	_M_t[5] *= expy;
}

void C2DAffineTransformation::translate(float x, float y)
{
	_M_t[2] +=  x;
	_M_t[5] +=  y;
}

void C2DAffineTransformation::rotate(float angle)
{
	const double sina = sin(angle);
	const double cosa = cos(angle);

	const double tx      = cosa * _M_t[2] - sina * _M_t[5];
	_M_t[5] = sina * _M_t[2] + cosa * _M_t[5];
	_M_t[2] = tx;

	const double a = _M_t[0] * cosa - sina * _M_t[3];
	const double b = _M_t[1] * cosa - sina * _M_t[4];
	const double c = _M_t[0] * sina + cosa * _M_t[3];
	const double d = _M_t[1] * sina + cosa * _M_t[4];

	_M_t[0] = a;
	_M_t[1] = b;
	_M_t[3] = c;
	_M_t[4] = d;

}


void C2DAffineTransformation::shear(float /*v*/)
{
	assert(0 && "not implemented");
}

gsl::DoubleVector C2DAffineTransformation::get_parameters() const
{
	gsl::DoubleVector result(degrees_of_freedom());
	copy(_M_t.begin(), _M_t.end(), result.begin());
	return result;
}

void C2DAffineTransformation::set_parameters(const gsl::DoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), _M_t.begin());

}

double C2DAffineTransformation::get_divcurl_cost(double, double, gsl::DoubleVector&) const
{
	return 0.0; 
}

double C2DAffineTransformation::get_divcurl_cost(double, double) const
{
	return 0.0; 
}


float C2DAffineTransformation::divergence() const
{
	return _M_t[0] + _M_t[1] + _M_t[3] + _M_t[4] - 2.0f;
}

float C2DAffineTransformation::grad_divergence() const
{
	return 0.0;
}


float C2DAffineTransformation::grad_curl() const
{
	return 0.0;
}


float C2DAffineTransformation::curl() const
{
	return _M_t[1] + _M_t[4] - _M_t[0] - _M_t[3];
}

const C2DBounds& C2DAffineTransformation::get_size() const
{
	return _M_size;
}

P2DTransformation C2DAffineTransformation::upscale(const C2DBounds& size) const
{
	float x_mult = float(size.x) / (float)get_size().x;
	float y_mult = float(size.y) / (float)get_size().y;

	C2DAffineTransformation *result = new C2DAffineTransformation(*this);
	result->_M_size = size;
	result->_M_t[3] *= x_mult;
	result->_M_t[5] *= y_mult;

	return P2DTransformation(result);
}

C2DFMatrix C2DAffineTransformation::derivative_at(int /*x*/, int /*y*/) const
{
	return C2DFMatrix(C2DFVector(_M_t[0], _M_t[1]),
			  C2DFVector(_M_t[3], _M_t[4]));
}

void C2DAffineTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	fill(_M_t.begin(), _M_t.end(), 0.0);
	_M_t[0] = _M_t[4] = 1.0;
}

float C2DAffineTransformation::get_max_transform() const
{
	// check the corners
	float m =      (C2DFVector(get_size()) -  apply(C2DFVector(get_size()))).norm2();
	float test0Y = (C2DFVector(0, get_size().y) - apply(C2DFVector(0, get_size().y))).norm2();
	float testX0 = (C2DFVector(get_size().x, 0) - apply(C2DFVector(get_size().x, 0))).norm2();
	float test00 = apply(C2DFVector(0, 0)).norm2();

	if (m < test0Y)
		m = test0Y;

	if (m < testX0)
		m = testX0;

	if (m < test00)
		m = test00;
	return sqrt(m);
}

void C2DAffineTransformation::add(const C2DTransformation& other)
{
	// *this  = other * *this
	const C2DAffineTransformation& a = dynamic_cast<const C2DAffineTransformation&>(other);

	vector<double> h(_M_t.size());

	h[0] = a._M_t[0] * _M_t[0] + a._M_t[1] * _M_t[3];
	h[1] = a._M_t[0] * _M_t[1] + a._M_t[1] * _M_t[4];
	h[2] = a._M_t[0] * _M_t[2] + a._M_t[1] * _M_t[5] + a._M_t[2];

	h[3] = a._M_t[3] * _M_t[0] + a._M_t[4] * _M_t[3];
	h[4] = a._M_t[3] * _M_t[1] + a._M_t[4] * _M_t[4];
	h[5] = a._M_t[3] * _M_t[2] + a._M_t[4] * _M_t[5] + a._M_t[5];

	copy(h.begin(), h.end(), _M_t.begin());
}

C2DAffineTransformation::const_iterator::const_iterator():
	_M_current(0.0f, 0.0f),
	_M_size(0.0f, 0.0f),
	_M_value(0.0f, 0.0f),
	_M_dx(0.0f, 0.0f)
{
}


C2DAffineTransformation::const_iterator& C2DAffineTransformation::const_iterator::operator ++()
{
	if (_M_trans) {
		++_M_current.x;
		if (_M_current.x < _M_size.x) {
			_M_value += _M_dx;
		} else {
			_M_current.x = 0;
			++_M_current.y;
			_M_value = _M_trans->transform(C2DFVector(_M_current));
			_M_dx = _M_trans->transform(C2DFVector(_M_current.x + 1.0, _M_current.y)) - _M_value;
		}
	}
	return *this;
}

C2DFVector C2DAffineTransformation::operator () (const C2DFVector& /*x*/) const
{
	assert(!"not implemented");
}

float C2DAffineTransformation::get_jacobian(const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C2DAffineTransformation::translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	assert(gradient.get_size() == _M_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t y = 0; y < _M_size.y; ++y) {
		for (size_t x = 0; x < _M_size.x; ++x, ++g) {
			r[0] += x * g->x;
			r[1] += y * g->x;
			r[2] += g->x;
			r[3] += x * g->y;
			r[4] += y * g->y;
			r[5] += g->y;
		}
	}
	const double f = 1.0 / gradient.size();
	std::transform(r.begin(), r.end(), params.begin(), _1 * f);
}

C2DAffineTransformation::const_iterator C2DAffineTransformation::const_iterator::operator ++(int)
{
	C2DAffineTransformation::const_iterator help(*this);
	++(*this);
	return help;
}

const C2DFVector C2DAffineTransformation::const_iterator::operator *() const
{
	return _M_value;
}


C2DAffineTransformation::const_iterator::const_iterator(const C2DBounds& pos, const C2DBounds& size, const C2DAffineTransformation *trans):
	_M_current(pos),
	_M_size(size),
	_M_trans(trans),
	_M_value(trans->transform(C2DFVector(pos)))
{
	_M_dx = _M_trans->transform(C2DFVector(pos.x + 1.0, pos.y)) - _M_value;
}

C2DAffineTransformation::const_iterator C2DAffineTransformation::begin() const
{
	return const_iterator(C2DBounds(0,0), get_size(), this);
}

C2DAffineTransformation::const_iterator C2DAffineTransformation::end() const
{
	return const_iterator(C2DBounds(0,get_size().y), get_size(), this);
}


float C2DAffineTransformation::pertuberate(C2DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

EXPORT_2D bool operator == (const C2DAffineTransformation::const_iterator& a,
		  const C2DAffineTransformation::const_iterator& b)
{
	return (a._M_current == b._M_current);
}

EXPORT_2D bool operator != (const C2DAffineTransformation::const_iterator& a,
		  const C2DAffineTransformation::const_iterator& b)
{
	return !(a._M_current == b._M_current);
}

class C2DAffineTransformCreator: public C2DTransformCreator {
	virtual P2DTransformation do_create(const C2DBounds& size) const;
};

P2DTransformation C2DAffineTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DAffineTransformation(size));
}

class C2DAffineTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DAffineTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C2DAffineTransformCreatorPlugin::C2DAffineTransformCreatorPlugin():
	C2DTransformCreatorPlugin("affine")
{
}

C2DAffineTransformCreatorPlugin::ProductPtr
C2DAffineTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C2DAffineTransformCreator());
}

bool C2DAffineTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DAffineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create affine transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DAffineTransformCreatorPlugin();
}


NS_MIA_END
