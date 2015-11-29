/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#include <mia/3d/linear_transform.hh>

NS_MIA_BEGIN
using namespace std;

C3DLinearTransformation::C3DLinearTransformation(const C3DBounds& size, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_matrix(12,0.0f),
	m_size(size)
{
	m_matrix[0] = m_matrix[5] = m_matrix[10] =  1.0;
}


C3DLinearTransformation::C3DLinearTransformation(const C3DLinearTransformation& other):
	C3DTransformation(other), 
	m_matrix(other.m_matrix),
	m_size(other.m_size)
{
}


C3DFVector C3DLinearTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DLinearTransformation::transform(const C3DFVector& x)const
{
	return C3DFVector(
		m_matrix[0] * x.x + m_matrix[1] * x.y + m_matrix[2] * x.z + m_matrix[3],
		m_matrix[4] * x.x + m_matrix[5] * x.y + m_matrix[6] * x.z + m_matrix[7],
		m_matrix[8] * x.x + m_matrix[9] * x.y + m_matrix[10] * x.z + m_matrix[11]);
}

C3DTransformation *C3DLinearTransformation::do_clone()const
{
	return new C3DLinearTransformation(*this);
}

C3DTransformation *C3DLinearTransformation::invert()const
{
	const double det = 
		m_matrix[10] * (m_matrix[0] * m_matrix[5] - m_matrix[1] * m_matrix[4]) + 
		m_matrix[ 9] * (m_matrix[2] * m_matrix[4] - m_matrix[0] * m_matrix[6]) + 
		m_matrix[ 8] * (m_matrix[1] * m_matrix[6] - m_matrix[2] * m_matrix[5]); 
	
	if (std::fabs(det) < 1e-8) 
		throw invalid_argument("C3DLinearTransformation::invert(): Matrix is numerically singular"); 
	
	const double inv_det = 1.0 / det; 

	C3DLinearTransformation *result = new C3DLinearTransformation(*this);

	const double h1625 =  m_matrix[1]  * m_matrix[6] - m_matrix[2] * m_matrix[5]; 
	const double h1735 =  m_matrix[1]  * m_matrix[7] - m_matrix[3] * m_matrix[5]; 
	const double h2736 =  m_matrix[2]  * m_matrix[7] - m_matrix[3] * m_matrix[6]; 
	
	const double h0624 =  m_matrix[0]  * m_matrix[6] - m_matrix[2] * m_matrix[4]; 
	const double h3407 =  m_matrix[3]  * m_matrix[4] - m_matrix[0] * m_matrix[7];
	const double h0514 =  m_matrix[0]  * m_matrix[5] - m_matrix[1] * m_matrix[4];
	
	
	result->m_matrix[0] = ( m_matrix[10] * m_matrix[5] - m_matrix[6] * m_matrix[9])  * inv_det; 
	result->m_matrix[1] = - ( m_matrix[10] * m_matrix[1] - m_matrix[2] * m_matrix[9])  * inv_det; 
	result->m_matrix[2] = h1625  * inv_det; 
	result->m_matrix[3] = -(h1625 *m_matrix[11] - h1735 *m_matrix[10] + h2736 *m_matrix[9])  * inv_det;

	result->m_matrix[4] = -( m_matrix[10] * m_matrix[4] - m_matrix[6] * m_matrix[8])  * inv_det; 
	result->m_matrix[5] = ( m_matrix[10] * m_matrix[0] - m_matrix[2] * m_matrix[8])  * inv_det; 
	result->m_matrix[6] = -h0624  * inv_det; 
	result->m_matrix[7] = (h0624*m_matrix[11] + h3407 *m_matrix[10] + h2736 *m_matrix[8])  * inv_det;

	result->m_matrix[8]  = ( m_matrix[4] * m_matrix[9] - m_matrix[5] * m_matrix[8])  * inv_det; 
	result->m_matrix[9]  = -( m_matrix[0] * m_matrix[9] - m_matrix[1] * m_matrix[8])  * inv_det; 
	result->m_matrix[10] = h0514  * inv_det; 
	result->m_matrix[11] = -(h0514*m_matrix[11] + h3407 *m_matrix[9] + h1735 *m_matrix[8])  * inv_det;

	return result; 
}


C3DLinearTransformation::C3DLinearTransformation(const C3DBounds& size, std::vector<double> transform, 
						 const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_matrix(transform),
	m_size(size)
{
}

size_t C3DLinearTransformation::degrees_of_freedom() const
{
	return 12;
}

CDoubleVector C3DLinearTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	copy(m_matrix.begin(), m_matrix.end(), result.begin());
	return result;
}

void C3DLinearTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), m_matrix.begin());

}

const C3DBounds& C3DLinearTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DLinearTransformation::do_upscale(const C3DBounds& size) const
{
	float x_mult = float(size.x) / (float)get_size().x;
	float y_mult = float(size.y) / (float)get_size().y;
	float z_mult = float(size.z) / (float)get_size().z;

	C3DLinearTransformation *result = new C3DLinearTransformation(*this);
	result->m_size   = size;
	result->m_matrix[3]  *= x_mult;
	result->m_matrix[7]  *= y_mult;
	result->m_matrix[11] *= z_mult;

	return P3DTransformation(result);
}


