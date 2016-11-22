/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/3d/transform/vectorfield.hh>
//#include <mia/3d/vfio.hh>

NS_MIA_BEGIN
using namespace std;

C3DGridTransformation::C3DGridTransformation(const C3DBounds& size, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf),
	m_field(size), 
	m_upscale_interpolator_factory("bspline:d=1", "zero")
{
}

C3DFVector C3DGridTransformation::get_displacement_at(const C3DFVector& x) const
{
	return m_field.get_interpol_val_at(x);
}

P3DTransformation C3DGridTransformation::do_upscale(const C3DBounds& size) const
{
	/* This implementation could be improved by using something like the spline interpolator 
	   that applies the scaling in a separable way. */

	TRACE("C3DGridTransformation::upscale");
	DEBUG_ASSERT_RELEASE_THROW(m_field.get_size().x != 0 && m_field.get_size().y != 0, 
				   "C3DGridTransformation::do_upscale: input field has a zero dimension"); 


	C3DGridTransformation *result = new C3DGridTransformation(size, get_interpolator_factory());

	unique_ptr<T3DInterpolator<C3DFVector> >
		interp(m_upscale_interpolator_factory.create(m_field)); 
	

	C3DFVector mult(float(size.x) / (float)m_field.get_size().x, 
			float(size.y) / (float)m_field.get_size().y,
			float(size.z) / (float)m_field.get_size().z); 
	
	C3DFVector imult = C3DFVector::_1 / mult; 
	
	auto i = result->m_field.begin_range(C3DBounds::_0, m_field.get_size());
	auto e = result->m_field.end_range(C3DBounds::_0, m_field.get_size());
	

	while (i != e)
		*i++ = (*interp)(imult * C3DFVector(i.pos())) * mult;
	
	return P3DTransformation(result);
}

const C3DBounds& C3DGridTransformation::get_size() const
{
	return m_field.get_size();
}

void C3DGridTransformation::update(float step, const C3DFVectorfield& a)
{
	C3DFVectorfield::const_iterator inf = a.begin();
	C3DFVectorfield::const_iterator enf = a.end();
	C3DFVectorfield::iterator onf = m_field.begin();

	while (inf != enf)
		*onf++ += step * *inf++;

}

C3DTransformation *C3DGridTransformation::do_clone() const
{
	return new C3DGridTransformation(*this);
}

C3DTransformation *C3DGridTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C3DGridTransformation(*this);
}


size_t C3DGridTransformation::degrees_of_freedom() const
{
	return 3 * m_field.size();
}

C3DFMatrix C3DGridTransformation::derivative_at(const  C3DFVector& x) const
{
	
	C3DFMatrix result = C3DFMatrix::_1; 

	const int ix = static_cast<int>(floor(x.x)); 
	const int iy = static_cast<int>(floor(x.y)); 
	const int iz = static_cast<int>(floor(x.z)); 

	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;
	const int sz = m_field.get_size().z;
	
	if (ix > 0 && ix < sx - 2 && 
	    iy > 0 && iy < sy - 2 && 
	    iz > 0 && iz < sz - 2) {
		
		const float dx = x.x - ix; 
		const float dy = x.y - iy; 
		const float dz = x.z - iz; 
		
		const float fx = 1.0 - dx; 
		const float fy = 1.0 - dy; 
		const float fz = 1.0 - dz; 
		
		C3DFMatrix m[8]; 
		m[0] = field_derivative_at(ix    , iy    , iz    );
		m[1] = field_derivative_at(ix + 1, iy    , iz    );
		m[2] = field_derivative_at(ix    , iy + 1, iz    );
		m[3] = field_derivative_at(ix + 1, iy + 1, iz    );
		m[4] = field_derivative_at(ix    , iy    , iz + 1);
		m[5] = field_derivative_at(ix + 1, iy    , iz + 1);
		m[6] = field_derivative_at(ix    , iy + 1, iz + 1);
		m[7] = field_derivative_at(ix + 1, iy + 1, iz + 1);

		result -= 
			fz * (
				fy * (fx * m[0] + dx * m[1]) + 
				dy * (fx * m[2] + dx * m[3])) + 
			dz * (
				fy * (fx * m[4] + dx * m[5]) + 
				dy * (fx * m[6] + dx * m[7])); 
	}
	return result; 
}

