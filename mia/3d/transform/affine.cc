/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <fstream>
#include <cmath>
#include <mia/core/msgstream.hh>
#include <mia/3d/transformfactory.hh>

#include <mia/3d/transform/affine.hh>


NS_MIA_BEGIN
using namespace std;

C3DFVector C3DAffineTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DAffineTransformation::transform(const C3DFVector& x)const
{
	return C3DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2] * x.z + m_t[3],
		m_t[4] * x.x + m_t[5] * x.y + m_t[6] * x.z + m_t[7],
		m_t[8] * x.x + m_t[9] * x.y + m_t[10] * x.z + m_t[11]);
}

C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(12,0.0f),
	m_size(size)
{
	m_t[0] = m_t[5] = m_t[10] =  1.0;
}

C3DAffineTransformation::C3DAffineTransformation(const C3DAffineTransformation& other):
	C3DTransformation(other), 
	m_t(other.m_t),
	m_size(other.m_size)
{
}

C3DTransformation *C3DAffineTransformation::do_clone()const
{
	return new C3DAffineTransformation(*this);
}

C3DTransformation *C3DAffineTransformation::invert()const
{
	const double det = 
		m_t[10] * (m_t[0] * m_t[5] - m_t[1] * m_t[4]) + 
		m_t[ 9] * (m_t[2] * m_t[4] - m_t[0] * m_t[6]) + 
		m_t[ 8] * (m_t[1] * m_t[6] - m_t[2] * m_t[5]); 
	
	if (std::fabs(det) < 1e-8) 
		throw invalid_argument("C3DAffineTransformation::invert(): Matrix is numerically singular"); 
	
	const double inv_det = 1.0 / det; 

	C3DAffineTransformation *result = new C3DAffineTransformation(*this);

	const double h1625 =  m_t[1]  * m_t[6] - m_t[2] * m_t[5]; 
	const double h1735 =  m_t[1]  * m_t[7] - m_t[3] * m_t[5]; 
	const double h2736 =  m_t[2]  * m_t[7] - m_t[3] * m_t[6]; 
	
	const double h0624 =  m_t[0]  * m_t[6] - m_t[2] * m_t[4]; 
	const double h3407 =  m_t[3]  * m_t[4] - m_t[0] * m_t[7];
	const double h0514 =  m_t[0]  * m_t[5] - m_t[1] * m_t[4];
	
	
	result->m_t[0] = ( m_t[10] * m_t[5] - m_t[6] * m_t[9])  * inv_det; 
	result->m_t[1] = - ( m_t[10] * m_t[1] - m_t[2] * m_t[9])  * inv_det; 
	result->m_t[2] = h1625  * inv_det; 
	result->m_t[3] = -(h1625 *m_t[11] - h1735 *m_t[10] + h2736 *m_t[9])  * inv_det;

	result->m_t[4] = -( m_t[10] * m_t[4] - m_t[6] * m_t[8])  * inv_det; 
	result->m_t[5] = ( m_t[10] * m_t[0] - m_t[2] * m_t[8])  * inv_det; 
	result->m_t[6] = -h0624  * inv_det; 
	result->m_t[7] = (h0624*m_t[11] + h3407 *m_t[10] + h2736 *m_t[8])  * inv_det;

	result->m_t[8]  = ( m_t[4] * m_t[9] - m_t[5] * m_t[8])  * inv_det; 
	result->m_t[9]  = -( m_t[0] * m_t[9] - m_t[1] * m_t[8])  * inv_det; 
	result->m_t[10] = h0514  * inv_det; 
	result->m_t[11] = -(h0514*m_t[11] + h3407 *m_t[9] + h1735 *m_t[8])  * inv_det;

	return result; 
}


C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size, std::vector<double> transform, 
						 const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(transform),
	m_size(size)
{
}

size_t C3DAffineTransformation::degrees_of_freedom() const
{
	return 12;
}

void C3DAffineTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

CDoubleVector C3DAffineTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	copy(m_t.begin(), m_t.end(), result.begin());
	return result;
}

void C3DAffineTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), m_t.begin());

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
	return m_t[0] + m_t[1] + m_t[3] + m_t[4] - 2.0f;
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
	return m_t[1] + m_t[4] - m_t[0] - m_t[3];
}

const C3DBounds& C3DAffineTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DAffineTransformation::do_upscale(const C3DBounds& size) const
{
	float x_mult = float(size.x) / (float)get_size().x;
	float y_mult = float(size.y) / (float)get_size().y;
	float z_mult = float(size.z) / (float)get_size().z;

	C3DAffineTransformation *result = new C3DAffineTransformation(*this);
	result->m_size   = size;
	result->m_t[3]  *= x_mult;
	result->m_t[7]  *= y_mult;
	result->m_t[11] *= z_mult;

	return P3DTransformation(result);
}


C3DFMatrix C3DAffineTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	return C3DFMatrix(C3DFVector(m_t[0], m_t[1], m_t[2]),
			  C3DFVector(m_t[4], m_t[5], m_t[6]),
			  C3DFVector(m_t[8], m_t[9], m_t[10]));
}

C3DFMatrix C3DAffineTransformation::derivative_at(int /*x*/, int /*y*/, int /*z*/) const
{
	return C3DFMatrix(C3DFVector(m_t[0], m_t[1], m_t[2]),
			  C3DFVector(m_t[4], m_t[5], m_t[6]),
			  C3DFVector(m_t[8], m_t[9], m_t[10]));
}

void C3DAffineTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	fill(m_t.begin(), m_t.end(), 0.0);
	m_t[0] = m_t[5] = m_t[10] = 1.0;
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


C3DFVector C3DAffineTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DAffineTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAffineTransformation doesn't implement a jacobian."); 
}

void C3DAffineTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
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



C3DAffineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DAffineTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DAffineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						      const C3DBounds& end, const C3DBounds& size, 
						      const C3DAffineTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DTransformation::iterator_impl * C3DAffineTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DAffineTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DAffineTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DAffineTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DAffineTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}


C3DTransformation::const_iterator C3DAffineTransformation::begin() const
{
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAffineTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAffineTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAffineTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}



float C3DAffineTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAffineTransformation doesn't implement pertuberate."); 
}

class C3DAffineTransformCreator: public C3DTransformCreator {
public: 
	C3DAffineTransformCreator(const C3DInterpolatorFactory& ipf); 
private: 
	virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
};

C3DAffineTransformCreator::C3DAffineTransformCreator(const C3DInterpolatorFactory& ipf):
	C3DTransformCreator(ipf) 
{
}

P3DTransformation C3DAffineTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DAffineTransformation(size, ipf));
}

class C3DAffineTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	C3DAffineTransformCreatorPlugin(); 
	virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
};

C3DAffineTransformCreatorPlugin::C3DAffineTransformCreatorPlugin():
	C3DTransformCreatorPlugin("affine")
{
}

C3DTransformCreator *C3DAffineTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DAffineTransformCreator(ipf);
}

const std::string C3DAffineTransformCreatorPlugin::do_get_descr() const
{
	return "Affine transformation (12 degrees of freedom)";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DAffineTransformCreatorPlugin();
}


NS_MIA_END
