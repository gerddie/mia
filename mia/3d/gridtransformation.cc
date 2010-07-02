/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/msgstream.hh>
#include <mia/3d/gridtransformation.hh>
#include <mia/3d/3dvfio.hh>

NS_MIA_BEGIN


C3DGridTransformation::C3DGridTransformation(const C3DBounds& size):
	_M_field(size)
{
}

C3DFVectorfield& C3DGridTransformation::field()
{
	return _M_field;
}

const C3DFVectorfield& C3DGridTransformation::field() const
{
	return _M_field;
}

P3DTransformation C3DGridTransformation::do_upscale(const C3DBounds& size) const
{
	TRACE("C3DGridTransformation::upscale");
	C3DGridTransformation *result = new C3DGridTransformation(size);

	// initial upscale
	if (_M_field.get_size().x != 0 && _M_field.get_size().y != 0) {

		float x_mult = float(size.x) / (float)_M_field.get_size().x;
		float y_mult = float(size.y) / (float)_M_field.get_size().y;
		float z_mult = float(size.z) / (float)_M_field.get_size().z;
		float ix_mult = 1.0f / x_mult;
		float iy_mult = 1.0f / y_mult;
		float iz_mult = 1.0f / z_mult;

		C3DFVectorfield::iterator i = result->_M_field.begin();

		for (unsigned int z = 0; z < size.z; z++){
			for (unsigned int y = 0; y < size.y; y++){
				for (unsigned int x = 0; x < size.x; x++,++i){
					C3DFVector help(ix_mult * x, iy_mult * y, iz_mult * z);
					C3DFVector val = _M_field.get_interpol_val_at(help);
					*i = C3DFVector(val.x * x_mult,val.y * y_mult, val.z * z_mult);
				}
			}
		}
	}
	return P3DTransformation(result);
}

const C3DBounds& C3DGridTransformation::get_size() const
{
	return _M_field.get_size();
}

bool C3DGridTransformation::do_save(const std::string& filename, const std::string& type) const
{
	C3DIOVectorfield outfield(_M_field);
	return C3DVFIOPluginHandler::instance().save(type, filename, outfield);
}

void C3DGridTransformation::do_add(const C3DTransformation& a)
{
	const C3DGridTransformation& other = dynamic_cast<const C3DGridTransformation&>(a);
	_M_field += other._M_field;
}

size_t C3DGridTransformation::do_degrees_of_freedom() const
{
	return 3 * _M_field.size();
}


C3DGridTransformation::const_iterator& C3DGridTransformation::const_iterator::operator ++()
{
	++_M_current;
	++_M_pos.x;

	if (_M_pos.x >= _M_size.x) {
		_M_pos.x = 0;
		++_M_pos.y;
		if (_M_pos.y >= _M_size.y) {
			_M_pos.y = 0;
			++_M_pos.z;
		}
	}
	return *this;
}

C3DGridTransformation::const_iterator C3DGridTransformation::const_iterator::operator ++(int)
{
	const_iterator help(*this);
	++(*this);
	return help;
}

const C3DFVector C3DGridTransformation::const_iterator::operator *() const
{
	return C3DFVector(_M_pos) - *_M_current;
}

bool operator == (const C3DGridTransformation::const_iterator& a, const C3DGridTransformation::const_iterator& b)
{
	return (a._M_current == b._M_current);
}

C3DGridTransformation::const_iterator::const_iterator():
	_M_pos(0,0,0),
	_M_size(0,0,0)
{
}

C3DGridTransformation::const_iterator::const_iterator(const C3DBounds& pos,
						      const C3DBounds& size,
						      C3DFVectorfield::const_iterator start):
	_M_pos(pos),
	_M_size(size),
	_M_current(start)
{
}


P3DImage C3DGridTransformation::apply(const C3DImage& image, const C3DInterpolatorFactory& ipf) const
{
	assert(image.get_size() == _M_field.get_size());
	return transform3d(image, ipf, *this);
}

C3DGridTransformation::const_iterator C3DGridTransformation::begin() const
{
	return const_iterator(C3DBounds(0,0,0), _M_field.get_size(), _M_field.begin());
}

C3DGridTransformation::const_iterator C3DGridTransformation::end() const
{
	return const_iterator(C3DBounds(0,_M_field.get_size().y, _M_field.get_size().z ),
			      _M_field.get_size(), _M_field.end());
}


C3DGridTransformation::const_iterator C3DGridTransformation::begin_at(size_t x, size_t y, size_t z) const
{
	return const_iterator(C3DBounds(x,y,z), _M_field.get_size(), _M_field.begin_at(x,y,z));
}


C3DGridTransformation::field_iterator C3DGridTransformation::field_begin()
{
	return _M_field.begin();
}

C3DGridTransformation::field_iterator C3DGridTransformation::field_end()
{
	return _M_field.end();
}

C3DGridTransformation::const_field_iterator C3DGridTransformation::field_begin() const
{
	return _M_field.begin();
}

C3DGridTransformation::const_field_iterator C3DGridTransformation::field_end()const
{
	return _M_field.end();
}

C3DGridTransformation::operator C3DFVectorfield&()
{
	return _M_field;
}


C3DGridTransformation operator + (const C3DGridTransformation& a, const C3DGridTransformation& b)
{
	assert( a.get_size() == b.get_size());

	C3DGridTransformation result(a.get_size());
	const int slice = a.get_size().y * a.get_size().x;

#ifdef __OPENMP
#pragma omp parallel for  shared(result) schedule(static) default(none)
#endif
	for (size_t z = 0; z < a.get_size().z; ++z)  {
		C3DFVectorfield::iterator ri = result.field_begin() + z * slice;
		C3DFVectorfield::const_iterator bi = b.field_begin() + z * slice;
		for (size_t y = 0; y < a.get_size().y; ++y)  {
			for (size_t x = 0; x < a.get_size().x; ++x, ++ri, ++bi)  {
				const C3DFVector xi = C3DFVector(x,y,z) - *bi;
				*ri = a(xi) +  *bi;
			}
		}
	}
	return result;
}


NS_MIA_END