C3DFMatrix C3DGridTransformation::derivative_at(int x, int y, int z) const
{
	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;
	const int sz = m_field.get_size().z;

	C3DFMatrix result = C3DFMatrix::_1; 
	if (z >= 0 && z < sz && 
	    y >= 0 && y < sy && 
	    x >= 0 && x < sx) {
		result -= field_derivative_at(x,y,z);
	}
	return result; 
}

C3DFMatrix C3DGridTransformation::field_derivative_at(int x, int y, int z) const
{
	C3DFMatrix result;
	
	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;
	const int sz = m_field.get_size().z;
	const int sxy = sx * sy; 

	C3DFVectorfield::const_pointer center = &m_field[(z * sy + y) * sx  + x];
	if (x > 0 && x < sx - 1) {
		result.x = (center[ 1] - center[-1]) * 0.5f;
	}
	if (y > 0 && y < sy - 1 ) {
		result.y = (center[ sx] - center[-sx]) * 0.5f;
	}
	if (z > 0 && z < sz - 1 ) {
		result.z = (center[ sxy] - center[-sxy]) * 0.5f;
	}
	return result;
}

void C3DGridTransformation::set_identity()
{
	fill(m_field.begin(),m_field.end(), C3DFVector::_0);
}

float C3DGridTransformation::get_max_transform() const
{
	C3DFVectorfield::const_iterator i = m_field.begin();
	C3DFVectorfield::const_iterator e = m_field.end();

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


CDoubleVector C3DGridTransformation::get_parameters() const
{
	CDoubleVector result(m_field.size() * 3);
	auto r = result.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
		*r++ = f->z;
	}
	return result;
}

void C3DGridTransformation::set_parameters(const CDoubleVector& params)
{
	assert(3 * m_field.size() == params.size());
	auto r = params.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
		f->z = *r++;
	}
}


C3DGridTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						    C3DFVectorfield::const_iterator start):
	C3DTransformation::iterator_impl(pos, size), 
	m_current(start)
{
	if (pos.y < size.y && pos.x < size.x && pos.z < size.z ) 
		m_value = C3DFVector(get_pos()) - *m_current; 
}

C3DGridTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, const C3DBounds& end, 
						    const C3DBounds& size, C3DFVectorfield::const_iterator start):
C3DTransformation::iterator_impl(pos, begin, end, size), 
		    m_current(start)
{
	if (pos.y < size.y && pos.x < size.x && pos.z < size.z ) 
		m_value = C3DFVector(get_pos()) - *m_current; 
}
		    

C3DTransformation::const_iterator C3DGridTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	TRACE_FUNCTION;
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), 
								   m_field.begin_at(begin.x, begin.y, begin.z)));
}


C3DTransformation::const_iterator C3DGridTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	TRACE_FUNCTION;
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), 
								   m_field.begin_at(end.x, end.y, end.z)));

}


C3DTransformation::iterator_impl * C3DGridTransformation::iterator_impl::clone() const
{
	return new C3DGridTransformation::iterator_impl(get_pos(), get_size(), m_current); 
}

const C3DFVector&  C3DGridTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DGridTransformation::iterator_impl::do_x_increment()
{
	++m_current; 
	m_value = C3DFVector(get_pos()) - *m_current; 
}

void C3DGridTransformation::iterator_impl::do_y_increment()
{
	++m_current; 
	m_value = C3DFVector(get_pos()) - *m_current; 
}

void C3DGridTransformation::iterator_impl::do_z_increment()
{
	++m_current; 
	m_value = C3DFVector(get_pos()) - *m_current; 
}


C3DFVector C3DGridTransformation::operator ()(const  C3DFVector& x) const
{
	return x - get_displacement_at(x);
}

C3DGridTransformation::const_iterator C3DGridTransformation::begin() const
{
	return const_iterator(new iterator_impl(C3DBounds::_0, m_field.get_size(), m_field.begin()));
}

C3DGridTransformation::const_iterator C3DGridTransformation::end() const
{
	return const_iterator(new iterator_impl( m_field.get_size(), m_field.get_size(), 
						 m_field.end()));
}


