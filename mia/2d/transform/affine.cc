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
#include <mia/core/msgstream.hh>
#include <mia/2d/transformfactory.hh>

#include <mia/2d/transform/affine.hh>


NS_MIA_BEGIN
using namespace boost::lambda;
using namespace std;

P2DImage C2DAffineTransformation::apply(const C2DImage& image,
					const C2DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C2DAffineTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform2d(image, ipf, *this);
}

C2DFVector C2DAffineTransformation::apply(const C2DFVector& x) const
{
	return transform(x);
}



C2DFVector C2DAffineTransformation::transform(const C2DFVector& x)const
{
	return C2DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2],
		m_t[3] * x.x + m_t[4] * x.y + m_t[5]
			  );
}

C2DAffineTransformation::C2DAffineTransformation(const C2DBounds& size):
	m_t(6,0.0f),
	m_size(size)
{
	m_t[0] = m_t[4] = 1.0;
}

C2DAffineTransformation::C2DAffineTransformation(const C2DAffineTransformation& other):
	m_t(other.m_t),
	m_size(other.m_size)
{
}

C2DTransformation *C2DAffineTransformation::do_clone()const
{
	return new C2DAffineTransformation(*this);
}

C2DTransformation *C2DAffineTransformation::invert()const
{
	const double det = m_t[0] * m_t[4] - m_t[1] * m_t[3]; 
	if (fabs(det) < 1e-6) 
		THROW(invalid_argument, "C2DAffineTransformation::invert(): Matrix is singular"); 
	
	const double inv_det = 1.0 / det; 

	C2DAffineTransformation *result = new C2DAffineTransformation(*this);
	
	result->m_t[0] = m_t[4] * inv_det; 
	result->m_t[1] = - m_t[1] * inv_det; 
	result->m_t[2] = (m_t[1] * m_t[5] - m_t[2] * m_t[4])* inv_det; 

	result->m_t[3] = - m_t[3] * inv_det; 
	result->m_t[4] = m_t[0] * inv_det; 
	result->m_t[5] = (m_t[2] * m_t[3] - m_t[0] * m_t[5])* inv_det; 
		
	return result; 
}


C2DAffineTransformation::C2DAffineTransformation(const C2DBounds& size,
						 std::vector<double> transform):
	m_t(transform),
	m_size(size)
{
}

bool C2DAffineTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 2D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 6 ; ++i)
		file << m_t[i] << " ";
	file << "\n";
	return file.good();
}

size_t C2DAffineTransformation::degrees_of_freedom() const
{
	return 6;
}

void C2DAffineTransformation::update(float /*step*/, const C2DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C2DAffineTransformation::scale(float x, float y)
{
	const double expx = exp(x);
	const double expy = exp(y);
	m_t[0] *= expx;
	m_t[1] *= expx;
	m_t[2] *= expx;
	m_t[3] *= expy;
	m_t[4] *= expy;
	m_t[5] *= expy;
}

void C2DAffineTransformation::translate(float x, float y)
{
	m_t[2] +=  x;
	m_t[5] +=  y;
}

void C2DAffineTransformation::rotate(float angle)
{
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


void C2DAffineTransformation::shear(float /*v*/)
{
	assert(0 && "not implemented");
}

CDoubleVector C2DAffineTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	copy(m_t.begin(), m_t.end(), result.begin());
	return result;
}

void C2DAffineTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), m_t.begin());

}

double C2DAffineTransformation::get_divcurl_cost(double, double, CDoubleVector&) const
{
	return 0.0; 
}

double C2DAffineTransformation::get_divcurl_cost(double, double) const
{
	return 0.0; 
}


float C2DAffineTransformation::divergence() const
{
	return m_t[0] + m_t[1] + m_t[3] + m_t[4] - 2.0f;
}

float C2DAffineTransformation::grad_divergence() const
{
	return 0.0;
}


float C2DAffineTransformation::grad_curl() const
{
	return 0.0;
}


float C2DAffineTransformation::curl() const
{
	return m_t[1] + m_t[4] - m_t[0] - m_t[3];
}

const C2DBounds& C2DAffineTransformation::get_size() const
{
	return m_size;
}

