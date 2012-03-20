/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
#include <mia/core/msgstream.hh>
#include <mia/core/utils.hh>
#include <mia/3d/transformfactory.hh>

#include <mia/3d/transform/rigid.hh>

NS_MIA_BEGIN
using namespace std;


C3DFVector C3DRigidTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DRigidTransformation::transform(const C3DFVector& x)const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C3DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2] * x.z + m_t[3],
		m_t[4] * x.x + m_t[5] * x.y + m_t[6] * x.z + m_t[7],
		m_t[8] * x.x + m_t[9] * x.y + m_t[10] * x.z + m_t[11]);
}

C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(12),
	m_size(size),
	m_translation(0.0, 0.0, 0.0),
	m_rotation(0.0,0.0,0.0),
	m_matrix_valid(false)
{
}

C3DRigidTransformation::C3DRigidTransformation(const C3DRigidTransformation& other):
	C3DTransformation(other), 
	m_t(other.m_t),
	m_size(other.m_size),
	m_translation(other.m_translation),
	m_rotation(other.m_rotation),
	m_matrix_valid(m_matrix_valid)
{
}

C3DTransformation *C3DRigidTransformation::do_clone()const
{
	return new C3DRigidTransformation(*this);
}

C3DTransformation *C3DRigidTransformation::invert()const
{
	
	C3DRigidTransformation *result = new C3DRigidTransformation(*this); 
	cverr() << "to be implemented\n"; 
	result->m_matrix_valid = false; 
	return result;
}


C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size,const C3DFVector& translation,
					       const C3DFVector& rotation, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(12),
	m_size(size),
	m_translation(translation),
	m_rotation(rotation),
	m_matrix_valid(false)
{
}

bool C3DRigidTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 3D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 12 ; ++i)
		file << m_t[i] << " ";
	file << "\n";
	return file.good();
}

size_t C3DRigidTransformation::degrees_of_freedom() const
{
	return 6;
}

void C3DRigidTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C3DRigidTransformation::translate(float x, float y, float z)
{
	m_matrix_valid = false;
	m_translation.x +=  x;
	m_translation.y +=  y;
	m_translation.z +=  z;
}

void C3DRigidTransformation::rotate(float xy, float xz, float yz)
{
	m_matrix_valid = false;
	m_rotation.z += xy;
	m_rotation.y += xz;
	m_rotation.x += yz;
	float sx,cx,sy,cy,sz,cz; 
	sincosf(m_rotation.z, &sz, &cz); 
	sincosf(m_rotation.y, &sy, &cy);
	sincosf(m_rotation.x, &sx, &cx);

	C3DFVector t(cy*cz * m_translation.x 
		     - (cx*sz-cz*sx*sy) * m_translation.y 
		     + (sx*sz-cx*cz*sy) * m_translation.z, 
		     cy*sz * m_translation.x 
		     + (cx*cz-sx*sy*sz) * m_translation.y 
		     - (cx*sy*sz+cz*sx) * m_translation.z, 
		     sy* m_translation.x  
		     + cy*sx  * m_translation.y + cx*cy * m_translation.z); 
	
	m_translation = t;
}

CDoubleVector C3DRigidTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	result[0] = m_translation.x;
	result[1] = m_translation.y;
	result[2] = m_translation.z;
	result[3] = m_rotation.x;
	result[4] = m_rotation.y;
	result[5] = m_rotation.z;

	return result;
}

void C3DRigidTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());

	m_translation.x = params[0];
	m_translation.y = params[1];
	m_translation.z = params[2];
	m_rotation.x    = params[3];
	m_rotation.y    = params[4];
	m_rotation.z    = params[5];
	cvdebug() << "Rigid transform = (" << m_translation << ", " << m_rotation << ")\n"; 

	m_matrix_valid = false;
}

float C3DRigidTransformation::divergence() const
{
	return 0.0;
}

float C3DRigidTransformation::grad_divergence() const
{
	return 0.0;
}


float C3DRigidTransformation::grad_curl() const
{
	return 0.0;
}


float C3DRigidTransformation::curl() const
{
	// this is not right
	return 0.0;
}

double C3DRigidTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/, CDoubleVector& /*gradient*/) const
{
	return 0.0; 
}

double C3DRigidTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/) const
{
	return 0.0; 
}