C3DGridTransformation::field_iterator C3DGridTransformation::field_begin()
{
	return m_field.begin();
}

C3DGridTransformation::field_iterator C3DGridTransformation::field_end()
{
	return m_field.end();
}

C3DGridTransformation::const_field_iterator C3DGridTransformation::field_begin() const
{
	return m_field.begin();
}

C3DGridTransformation::const_field_iterator C3DGridTransformation::field_end()const
{
	return m_field.end();
}

void C3DGridTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(3 * params.size() != gradient.size());

	// translating the gradient also means multiplication with -1 
	// because the transformation is I-u(x)
	auto r = params.begin();
	for(auto f = gradient.begin(); f != gradient.end(); ++f, r+=3) {
		r[0] = -f->x;
		r[1] = -f->y;
		r[2] = -f->z;
	}
}

float C3DGridTransformation::pertuberate(C3DFVectorfield& v) const
{
	C3DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	for (size_t z = 0; z < v.get_size().z; ++z)
		for (size_t y = 0; y < v.get_size().y; ++y)
			for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
				const C3DFMatrix j = field_derivative_at(x,y,z);
				const C3DFVector ue = j * *iv;
				*iv -= ue;
				float gamma = iv->norm2();
				if (gamma > max_gamma)
					max_gamma = gamma;
			}
	return sqrt(max_gamma);
}



float C3DGridTransformation::get_jacobian(const C3DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_size());
	assert(0 && "this function needs to be fixed"); 
	const int dx = v.get_size().x;
	const int dxy = v.get_size().x * v.get_size().y;
	
	float j_min = numeric_limits<float>::max();

	for(size_t z = 1; z < get_size().z - 1; ++z) {
		for(size_t y = 1; y < get_size().y - 1; ++y) {
			C3DFVectorfield::const_iterator iu = m_field.begin_at(1,y,z);
			C3DFVectorfield::const_iterator iv = v.begin_at(1,y,z);
			for(size_t x = 1; x < get_size().x - 1; ++x, ++iu, ++iv) {
				C3DFVector jx((iu[1] - iu[-1]) + (delta * (iv[1] - iv[-1])));
				C3DFVector jy((iu[dx] - iu[-dx]) + (delta * (iv[dx] - iv[-dx])));
				C3DFVector jz((iu[dxy] - iu[-dxy]) + (delta * (iv[dxy] - iv[-dxy])));
				
				const float j = (2.0 - jx.x) * (2.0 - jy.y) * (2.0 - jz.z) - jx.y * jy.x * jy.z;
				if ( j_min > j) {
					j_min = j;
				}
			}
		}
	}
	return 0.25 * j_min;
}

EXPORT_3D C3DGridTransformation operator + (const C3DGridTransformation& a, const C3DGridTransformation& b)
{
	assert( a.get_size() == b.get_size());

	C3DGridTransformation result(a.get_size(), a.get_interpolator_factory());

	C3DFVectorfield::iterator ri = result.field_begin();
	C3DFVectorfield::const_iterator bi = b.field_begin();
	
	
	for (size_t z = 0; z < a.get_size().z; ++z)  {
		for (size_t y = 0; y < a.get_size().y; ++y)  {
			for (size_t x = 0; x < a.get_size().x; ++x, ++ri, ++bi)  {
				const C3DFVector xi = C3DFVector(x,y,z) - *bi;
				*ri = a.get_displacement_at(xi) +  *bi;
			}
		}
	}
	return result;
}

/**
   Transformation creator 
 */

C3DGridTransformCreator::C3DGridTransformCreator(const C3DInterpolatorFactory& ipf):
	C3DTransformCreator(ipf)
{
}

P3DTransformation C3DGridTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DGridTransformation(size, ipf));
}


C3DGridTransformCreatorPlugin::C3DGridTransformCreatorPlugin():
	C3DTransformCreatorPlugin("vf")
{
}

C3DTransformCreator *C3DGridTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DGridTransformCreator(ipf);
}

const std::string C3DGridTransformCreatorPlugin::do_get_descr() const
{
	return "This plug-in implements a transformation that defines a translation for "
		"each point of the grid defining the domain of the transformation.";
}


NS_MIA_END
