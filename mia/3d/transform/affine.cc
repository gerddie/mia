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
#include <cmath>
#include <mia/core/msgstream.hh>
#include <mia/3d/transformfactory.hh>

#include <mia/3d/transform/affine.hh>


NS_MIA_BEGIN
using namespace boost::lambda;
using namespace std;

P3DImage C3DAffineTransformation::apply(const C3DImage& image,
					const C3DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C3DAffineTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform3d(image, ipf, *this);
}

C3DFVector C3DAffineTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DAffineTransformation::transform(const C3DFVector& x)const
{
	return C3DFVector(
		_M_t[0] * x.x + _M_t[1] * x.y + _M_t[2] * x.z + _M_t[3],
		_M_t[4] * x.x + _M_t[5] * x.y + _M_t[6] * x.z + _M_t[7],
		_M_t[8] * x.x + _M_t[9] * x.y + _M_t[10] * x.z + _M_t[11]);
}

C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size):
	_M_t(12,0.0f),
	_M_size(size)
{
	_M_t[0] = _M_t[5] = _M_t[10] =  1.0;
}

C3DAffineTransformation::C3DAffineTransformation(const C3DAffineTransformation& other):
	_M_t(other._M_t),
	_M_size(other._M_size)
{
}

C3DTransformation *C3DAffineTransformation::do_clone()const
{
	return new C3DAffineTransformation(*this);
}

C3DTransformation *C3DAffineTransformation::invert()const
{
	const double det = 
		_M_t[10] * (_M_t[0] * _M_t[5] - _M_t[1] * _M_t[4]) + 
		_M_t[ 9] * (_M_t[2] * _M_t[4] - _M_t[0] * _M_t[6]) + 
		_M_t[ 8] * (_M_t[1] * _M_t[6] - _M_t[2] * _M_t[5]); 
	
	if (std::fabs(det) < 1e-8) 
		THROW(invalid_argument, "C3DAffineTransformation::invert(): Matrix is singular"); 
	
	const double inv_det = 1.0 / det; 

	C3DAffineTransformation *result = new C3DAffineTransformation(*this);

	const double h1625 =  _M_t[1]  * _M_t[6] - _M_t[2] * _M_t[5]; 
	const double h1735 =  _M_t[1]  * _M_t[7] - _M_t[3] * _M_t[5]; 
	const double h2736 =  _M_t[2]  * _M_t[7] - _M_t[3] * _M_t[6]; 
	
	const double h0624 =  _M_t[0]  * _M_t[6] - _M_t[2] * _M_t[4]; 
	const double h3407 =  _M_t[3]  * _M_t[4] - _M_t[0] * _M_t[7];
	const double h0514 =  _M_t[0]  * _M_t[5] - _M_t[1] * _M_t[4];
	
	
	result->_M_t[0] = ( _M_t[10] * _M_t[5] - _M_t[6] * _M_t[9])  * inv_det; 
	result->_M_t[1] = - ( _M_t[10] * _M_t[1] - _M_t[2] * _M_t[9])  * inv_det; 
	result->_M_t[2] = h1625  * inv_det; 
	result->_M_t[3] = -(h1625 *_M_t[11] - h1735 *_M_t[10] + h2736 *_M_t[9])  * inv_det;

	result->_M_t[4] = -( _M_t[10] * _M_t[4] - _M_t[6] * _M_t[8])  * inv_det; 
	result->_M_t[5] = ( _M_t[10] * _M_t[0] - _M_t[2] * _M_t[8])  * inv_det; 
	result->_M_t[6] = -h0624  * inv_det; 
	result->_M_t[7] = (h0624*_M_t[11] + h3407 *_M_t[10] + h2736 *_M_t[8])  * inv_det;

	result->_M_t[8]  = ( _M_t[4] * _M_t[9] - _M_t[5] * _M_t[8])  * inv_det; 
	result->_M_t[9]  = -( _M_t[0] * _M_t[9] - _M_t[1] * _M_t[8])  * inv_det; 
	result->_M_t[10] = h0514  * inv_det; 
	result->_M_t[11] = -(h0514*_M_t[11] + h3407 *_M_t[9] + h1735 *_M_t[8])  * inv_det;

	return result; 
}


C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size,
						 std::vector<double> transform):
	_M_t(transform),
	_M_size(size)
{
}

bool C3DAffineTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 3D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 12 ; ++i)
		file << _M_t[i] << " ";
	file << "\n";
	return file.good();
}

