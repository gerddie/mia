/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/* 
  LatexBeginPluginDescription{3D Transformations}
  
   \subsection{Affine}
   \label{transform3d:affine}
   
   \begin{description}
   
   \item [Plugin:] affine
   \item [Description:] Affine-linear transformations - i.e. the transformation cann be described in terms of a 
   multiplication by a $2\times2$ matrix $A$ and the addition of a translation vector $b$: 
   \begin{equation}
   x \rightarrow Ax + b
   \end{equation}
   
   \item [Degrees of Freedom:] 12
  
   \end{description}
   This plug-in doesn't take parameters 

   LatexEnd  
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

C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size):
	m_t(12,0.0f),
	m_size(size)
{
	m_t[0] = m_t[5] = m_t[10] =  1.0;
}

C3DAffineTransformation::C3DAffineTransformation(const C3DAffineTransformation& other):
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
		THROW(invalid_argument, "C3DAffineTransformation::invert(): Matrix is singular"); 
	
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


C3DAffineTransformation::C3DAffineTransformation(const C3DBounds& size,
						 std::vector<double> transform):
	m_t(transform),
	m_size(size)
{
}

bool C3DAffineTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 3D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 12 ; ++i)
		file << m_t[i] << " ";
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
	m_t[0] *= expx;
	m_t[1] *= expx;
	m_t[2] *= expx;
	m_t[3] *= expx;
	m_t[4] *= expy;
	m_t[5] *= expy;
	m_t[6] *= expy;
	m_t[7] *= expy;
	m_t[8] *= expz;
	m_t[9] *= expz;
	m_t[10] *= expz;
	m_t[11] *= expz;


}

void C3DAffineTransformation::translate(float x, float y, float z)
{
	m_t[ 3] +=  x;
	m_t[ 7] +=  y;
	m_t[11] +=  z;
}

void C3DAffineTransformation::rotate(float angle)
{
	assert(0 && "Eliminate function"); 
	const double sina = sin(angle);
	const double cosa = cos(angle);

	const double tx      = cosa * m_t[2] - sina * m_t[5];
	m_t[5] = sina * m_t[2] + cosa * m_t[5];
	m_t[2] = tx;

	const double a = m_t[0] * cosa - sina * m_t[3];
	const double b = m_t[1] * cosa - sina * m_t[4];
	const double c = m_t[0] * sina + cosa * m_t[3];
	const double d = m_t[1] * sina + cosa * m_t[4];

	m_t[0] = a;
	m_t[1] = b;
	m_t[3] = c;
	m_t[4] = d;

}


void C3DAffineTransformation::shear(float /*v*/)
{
	assert(0 && "not implemented");
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

void C3DAffineTransformation::add(const C3DTransformation& other)
{
	// *this  = other * *this
	const C3DAffineTransformation& a = dynamic_cast<const C3DAffineTransformation&>(other);

	vector<double> h(m_t.size());

	h[0] = a.m_t[0] * m_t[0] + a.m_t[1] * m_t[4] + a.m_t[2] * m_t[8];
	h[1] = a.m_t[0] * m_t[1] + a.m_t[1] * m_t[5] + a.m_t[2] * m_t[9];
	h[2] = a.m_t[0] * m_t[2] + a.m_t[1] * m_t[6] + a.m_t[2] * m_t[10];
	h[3] = a.m_t[0] * m_t[3] + a.m_t[1] * m_t[7] + a.m_t[2] * m_t[11] + a.m_t[3];

	h[4] = a.m_t[4] * m_t[0] + a.m_t[5] * m_t[4] + a.m_t[6] * m_t[8];
	h[5] = a.m_t[4] * m_t[1] + a.m_t[5] * m_t[5] + a.m_t[6] * m_t[9];
	h[6] = a.m_t[4] * m_t[2] + a.m_t[5] * m_t[6] + a.m_t[6] * m_t[10];
	h[7] = a.m_t[4] * m_t[3] + a.m_t[5] * m_t[7] + a.m_t[6] * m_t[11] + a.m_t[7];

	h[8]  = a.m_t[8] * m_t[0] + a.m_t[9] * m_t[4] + a.m_t[10] * m_t[8];
	h[9]  = a.m_t[8] * m_t[1] + a.m_t[9] * m_t[5] + a.m_t[10] * m_t[9];
	h[10] = a.m_t[8] * m_t[2] + a.m_t[9] * m_t[6] + a.m_t[10] * m_t[10];
	h[11] = a.m_t[8] * m_t[3] + a.m_t[9] * m_t[7] + a.m_t[10] * m_t[11] + a.m_t[11];

	copy(h.begin(), h.end(), m_t.begin());
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

C3DTransformation::iterator_impl * C3DAffineTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), m_trans); 
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
	virtual C3DTransformCreator *do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C3DAffineTransformCreatorPlugin::C3DAffineTransformCreatorPlugin():
	C3DTransformCreatorPlugin("affine")
{
}

C3DTransformCreator *C3DAffineTransformCreatorPlugin::do_create() const
{
	return new C3DAffineTransformCreator();
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
