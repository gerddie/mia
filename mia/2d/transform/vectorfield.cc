/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <limits>
#include <mia/core/msgstream.hh>
#include <mia/2d/transform/vectorfield.hh>
#include <mia/2d/vfio.hh>

NS_MIA_BEGIN
using namespace std;

C2DGridTransformation::C2DGridTransformation(const C2DBounds& size, const C2DInterpolatorFactory& ipf):
	C2DTransformation(ipf),
	m_field(size), 
	m_upscale_interpolator_factory("bspline:d=1", "zero")
{
}

P2DTransformation C2DGridTransformation::do_upscale(const C2DBounds& size) const
{
	/* This implementation could be improved by using something like the spline interpolator 
	   that applies the scaling in a separable way. */

	TRACE("C2DGridTransformation::upscale");
	DEBUG_ASSERT_RELEASE_THROW(m_field.get_size().x != 0 && m_field.get_size().y != 0, 
				   "C2DGridTransformation::do_upscale: input field has a zero dimension"); 


	C2DGridTransformation *result = new C2DGridTransformation(size, get_interpolator_factory());

	unique_ptr<T2DInterpolator<C2DFVector> >
		interp(m_upscale_interpolator_factory.create(m_field)); 
	

	float x_mult = float(size.x) / (float)m_field.get_size().x;
	float y_mult = float(size.y) / (float)m_field.get_size().y;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;
	
	auto i = result->m_field.begin();
	
	for (unsigned int y = 0; y < size.y; y++){
		for (unsigned int x = 0; x < size.x; x++,++i){
			C2DFVector help(ix_mult * x, iy_mult * y);
			C2DFVector val = (*interp)(help);
			*i = C2DFVector(val.x * x_mult,val.y * y_mult);
		}
	}
	
	return P2DTransformation(result);
}

const C2DBounds& C2DGridTransformation::get_size() const
{
	return m_field.get_size();
}

void C2DGridTransformation::update(float step, const C2DFVectorfield& a)
{
	C2DFVectorfield::const_iterator inf = a.begin();
	C2DFVectorfield::const_iterator enf = a.end();
	C2DFVectorfield::iterator onf = m_field.begin();

	while (inf != enf)
		*onf++ += step * *inf++;

}

C2DTransformation *C2DGridTransformation::do_clone() const
{
	return new C2DGridTransformation(*this);
}

C2DTransformation *C2DGridTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C2DGridTransformation(*this);
}


size_t C2DGridTransformation::degrees_of_freedom() const
{
	return 2 * m_field.size();
}


C2DFMatrix C2DGridTransformation::derivative_at(int x, int y) const
{
	C2DFMatrix result(C2DFVector(1.0, 0), C2DFVector(0, 1.0));
	result -= field_derivative_at(x,y);
	return result;
}

C2DFMatrix C2DGridTransformation::derivative_at(const C2DFVector& x) const
{
	C2DFMatrix result = C2DFMatrix::_1; 

	const int ix = static_cast<int>(floor(x.x)); 
	const int iy = static_cast<int>(floor(x.y)); 

	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;
	
	if (ix > 0 && ix < sx - 2 && 
	    iy > 0 && iy < sy - 2) {
		
		const float dx = x.x - ix; 
		const float dy = x.y - iy; 
		
		const float fx = 1.0 - dx; 
		const float fy = 1.0 - dy; 
		
		C2DFMatrix m[4]; 
		m[0] = field_derivative_at(ix    , iy    );
		m[1] = field_derivative_at(ix + 1, iy    );
		m[2] = field_derivative_at(ix    , iy + 1);
		m[3] = field_derivative_at(ix + 1, iy + 1);

		result -= fy * (fx * m[0] + dx * m[1]) + 
			dy * (fx * m[2] + dx * m[3]);  
	}
	return result; 
}