size_t C3DAffineTransformation::degrees_of_freedom() const
{
	return 12;
}

void C3DAffineTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C3DAffineTransformation::scale(float x, float y, float z)
{
	const double expx = exp(x);
	const double expy = exp(y);
	const double expz = exp(z);
	_M_t[0] *= expx;
	_M_t[1] *= expx;
	_M_t[2] *= expx;
	_M_t[3] *= expx;
	_M_t[4] *= expy;
	_M_t[5] *= expy;
	_M_t[6] *= expy;
	_M_t[7] *= expy;
	_M_t[8] *= expz;
	_M_t[9] *= expz;
	_M_t[10] *= expz;
	_M_t[11] *= expz;


}

void C3DAffineTransformation::translate(float x, float y, float z)
{
	_M_t[ 3] +=  x;
	_M_t[ 7] +=  y;
	_M_t[11] +=  z;
}

void C3DAffineTransformation::rotate(float angle)
{
	assert(0 && "Eliminate function"); 
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


void C3DAffineTransformation::shear(float /*v*/)
{
	assert(0 && "not implemented");
}

CDoubleVector C3DAffineTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	copy(_M_t.begin(), _M_t.end(), result.begin());
	return result;
}

void C3DAffineTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), _M_t.begin());

}

double C3DAffineTransformation::get_divcurl_cost(double, double, CDoubleVector&) const
{
	return 0.0; 
}

double C3DAffineTransformation::get_divcurl_cost(double, double) const
{
	return 0.0; 
}


float C3DAffineTransformation::divergence() const
{
	assert(0 && "not implemented");
	return _M_t[0] + _M_t[1] + _M_t[3] + _M_t[4] - 2.0f;
}

float C3DAffineTransformation::grad_divergence() const
{
	return 0.0;
}


float C3DAffineTransformation::grad_curl() const
{
	return 0.0;
}


float C3DAffineTransformation::curl() const
{
	assert(0 && "not implemented");
	return _M_t[1] + _M_t[4] - _M_t[0] - _M_t[3];
}

const C3DBounds& C3DAffineTransformation::get_size() const
{
	return _M_size;
}

P3DTransformation C3DAffineTransformation::do_upscale(const C3DBounds& size) const
{
	float x_mult = float(size.x) / (float)get_size().x;
	float y_mult = float(size.y) / (float)get_size().y;
	float z_mult = float(size.z) / (float)get_size().z;

	C3DAffineTransformation *result = new C3DAffineTransformation(*this);
	result->_M_size   = size;
	result->_M_t[3]  *= x_mult;
	result->_M_t[7]  *= y_mult;
	result->_M_t[11] *= z_mult;

	return P3DTransformation(result);
}

C3DFMatrix C3DAffineTransformation::derivative_at(int /*x*/, int /*y*/, int /*z*/) const
{
	return C3DFMatrix(C3DFVector(_M_t[0], _M_t[1], _M_t[2]),
			  C3DFVector(_M_t[4], _M_t[5], _M_t[6]),
			  C3DFVector(_M_t[8], _M_t[9], _M_t[10]));
}

void C3DAffineTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	fill(_M_t.begin(), _M_t.end(), 0.0);
	_M_t[0] = _M_t[5] = _M_t[10] = 1.0;
}

float C3DAffineTransformation::get_max_transform() const
{
	C3DFVector corners[7] = {
		C3DFVector(get_size().x, 0, 0), 
		C3DFVector(get_size().x, get_size().y,            0), 
		C3DFVector(           0, get_size().y,            0), 
		C3DFVector(           0, get_size().y, get_size().z), 
		C3DFVector(get_size().x,            0, get_size().z), 
		C3DFVector(           0,            0, get_size().z), 
		C3DFVector(get_size())
	};

	float result = apply(C3DFVector()).norm2(); 
	for(int i = 0; i < 7; ++i) {
		float h = (apply(corners[i]) - corners[i]).norm2(); 
		if (result < h) 
			result = h; 
	}

	return sqrt(result);
}