const C3DBounds& C3DRigidTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DRigidTransformation::do_upscale(const C3DBounds& size) const
{
	C3DFVector new_trans(float(size.x) / (float)get_size().x * m_translation.x,
			     float(size.y) / (float)get_size().y * m_translation.y,
			     float(size.z) / (float)get_size().z * m_translation.z);
	return P3DTransformation(new C3DRigidTransformation(size, new_trans, m_rotation, get_interpolator_factory()));
}

C3DFMatrix C3DRigidTransformation::derivative_at(int /*x*/, int /*y*/, int /* y */) const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C3DFMatrix(
		C3DFVector(m_t[0], m_t[4], m_t[8]), 
		C3DFVector(m_t[1], m_t[5], m_t[9]), 
		C3DFVector(m_t[2], m_t[6], m_t[10]));
}

void C3DRigidTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_translation = C3DFVector(); 
	m_rotation    = C3DFVector(); 
}

void C3DRigidTransformation::evaluate_matrix() const
{
	CScopedLock lock(m_mutex); 
	
	float sx,cx,sy,cy,sz,cz; 
	sincosf(m_rotation.z, &sz, &cz); 
	sincosf(m_rotation.y, &sy, &cy);
	sincosf(m_rotation.x, &sx, &cx);

	m_t[0] =    cy*cz;
	m_t[1] =  - cx*sz-cz*sx*sy; 
	m_t[2] =    sx*sz-cx*cz*sy;
	m_t[3] = m_translation.x;
	
	m_t[4] = cy*sz;
	m_t[5] = cx*cz-sx*sy*sz;
	m_t[6] = - cx*sy*sz-cz*sx;
	m_t[7] = m_translation.y; 

	m_t[8] = sy;
	m_t[9] = cy*sx;
	m_t[10]= cx*cy;
	m_t[11]= m_translation.z; 

	m_matrix_valid = true;
}

float C3DRigidTransformation::get_max_transform() const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
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

void C3DRigidTransformation::add(const C3DTransformation& /*other*/)
{
	// *this  = other * *this
	assert(0 && "not implemented");
}

C3DFVector C3DRigidTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DRigidTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C3DRigidTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	//

	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t z = 0; z < m_size.z; ++z)
		for (size_t y = 0; y < m_size.y; ++y)
			for (size_t x = 0; x < m_size.x; ++x, ++g) {
				r[0] += g->x;
				r[1] += g->y;
				r[2] += g->z;
				r[3] += -z * g->y + y * g->z; 
				r[4] += -z * g->x + x * g->z; 
				r[5] += -y * g->x + x * g->y; 
			}
	copy(r.begin(), r.end(), params.begin()); 
}



C3DRigidTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DRigidTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.transform(C3DFVector(pos)))
{
	m_dx = m_trans.transform(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DRigidTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						     const C3DBounds& end, const C3DBounds& size, 
						     const C3DRigidTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.transform(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}
		
C3DTransformation::iterator_impl * C3DRigidTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DRigidTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DRigidTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DRigidTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.transform(C3DFVector(get_pos())); 
	m_dx = m_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DRigidTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.transform(C3DFVector(get_pos())); 
	m_dx = m_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

C3DTransformation::const_iterator C3DRigidTransformation::begin() const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}


float C3DRigidTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

class C3DRigidTransformCreator: public C3DTransformCreator {
public: 
	C3DRigidTransformCreator(const C3DInterpolatorFactory& ipf); 
private: 
	virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
};

C3DRigidTransformCreator::C3DRigidTransformCreator(const C3DInterpolatorFactory& ipf):
	C3DTransformCreator(ipf)
{
}

P3DTransformation C3DRigidTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DRigidTransformation(size, ipf));
}

class C3DRigidTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	C3DRigidTransformCreatorPlugin();
	virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C3DRigidTransformCreatorPlugin::C3DRigidTransformCreatorPlugin():
	C3DTransformCreatorPlugin("rigid")
{
}

C3DTransformCreator *C3DRigidTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DRigidTransformCreator(ipf);
}

bool C3DRigidTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C3DRigidTransformCreatorPlugin::do_get_descr() const
{
	return "Rigid transformation, i.e. rotation and translation (six degrees of freedom).";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DRigidTransformCreatorPlugin();
}


NS_MIA_END