P2DTransformation C2DAffineTransformation::do_upscale(const C2DBounds& size) const
{
	float x_mult = float(size.x) / (float)get_size().x;
	float y_mult = float(size.y) / (float)get_size().y;

	C2DAffineTransformation *result = new C2DAffineTransformation(*this);
	result->m_size = size;
	result->m_t[3] *= x_mult;
	result->m_t[5] *= y_mult;

	return P2DTransformation(result);
}

C2DFMatrix C2DAffineTransformation::derivative_at(int /*x*/, int /*y*/) const
{
	return C2DFMatrix(C2DFVector(m_t[0], m_t[1]),
			  C2DFVector(m_t[3], m_t[4]));
}

void C2DAffineTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	fill(m_t.begin(), m_t.end(), 0.0);
	m_t[0] = m_t[4] = 1.0;
}

float C2DAffineTransformation::get_max_transform() const
{
	// check the corners
	float m =      (apply(C2DFVector(get_size())) - C2DFVector(get_size())).norm2();
	float test0Y = (apply(C2DFVector(0, get_size().y)) - C2DFVector(0, get_size().y)).norm2();
	float testX0 = (apply(C2DFVector(get_size().x, 0)) - C2DFVector(get_size().x, 0)).norm2();
	float test00 = (apply(C2DFVector(0, 0)) - C2DFVector(0, 0)).norm2();

	if (m < test0Y)
		m = test0Y;

	if (m < testX0)
		m = testX0;

	if (m < test00)
		m = test00;
	return sqrt(m);
}

void C2DAffineTransformation::add(const C2DTransformation& other)
{
	// *this  = other * *this
	const C2DAffineTransformation& a = dynamic_cast<const C2DAffineTransformation&>(other);

	vector<double> h(m_t.size());

	h[0] = a.m_t[0] * m_t[0] + a.m_t[1] * m_t[3];
	h[1] = a.m_t[0] * m_t[1] + a.m_t[1] * m_t[4];
	h[2] = a.m_t[0] * m_t[2] + a.m_t[1] * m_t[5] + a.m_t[2];

	h[3] = a.m_t[3] * m_t[0] + a.m_t[4] * m_t[3];
	h[4] = a.m_t[3] * m_t[1] + a.m_t[4] * m_t[4];
	h[5] = a.m_t[3] * m_t[2] + a.m_t[4] * m_t[5] + a.m_t[5];

	copy(h.begin(), h.end(), m_t.begin());
}


C2DFVector C2DAffineTransformation::operator () (const C2DFVector& x) const
{
	return apply(x); 
}

float C2DAffineTransformation::get_jacobian(const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C2DAffineTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t y = 0; y < m_size.y; ++y) {
		for (size_t x = 0; x < m_size.x; ++x, ++g) {
			r[0] += x * g->x;
			r[1] += y * g->x;
			r[2] += g->x;
			r[3] += x * g->y;
			r[4] += y * g->y;
			r[5] += g->y;
		}
	}
	std::copy(r.begin(), r.end(), params.begin());
}



C2DAffineTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						      const C2DAffineTransformation& trans):
	C2DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C2DFVector(pos)))
{
	m_dx = m_trans.apply(C2DFVector(pos.x + 1.0, pos.y)) - m_value;
}

C2DTransformation::iterator_impl * C2DAffineTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), m_trans); 
}

const C2DFVector&  C2DAffineTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C2DAffineTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C2DAffineTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.apply(C2DFVector(get_pos())); 
	m_dx = m_trans.apply(C2DFVector(get_pos().x + 1.0, get_pos().y)) - m_value;
}


C2DTransformation::const_iterator C2DAffineTransformation::begin() const
{
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), get_size(), *this)); 
}

C2DTransformation::const_iterator C2DAffineTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}


float C2DAffineTransformation::pertuberate(C2DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

class C2DAffineTransformCreator: public C2DTransformCreator {
	virtual P2DTransformation do_create(const C2DBounds& size) const;
};

P2DTransformation C2DAffineTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DAffineTransformation(size));
}

class C2DAffineTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DAffineTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C2DAffineTransformCreatorPlugin::C2DAffineTransformCreatorPlugin():
	C2DTransformCreatorPlugin("affine")
{
}

C2DAffineTransformCreatorPlugin::ProductPtr
C2DAffineTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C2DAffineTransformCreator());
}

bool C2DAffineTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DAffineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create affine transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DAffineTransformCreatorPlugin();
}


NS_MIA_END