C3DFMatrix C3DLinearTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	return C3DFMatrix(C3DFVector(m_matrix[0], m_matrix[1], m_matrix[2]),
			  C3DFVector(m_matrix[4], m_matrix[5], m_matrix[6]),
			  C3DFVector(m_matrix[8], m_matrix[9], m_matrix[10]));
}

C3DFMatrix C3DLinearTransformation::derivative_at(int MIA_PARAM_UNUSED(x), int MIA_PARAM_UNUSED(y), 
						  int MIA_PARAM_UNUSED(z)) const
{
	return C3DFMatrix(C3DFVector(m_matrix[0], m_matrix[1], m_matrix[2]),
			  C3DFVector(m_matrix[4], m_matrix[5], m_matrix[6]),
			  C3DFVector(m_matrix[8], m_matrix[9], m_matrix[10]));
}

void C3DLinearTransformation::set_identity()
{
	fill(m_matrix.begin(), m_matrix.end(), 0.0);
	m_matrix[0] = m_matrix[5] = m_matrix[10] = 1.0;
}

float C3DLinearTransformation::get_max_transform() const
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


C3DFVector C3DLinearTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}


void C3DLinearTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t z = 0; z < m_size.z; ++z) {
		for (size_t y = 0; y < m_size.y; ++y) {
			for (size_t x = 0; x < m_size.x; ++x, ++g) {
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



C3DLinearTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DLinearTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DLinearTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						      const C3DBounds& end, const C3DBounds& size, 
						      const C3DLinearTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DTransformation::iterator_impl * C3DLinearTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DLinearTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DLinearTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DLinearTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DLinearTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}


C3DTransformation::const_iterator C3DLinearTransformation::begin() const
{
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DLinearTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DLinearTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DLinearTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}


/*
  deprecated functions ... 

*/


float C3DLinearTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DLinearTransformation doesn't implement a jacobian."); 
}

void C3DLinearTransformation::add(const C3DTransformation& other)
{
	// *this  = other * *this
	const C3DLinearTransformation& a = dynamic_cast<const C3DLinearTransformation&>(other);

	vector<double> h(m_matrix.size());

	h[0] = a.m_matrix[0] * m_matrix[0] + a.m_matrix[1] * m_matrix[4] + a.m_matrix[2] * m_matrix[8];
	h[1] = a.m_matrix[0] * m_matrix[1] + a.m_matrix[1] * m_matrix[5] + a.m_matrix[2] * m_matrix[9];
	h[2] = a.m_matrix[0] * m_matrix[2] + a.m_matrix[1] * m_matrix[6] + a.m_matrix[2] * m_matrix[10];
	h[3] = a.m_matrix[0] * m_matrix[3] + a.m_matrix[1] * m_matrix[7] + a.m_matrix[2] * m_matrix[11] + a.m_matrix[3];

	h[4] = a.m_matrix[4] * m_matrix[0] + a.m_matrix[5] * m_matrix[4] + a.m_matrix[6] * m_matrix[8];
	h[5] = a.m_matrix[4] * m_matrix[1] + a.m_matrix[5] * m_matrix[5] + a.m_matrix[6] * m_matrix[9];
	h[6] = a.m_matrix[4] * m_matrix[2] + a.m_matrix[5] * m_matrix[6] + a.m_matrix[6] * m_matrix[10];
	h[7] = a.m_matrix[4] * m_matrix[3] + a.m_matrix[5] * m_matrix[7] + a.m_matrix[6] * m_matrix[11] + a.m_matrix[7];

	h[8]  = a.m_matrix[8] * m_matrix[0] + a.m_matrix[9] * m_matrix[4] + a.m_matrix[10] * m_matrix[8];
	h[9]  = a.m_matrix[8] * m_matrix[1] + a.m_matrix[9] * m_matrix[5] + a.m_matrix[10] * m_matrix[9];
	h[10] = a.m_matrix[8] * m_matrix[2] + a.m_matrix[9] * m_matrix[6] + a.m_matrix[10] * m_matrix[10];
	h[11] = a.m_matrix[8] * m_matrix[3] + a.m_matrix[9] * m_matrix[7] + a.m_matrix[10] * m_matrix[11] + a.m_matrix[11];

	copy(h.begin(), h.end(), m_matrix.begin());
}


float C3DLinearTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DLinearTransformation doesn't implement pertuberate."); 
}

double C3DLinearTransformation::get_divcurl_cost(double, double, CDoubleVector&) const
{
	return 0.0; 
}

double C3DLinearTransformation::get_divcurl_cost(double, double) const
{
	return 0.0; 
}


float C3DLinearTransformation::divergence() const
{
	assert(0 && "not implemented");
	return m_matrix[0] + m_matrix[1] + m_matrix[3] + m_matrix[4] - 2.0f;
}

float C3DLinearTransformation::grad_divergence() const
{
	return 0.0;
}


float C3DLinearTransformation::grad_curl() const
{
	return 0.0;
}


float C3DLinearTransformation::curl() const
{
	assert(0 && "not implemented");
	return m_matrix[1] + m_matrix[4] - m_matrix[0] - m_matrix[3];
}

void C3DLinearTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}