void C3DAffineTransformation::add(const C3DTransformation& other)
{
	// *this  = other * *this
	const C3DAffineTransformation& a = dynamic_cast<const C3DAffineTransformation&>(other);

	vector<double> h(_M_t.size());

	h[0] = a._M_t[0] * _M_t[0] + a._M_t[1] * _M_t[4] + a._M_t[2] * _M_t[8];
	h[1] = a._M_t[0] * _M_t[1] + a._M_t[1] * _M_t[5] + a._M_t[2] * _M_t[9];
	h[2] = a._M_t[0] * _M_t[2] + a._M_t[1] * _M_t[6] + a._M_t[2] * _M_t[10];
	h[3] = a._M_t[0] * _M_t[3] + a._M_t[1] * _M_t[7] + a._M_t[2] * _M_t[11] + a._M_t[3];

	h[4] = a._M_t[4] * _M_t[0] + a._M_t[5] * _M_t[4] + a._M_t[6] * _M_t[8];
	h[5] = a._M_t[4] * _M_t[1] + a._M_t[5] * _M_t[5] + a._M_t[6] * _M_t[9];
	h[6] = a._M_t[4] * _M_t[2] + a._M_t[5] * _M_t[6] + a._M_t[6] * _M_t[10];
	h[7] = a._M_t[4] * _M_t[3] + a._M_t[5] * _M_t[7] + a._M_t[6] * _M_t[11] + a._M_t[7];

	h[8]  = a._M_t[8] * _M_t[0] + a._M_t[9] * _M_t[4] + a._M_t[10] * _M_t[8];
	h[9]  = a._M_t[8] * _M_t[1] + a._M_t[9] * _M_t[5] + a._M_t[10] * _M_t[9];
	h[10] = a._M_t[8] * _M_t[2] + a._M_t[9] * _M_t[6] + a._M_t[10] * _M_t[10];
	h[11] = a._M_t[8] * _M_t[3] + a._M_t[9] * _M_t[7] + a._M_t[10] * _M_t[11] + a._M_t[11];

	copy(h.begin(), h.end(), _M_t.begin());
}


C3DFVector C3DAffineTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DAffineTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C3DAffineTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(gradient.get_size() == _M_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t z = 0; z < _M_size.z; ++z) {
		for (size_t y = 0; y < _M_size.y; ++y) {
			for (size_t x = 0; x < _M_size.x; ++x, ++g) {
				r[0] += x * g->x;
				r[1] += y * g->x;
				r[2] += z * g->x;
				r[3] += g->x;
				
				r[4] += x * g->y;
				r[5] += y * g->y;
				r[6] += z * g->y;
				r[7] += g->y;
				
				r[8]  += x * g->z;
				r[9]  += y * g->z;
				r[10] += z * g->z;
				r[11] += g->z;
				
			}
		}
	}
	std::copy(r.begin(), r.end(), params.begin());
}



C3DAffineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DAffineTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	_M_trans(trans), 
	_M_value(trans.apply(C3DFVector(pos)))
{
	_M_dx = _M_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - _M_value;
}

C3DTransformation::iterator_impl * C3DAffineTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), _M_trans); 
}

const C3DFVector&  C3DAffineTransformation::iterator_impl::do_get_value()const
{
	return _M_value; 
}

void C3DAffineTransformation::iterator_impl::do_x_increment()
{
	_M_value += _M_dx; 
}

void C3DAffineTransformation::iterator_impl::do_y_increment()
{
	_M_value = _M_trans.apply(C3DFVector(get_pos())); 
	_M_dx = _M_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - _M_value;
}

void C3DAffineTransformation::iterator_impl::do_z_increment()
{
	_M_value = _M_trans.apply(C3DFVector(get_pos())); 
	_M_dx = _M_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - _M_value;
}


C3DTransformation::const_iterator C3DAffineTransformation::begin() const
{
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAffineTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}


float C3DAffineTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

class C3DAffineTransformCreator: public C3DTransformCreator {
	virtual P3DTransformation do_create(const C3DBounds& size) const;
};

P3DTransformation C3DAffineTransformCreator::do_create(const C3DBounds& size) const
{
	return P3DTransformation(new C3DAffineTransformation(size));
}

class C3DAffineTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	typedef C3DTransformCreatorPlugin::ProductPtr ProductPtr;

	C3DAffineTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C3DAffineTransformCreatorPlugin::C3DAffineTransformCreatorPlugin():
	C3DTransformCreatorPlugin("affine")
{
}

C3DAffineTransformCreatorPlugin::ProductPtr
C3DAffineTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C3DAffineTransformCreator());
}

bool C3DAffineTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C3DAffineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create affine transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DAffineTransformCreatorPlugin();
}


NS_MIA_END
