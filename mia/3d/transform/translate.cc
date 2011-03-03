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

#include <numeric>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/transform/translate.hh>


NS_MIA_BEGIN
using namespace std;
C3DTranslateTransformation::C3DTranslateTransformation(const C3DBounds& size):
	_M_transform(0,0,0),
	_M_size(size)
{
}

C3DTranslateTransformation::C3DTranslateTransformation(const C3DBounds& size,const C3DFVector& transform):
	_M_transform(transform),
	_M_size(size)
{
}


void C3DTranslateTransformation::translate(float x, float y, float z)
{
	_M_transform.x += x;
	_M_transform.y += y;
	_M_transform.z += z;
}

C3DFVector C3DTranslateTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DTranslateTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
							 const C3DFVector& value):
	C3DTransformation::iterator_impl(pos, size), 
	_M_translate(value)
{
	_M_value = -1.0f * _M_translate; 
}

C3DTransformation::iterator_impl * C3DTranslateTransformation::iterator_impl::clone() const 
{
	return new iterator_impl(get_pos(), get_size(), _M_value); 
}

const C3DFVector&  C3DTranslateTransformation::iterator_impl::do_get_value() const 
{
	return _M_value; 
}

void C3DTranslateTransformation::iterator_impl::do_x_increment()
{
	_M_value = C3DFVector(get_pos()) - _M_translate; 
}

void C3DTranslateTransformation::iterator_impl::do_y_increment()
{
	_M_value = C3DFVector(get_pos()) - _M_translate; 
}

void C3DTranslateTransformation::iterator_impl::do_z_increment()
{
	_M_value = C3DFVector(get_pos()) - _M_translate; 
}

C3DTransformation::const_iterator C3DTranslateTransformation::begin() const
{
	
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), _M_size, _M_transform));
}

C3DTransformation::const_iterator C3DTranslateTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(_M_size, _M_size, _M_transform));
}


const C3DBounds& C3DTranslateTransformation::get_size() const
{
	return _M_size;
}

C3DTransformation *C3DTranslateTransformation::do_clone() const
{
	return new C3DTranslateTransformation(*this);
}

C3DTransformation *C3DTranslateTransformation::invert() const
{
	C3DTranslateTransformation *result = new C3DTranslateTransformation(*this);
	result->_M_transform.x = -_M_transform.x;
	result->_M_transform.y = -_M_transform.y;
	result->_M_transform.z = -_M_transform.z;
	return result; 
}

P3DImage C3DTranslateTransformation::apply(const C3DImage& image, const C3DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C3DTranslateTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform3d(image, ipf, *this);
}

bool C3DTranslateTransformation::save(const std::string& /*filename*/, const std::string& /*type*/) const
{
	assert(0 && "not implemented");

}

P3DTransformation C3DTranslateTransformation::do_upscale(const C3DBounds& size) const
{

	return P3DTransformation(new C3DTranslateTransformation(size,
								C3DFVector((_M_transform.x * size.x) / _M_size.x,
									   (_M_transform.y * size.y) / _M_size.y,
									   (_M_transform.z * size.z) / _M_size.z)));
}


void C3DTranslateTransformation::add(const C3DTransformation& /*a*/)
{
	assert(0 && "not implemented");
}

void C3DTranslateTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(gradient.get_size() == _M_size);
	assert(params.size() == 3);
	C3DFVector r = accumulate(gradient.begin(), gradient.end(), C3DFVector(0,0,0)); 
	params[0] = -r.x;
	params[1] = -r.y;
	params[2] = -r.z; 
}


size_t C3DTranslateTransformation::degrees_of_freedom() const
{
	return 3;
}

void C3DTranslateTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(0 && "not implemented");
}

C3DFMatrix C3DTranslateTransformation::derivative_at(int /*x*/, int /*y*/, int /*z*/) const
{
	return C3DFMatrix();
}

CDoubleVector C3DTranslateTransformation::get_parameters() const
{
	CDoubleVector result(3);
	result[0] = _M_transform.x;
	result[1] = _M_transform.y;
	result[2] = _M_transform.z;
	return result;
}

void C3DTranslateTransformation::set_parameters(const CDoubleVector& params)
{
	assert(params.size() == 3);
	_M_transform.x = params[0];
	_M_transform.y = params[1];
	_M_transform.z = params[2];
}

void C3DTranslateTransformation::set_identity()
{
	_M_transform.x = _M_transform.y = _M_transform.z = 0.0;
}

float C3DTranslateTransformation::get_max_transform() const
{
	return _M_transform.norm();
}

float C3DTranslateTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	return 0.0;
}

C3DFVector C3DTranslateTransformation::operator () (const C3DFVector& x) const
{
	return x - _M_transform;
}

float C3DTranslateTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	return 0.0;
}

C3DFVector C3DTranslateTransformation::transform(const C3DFVector& x)const
{
	return x + _M_transform;
}

float C3DTranslateTransformation::divergence() const
{
	return 0.0;
}

float C3DTranslateTransformation::curl() const
{
	return 0.0;
}

float C3DTranslateTransformation::grad_divergence() const
{
	return 0.0;
}

float C3DTranslateTransformation::grad_curl() const
{
	return 0.0;
}

double C3DTranslateTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/, 
						    CDoubleVector& /*gradient*/) const
{
	return 0.0;
}

double C3DTranslateTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/) const
{
	return 0.0;
}


class C3DTranslateTransformCreator: public C3DTransformCreator {
	virtual P3DTransformation do_create(const C3DBounds& size) const;
};

P3DTransformation C3DTranslateTransformCreator::do_create(const C3DBounds& size) const
{
	return P3DTransformation(new C3DTranslateTransformation(size));
}

class C3DTranslateTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	typedef C3DTransformCreatorPlugin::ProductPtr ProductPtr;

	C3DTranslateTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C3DTranslateTransformCreatorPlugin::C3DTranslateTransformCreatorPlugin():
	C3DTransformCreatorPlugin("translate")
{
}

C3DTranslateTransformCreatorPlugin::ProductPtr
C3DTranslateTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C3DTranslateTransformCreator());
}

bool C3DTranslateTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C3DTranslateTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create a translate only transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DTranslateTransformCreatorPlugin();
}


NS_MIA_END

