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
#include <mia/2d/transformfactory.hh>
#include <mia/2d/transform/translate.hh>


NS_MIA_BEGIN
using namespace std;
C2DTranslateTransformation::C2DTranslateTransformation(const C2DBounds& size):
	_M_transform(0,0),
	_M_size(size)
{
}

C2DTranslateTransformation::C2DTranslateTransformation(const C2DBounds& size,const C2DFVector& transform):
	_M_transform(transform),
	_M_size(size)
{
}


void C2DTranslateTransformation::translate(float x, float y)
{
	_M_transform.x += x;
	_M_transform.y += y;
}

C2DFVector C2DTranslateTransformation::apply(const C2DFVector& x) const
{
	return transform(x);
}



C2DTranslateTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
							 const C2DFVector& value):
	C2DTransformation::iterator_impl(pos, size), 
	_M_translate(value)
{
	_M_value = -1.0f * _M_translate; 
}

C2DTransformation::iterator_impl * C2DTranslateTransformation::iterator_impl::clone() const 
{
	return new iterator_impl(get_pos(), get_size(), _M_value); 
}

const C2DFVector&  C2DTranslateTransformation::iterator_impl::do_get_value() const 
{
	return _M_value; 
}

void C2DTranslateTransformation::iterator_impl::do_x_increment()
{
	_M_value = C2DFVector(get_pos()) - _M_translate; 
}

void C2DTranslateTransformation::iterator_impl::do_y_increment()
{
	_M_value = C2DFVector(get_pos()) - _M_translate; 
}

C2DTransformation::const_iterator C2DTranslateTransformation::begin() const
{
	
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), _M_size, _M_transform));
}

C2DTransformation::const_iterator C2DTranslateTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(_M_size, _M_size, _M_transform));
}


const C2DBounds& C2DTranslateTransformation::get_size() const
{
	return _M_size;
}

C2DTransformation *C2DTranslateTransformation::do_clone() const
{
	return new C2DTranslateTransformation(*this);
}

C2DTransformation *C2DTranslateTransformation::invert() const
{
	C2DTranslateTransformation *result = new C2DTranslateTransformation(*this);
	result->_M_transform.x = -_M_transform.x;
	result->_M_transform.y = -_M_transform.y;
	return result; 
}

P2DImage C2DTranslateTransformation::apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C2DTranslateTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform2d(image, ipf, *this);
}

bool C2DTranslateTransformation::save(const std::string& filename, const std::string& type) const
{
	assert(0 && "not implemented");

}

P2DTransformation C2DTranslateTransformation::upscale(const C2DBounds& size) const
{

	return P2DTransformation(new C2DTranslateTransformation(size,
								C2DFVector((_M_transform.x * size.x) / _M_size.x,
									   (_M_transform.y * size.y) / _M_size.y)));
}


void C2DTranslateTransformation::add(const C2DTransformation& a)
{
	assert(0 && "not implemented");
}

void C2DTranslateTransformation::translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	assert(gradient.get_size() == _M_size);
	assert(params.size() == 2);
	C2DFVector r = accumulate(gradient.begin(), gradient.end(), C2DFVector(0,0));
	params[0] = r.x / gradient.size();
	params[1] = r.y / gradient.size();
}


size_t C2DTranslateTransformation::degrees_of_freedom() const
{
	return 2;
}

void C2DTranslateTransformation::update(float step, const C2DFVectorfield& a)
{
	assert(0 && "not implemented");
}

C2DFMatrix C2DTranslateTransformation::derivative_at(int x, int y) const
{
	return C2DFMatrix();
}

gsl::DoubleVector C2DTranslateTransformation::get_parameters() const
{
	gsl::DoubleVector result(2);
	result[0] = _M_transform.x;
	result[1] = _M_transform.y;
	return result;
}

void C2DTranslateTransformation::set_parameters(const gsl::DoubleVector& params)
{
	assert(params.size() == 2);
	_M_transform.x = params[0];
	_M_transform.y = params[1];
}

void C2DTranslateTransformation::set_identity()
{
	_M_transform.x = _M_transform.y = 0.0;
}

float C2DTranslateTransformation::get_max_transform() const
{
	return _M_transform.norm();
}

float C2DTranslateTransformation::pertuberate(C2DFVectorfield& v) const
{
	return 0.0;
}

C2DFVector C2DTranslateTransformation::operator () (const C2DFVector& x) const
{
	return _M_transform;
}

float C2DTranslateTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	return 0.0;
}

C2DFVector C2DTranslateTransformation::transform(const C2DFVector& x)const
{
	return x + _M_transform;
}

float C2DTranslateTransformation::divergence() const
{
	return 0.0;
}

float C2DTranslateTransformation::curl() const
{
	return 0.0;
}

float C2DTranslateTransformation::grad_divergence() const
{
	return 0.0;
}

float C2DTranslateTransformation::grad_curl() const
{
	return 0.0;
}

double C2DTranslateTransformation::get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const
{
	return 0.0;
}

double C2DTranslateTransformation::get_divcurl_cost(double wd, double wr) const
{
	return 0.0;
}


class C2DTranslateTransformCreator: public C2DTransformCreator {
	virtual P2DTransformation do_create(const C2DBounds& size) const;
};

P2DTransformation C2DTranslateTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DTranslateTransformation(size));
}

class C2DTranslateTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DTranslateTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C2DTranslateTransformCreatorPlugin::C2DTranslateTransformCreatorPlugin():
	C2DTransformCreatorPlugin("translate")
{
}

C2DTranslateTransformCreatorPlugin::ProductPtr
C2DTranslateTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C2DTranslateTransformCreator());
}

bool C2DTranslateTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DTranslateTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create a translate only transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DTranslateTransformCreatorPlugin();
}


NS_MIA_END