C2DFMatrix C2DGridTransformation::field_derivative_at(int x, int y) const
{
	C2DFMatrix result;

	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;

	if (y >= 0 && y < sy ) {
		if (x >= 0 && x < sx ) {
			C2DFVectorfield::const_pointer center = &m_field[sx * y + x];
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
	fill(m_field.begin(),m_field.end(), C2DFVector(0,0));
}

float C2DGridTransformation::get_max_transform() const
{
	C2DFVectorfield::const_iterator i = m_field.begin();
	C2DFVectorfield::const_iterator e = m_field.end();

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


CDoubleVector C2DGridTransformation::get_parameters() const
{
	CDoubleVector result(m_field.size() * 2);
	auto r = result.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
	return result;
}

void C2DGridTransformation::set_parameters(const CDoubleVector& params)
{
	assert(2 * m_field.size() == params.size());
	auto r = params.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
	}
}


C2DGridTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						    C2DFVectorfield::const_iterator start):
	C2DTransformation::iterator_impl(pos, size), 
	m_current(start)
{
	if (pos.y < size.y && pos.x < size.x ) 
		m_value = C2DFVector(get_pos()) - *m_current; 
}

C2DTransformation::iterator_impl * C2DGridTransformation::iterator_impl::clone() const
{
	return new C2DGridTransformation::iterator_impl(get_pos(), get_size(), m_current); 
}

const C2DFVector&  C2DGridTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C2DGridTransformation::iterator_impl::do_x_increment()
{
	++m_current; 
	m_value = C2DFVector(get_pos()) - *m_current; 
}

void C2DGridTransformation::iterator_impl::do_y_increment()
{
	++m_current; 
	m_value = C2DFVector(get_pos()) - *m_current; 
}


C2DFVector C2DGridTransformation::operator ()(const  C2DFVector& x) const
{
	return x - apply(x);
}

C2DGridTransformation::const_iterator C2DGridTransformation::begin() const
{
	return const_iterator(new iterator_impl(C2DBounds(0,0), 
						m_field.get_size(), m_field.begin()));
}

C2DGridTransformation::const_iterator C2DGridTransformation::end() const
{
	return const_iterator(new iterator_impl( m_field.get_size(), m_field.get_size(), 
						 m_field.end()));
}


C2DGridTransformation::field_iterator C2DGridTransformation::field_begin()
{
	return m_field.begin();
}

C2DGridTransformation::field_iterator C2DGridTransformation::field_end()
{
	return m_field.end();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_begin() const
{
	return m_field.begin();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_end()const
{
	return m_field.end();
}

void C2DGridTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(2 * params.size() != gradient.size());

	// translating the gradient also means multiplication with -1 
	// because the transformation is I-u(x)
	auto r = params.begin();
	for(auto f = gradient.begin(); f != gradient.end(); ++f, r+=2) {
		r[0] = -f->x;
		r[1] = -f->y;
	}
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
			if (gamma > max_gamma)
				max_gamma = gamma;
		}
	return sqrt(max_gamma);
}

float C2DGridTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();

	for(size_t y = 1; y < get_size().y - 1; ++y) {
		C2DFVectorfield::const_iterator iu = m_field.begin_at(1,y);
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

	C2DGridTransformation result(a.get_size(), a.get_interpolator_factory());

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

C2DGridTransformCreator::C2DGridTransformCreator(const C2DInterpolatorFactory& ipf):
	C2DTransformCreator(ipf)
{
}

P2DTransformation C2DGridTransformCreator::do_create(const C2DBounds& size, const C2DInterpolatorFactory& ipf) const
{
	return P2DTransformation(new C2DGridTransformation(size, ipf));
}



/**
   Plugin class to create the creater.  
 */
C2DGridTransformCreatorPlugin::C2DGridTransformCreatorPlugin():
	C2DTransformCreatorPlugin("vf")
{
}

C2DTransformCreator *C2DGridTransformCreatorPlugin::do_create(const C2DInterpolatorFactory& ipf) const
{
	return new C2DGridTransformCreator(ipf);
}

const std::string C2DGridTransformCreatorPlugin::do_get_descr() const
{
	return "This plug-in implements a transformation that defines a translation for "
		"each point of the grid defining the domain of the transformation.";
}


NS_MIA_END
