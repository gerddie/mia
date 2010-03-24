/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
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
 *
 */
#include <limits>
#include <mia/core/msgstream.hh>
#include <mia/2d/transform/vectorfield.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/2dvfio.hh>

NS_MIA_BEGIN
using namespace std;

C2DGridTransformation::C2DGridTransformation(const C2DBounds& size):
	_M_field(size)
{
}

P2DTransformation C2DGridTransformation::upscale(const C2DBounds& size) const
{
	TRACE("C2DGridTransformation::upscale");
	C2DGridTransformation *result = new C2DGridTransformation(size);

	// initial upscale
	if (_M_field.get_size().x != 0 && _M_field.get_size().y != 0) {

		float x_mult = float(size.x) / (float)_M_field.get_size().x;
		float y_mult = float(size.y) / (float)_M_field.get_size().y;
		float ix_mult = 1.0f / x_mult;
		float iy_mult = 1.0f / y_mult;

		C2DFVectorfield::iterator i = result->_M_field.begin();

		for (unsigned int y = 0; y < size.y; y++){
			for (unsigned int x = 0; x < size.x; x++,++i){
				C2DFVector help(ix_mult * x, iy_mult * y);
				C2DFVector val = _M_field.get_interpol_val_at(help);
				*i = C2DFVector(val.x * x_mult,val.y * y_mult);
			}
		}
	}
	return P2DTransformation(result);
}

const C2DBounds& C2DGridTransformation::get_size() const
{
	return _M_field.get_size();
}

bool C2DGridTransformation::save(const std::string& filename, const std::string& type) const
{
	C2DIOVectorfield outfield(_M_field);
	return C2DVFIOPluginHandler::instance().save(type, filename, outfield);
}

void C2DGridTransformation::add(const C2DTransformation& a)
{
	const C2DGridTransformation& other = dynamic_cast<const C2DGridTransformation&>(a);
	_M_field += other._M_field;
}

void C2DGridTransformation::update(float step, const C2DFVectorfield& a)
{
	C2DFVectorfield::const_iterator inf = a.begin();
	C2DFVectorfield::const_iterator enf = a.end();
	C2DFVectorfield::iterator onf = _M_field.begin();

	while (inf != enf)
		*onf++ += step * *inf++;

}

C2DTransformation *C2DGridTransformation::clone() const
{
	return new C2DGridTransformation(*this);
}

size_t C2DGridTransformation::degrees_of_freedom() const
{
	return 2 * _M_field.size();
}


C2DFMatrix C2DGridTransformation::derivative_at(int x, int y) const
{
	C2DFMatrix result(C2DFVector(1.0, 0), C2DFVector(0, 1.0));
	result -= field_derivative_at(x,y);
	return result;
}

C2DFMatrix C2DGridTransformation::field_derivative_at(int x, int y) const
{
	C2DFMatrix result;

	const int sx = _M_field.get_size().x;
	const int sy = _M_field.get_size().y;

	if (y >= 0 && y < sy ) {
		if (x >= 0 && x < sx ) {
			C2DFVectorfield::const_pointer center = &_M_field[sx * y + x];
			if (x > 0 && x < sx - 1) {
				result.x = (center[ 1] - center[-1]) * 0.5f;
			}
			if (y > 0 && y < sy - 1 ) {
				result.y = (center[ sx] - center[-sx]) * 0.5f;
			}
		}
	}
	return result;
}

void C2DGridTransformation::set_identity()
{
	fill(_M_field.begin(),_M_field.end(), C2DFVector(0,0));
}

float C2DGridTransformation::get_max_transform() const
{
	C2DFVectorfield::const_iterator i = _M_field.begin();
	C2DFVectorfield::const_iterator e = _M_field.end();

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


C2DGridTransformation::const_iterator& C2DGridTransformation::const_iterator::operator ++()
{
	++_M_current;
	++_M_pos.x;

	if (_M_pos.x >= _M_size.x) {
		_M_pos.x = 0;
		++_M_pos.y;
	}
	return *this;
}

C2DGridTransformation::const_iterator C2DGridTransformation::const_iterator::operator ++(int)
{
	const_iterator help(*this);
	++(*this);
	return help;
}

const C2DFVector C2DGridTransformation::const_iterator::operator *() const
{
	return C2DFVector(_M_pos) - *_M_current;
}

EXPORT_2D bool operator == (const C2DGridTransformation::const_iterator& a, const C2DGridTransformation::const_iterator& b)
{
	return (a._M_current == b._M_current);
}

EXPORT_2D bool operator != (const C2DGridTransformation::const_iterator& a,
			    const C2DGridTransformation::const_iterator& b)
{
	return !( a == b);
}

C2DGridTransformation::const_iterator::const_iterator():
	_M_pos(0,0),
	_M_size(0,0)
{
}

C2DGridTransformation::const_iterator::const_iterator(const C2DBounds& pos, const C2DBounds& size, C2DFVectorfield::const_iterator start):
	_M_pos(pos),
	_M_size(size),
	_M_current(start)
{
}


P2DImage C2DGridTransformation::apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const
{
	assert(image.get_size() == _M_field.get_size());
	return transform2d(image, ipf, *this);
}

C2DFVector C2DGridTransformation::operator ()(const  C2DFVector& x) const
{
	return x - apply(x);
}

C2DGridTransformation::const_iterator C2DGridTransformation::begin() const
{
	return const_iterator(C2DBounds(0,0), _M_field.get_size(), _M_field.begin());
}

C2DGridTransformation::const_iterator C2DGridTransformation::end() const
{
	return const_iterator(C2DBounds(0,_M_field.get_size().y), _M_field.get_size(), _M_field.end());
}


C2DGridTransformation::field_iterator C2DGridTransformation::field_begin()
{
	return _M_field.begin();
}

C2DGridTransformation::field_iterator C2DGridTransformation::field_end()
{
	return _M_field.end();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_begin() const
{
	return _M_field.begin();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_end()const
{
	return _M_field.end();
}

C2DFVectorfield C2DGridTransformation::translate(const C2DFVectorfield& gradient) const
{
	return gradient;
}

float C2DGridTransformation::pertuberate(C2DFVectorfield& v) const
{
	C2DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	for (size_t y = 0; y < v.get_size().y; ++y)
		for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
			const C2DFMatrix j = field_derivative_at(x,y);
			const C2DFVector ue = j * *iv;
			*iv -= ue;
			float gamma = iv->norm2();
			if (gamma > max_gamma) {
				max_gamma = gamma;
			}
		}
	return sqrt(max_gamma);
}

float C2DGridTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();

	for(size_t y = 1; y < get_size().y - 1; ++y) {
		C2DFVectorfield::const_iterator iu = _M_field.begin_at(1,y);
		C2DFVectorfield::const_iterator iv = v.begin_at(1,y);
		for(size_t x = 1; x < get_size().x - 1; ++x, ++iu, ++iv) {
			C2DFVector jx((iu[1] - iu[-1]) + (delta * (iv[1] - iv[-1])));
			C2DFVector jy((iu[dx] - iu[-dx]) + (delta * (iv[dx] - iv[-dx])));

			const float j = (2.0 - jx.x) * (2.0 - jy.y) - jx.y * jy.x;
			if ( j_min > j) {
				j_min = j;
			}

		}
	}
	return 0.25 * j_min;
}

EXPORT_2D C2DGridTransformation operator + (const C2DGridTransformation& a, const C2DGridTransformation& b)
{
	assert( a.get_size() == b.get_size());

	C2DGridTransformation result(a.get_size());

	C2DFVectorfield::iterator ri = result.field_begin();
	C2DFVectorfield::const_iterator bi = b.field_begin();

	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x, ++ri, ++bi)  {
			const C2DFVector xi = C2DFVector(x,y) - *bi;
			*ri = a.apply(xi) +  *bi;
		}
	}
	return result;
}

class C2DGridTransformCreator: public C2DTransformCreator {
	virtual P2DTransformation do_create(const C2DBounds& size) const;
};

P2DTransformation C2DGridTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DGridTransformation(size));
}

class C2DGridTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DGridTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C2DGridTransformCreatorPlugin::C2DGridTransformCreatorPlugin():
	C2DTransformCreatorPlugin("vf")
{
}

C2DGridTransformCreatorPlugin::ProductPtr
C2DGridTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C2DGridTransformCreator());
}

bool C2DGridTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DGridTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create vectorfield transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DGridTransformCreatorPlugin();
}



NS_MIA_END
