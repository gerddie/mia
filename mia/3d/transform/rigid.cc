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

#include <fstream>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <mia/core/msgstream.hh>
#include <mia/core/utils.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/transform/rigid.hh>

NS_MIA_BEGIN
using namespace std;


C3DFVector C3DRigidTransformation::apply(const C3DFVector& x) const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	lock.release(); 
	return C3DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2]  * x.z + m_t[3],
		m_t[4] * x.x + m_t[5] * x.y + m_t[6]  * x.z + m_t[7],
		m_t[8] * x.x + m_t[9] * x.y + m_t[10] * x.z + m_t[11]);

}


C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size, const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(12),
	m_size(size),
	m_translation(0.0, 0.0, 0.0),
	m_rotation(0.0,0.0,0.0),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	m_rot_center.x = relative_rot_center.x * (m_size.x - 1); 
	m_rot_center.y = relative_rot_center.y * (m_size.y - 1); 
	m_rot_center.z = relative_rot_center.z * (m_size.z - 1); 
}

C3DRigidTransformation::C3DRigidTransformation(const C3DRigidTransformation& other):
	C3DTransformation(other), 
	m_t(other.m_t),
	m_size(other.m_size),
	m_translation(other.m_translation),
	m_rotation(other.m_rotation),
	m_relative_rot_center(other.m_relative_rot_center), 
	m_rot_center(other.m_rot_center),
	m_matrix_valid(other.m_matrix_valid)
{
}

C3DTransformation *C3DRigidTransformation::do_clone()const
{
	return new C3DRigidTransformation(*this);
}

C3DTransformation *C3DRigidTransformation::invert()const
{
	assert(0 && "C3DRigidTransformation::invert is not yet implemented"); 

	C3DRigidTransformation *result = new C3DRigidTransformation(*this); 
	result->m_matrix_valid = false; 
	return result;
}


C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size,const C3DFVector& translation,
					       const C3DFVector& rotation, const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(12),
	m_size(size),
	m_translation(translation),
	m_rotation(rotation),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	m_rot_center.x = relative_rot_center.x * (m_size.x - 1); 
	m_rot_center.y = relative_rot_center.y * (m_size.y - 1); 
	m_rot_center.z = relative_rot_center.z * (m_size.z - 1); 
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
	CScopedLock lock(m_mutex); 
	m_matrix_valid = false;
	m_translation.x +=  x;
	m_translation.y +=  y;
	m_translation.z +=  z;
}

void C3DRigidTransformation::rotate(float xy, float xz, float yz)
{
	CScopedLock lock(m_mutex); 
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
	CScopedLock lock(m_mutex); 
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

const C3DBounds& C3DRigidTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DRigidTransformation::do_upscale(const C3DBounds& size) const
{
	C3DFVector new_trans(float(size.x) / (float)get_size().x * m_translation.x,
			     float(size.y) / (float)get_size().y * m_translation.y,
			     float(size.z) / (float)get_size().z * m_translation.z);
	return P3DTransformation(new C3DRigidTransformation(size, new_trans, m_rotation, m_relative_rot_center, get_interpolator_factory()));
}

C3DFMatrix C3DRigidTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid)
		evaluate_matrix();
	lock.release(); 

	return C3DFMatrix(
		C3DFVector(m_t[0], m_t[4], m_t[8]), 
		C3DFVector(m_t[1], m_t[5], m_t[9]), 
		C3DFVector(m_t[2], m_t[6], m_t[10]));
}

C3DFMatrix C3DRigidTransformation::derivative_at(int MIA_PARAM_UNUSED(x), int MIA_PARAM_UNUSED(y), 
						 int MIA_PARAM_UNUSED(z)) const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid)
		evaluate_matrix();
	lock.release(); 

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
	
	float sx,cx,sy,cy,sz,cz; 
	sincosf(m_rotation.z, &sz, &cz); 
	sincosf(m_rotation.y, &sy, &cy);
	sincosf(m_rotation.x, &sx, &cx);

	m_t[0] =    cy*cz;
	m_t[1] =  - cx*sz-cz*sx*sy; 
	m_t[2] =    sx*sz-cx*cz*sy;

	
	m_t[4] = cy*sz;
	m_t[5] = cx*cz-sx*sy*sz;
	m_t[6] = - cx*sy*sz-cz*sx;


	m_t[8] = sy;
	m_t[9] = cy*sx;
	m_t[10]= cx*cy;

	C3DFVector center(
		(1 - m_t[0]) * m_rot_center.x      - m_t[1]  * m_rot_center.y      - m_t[2]   * m_rot_center.z, 
		   - m_t[4]  * m_rot_center.x + (1 - m_t[5]) * m_rot_center.y      - m_t[6]   * m_rot_center.z, 
		   - m_t[8]  * m_rot_center.x -      m_t[9]  * m_rot_center.y + (1 - m_t[10]) * m_rot_center.z
		);

	m_t[3] = m_translation.x + center.x;
	m_t[7] = m_translation.y + center.y; 
	m_t[11]= m_translation.z + center.z; 

	m_matrix_valid = true;
}

float C3DRigidTransformation::get_max_transform() const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	lock.release(); 

	C3DFVector corners[8] = {
		C3DFVector(             0,              0,              0), 
		C3DFVector(             0, get_size().y-1,              0), 
		C3DFVector(             0,              0, get_size().z-1), 
		C3DFVector(             0, get_size().y-1, get_size().z-1), 
		C3DFVector(get_size().x-1,              0,              0), 
		C3DFVector(get_size().x-1, get_size().y-1,              0), 
		C3DFVector(get_size().x-1,              0, get_size().z-1), 
		C3DFVector(get_size().x-1, get_size().y-1, get_size().z-1), 
	};

	float result = apply(C3DFVector()).norm2(); 
	for(int i = 0; i < 8; ++i) {
		float h = (apply(corners[i]) - corners[i]).norm2(); 
		if (result < h) 
			result = h; 
	}

	return sqrt(result);

}

C3DFVector C3DRigidTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DRigidTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	// a rigid transformation doesn't introduce a volume change
	return 1.0; 
}

void C3DRigidTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	typedef vector<double> dvect; 
	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	auto sumslice = [&gradient, this] 
		(const tbb::blocked_range<unsigned int>& range, dvect ls)->dvect{
		
		double fz = range.begin() - m_rot_center.z; 
		for (unsigned int z = range.begin(); z != range.end();++z, fz += 1.0) {
			auto g = gradient.begin_at(0,0,z);
			double fy =  - m_rot_center.y; 
			for (size_t y = 0; y < m_size.y; ++y, fy += 1.0) {
				double fx =  - m_rot_center.x; 
				for (size_t x = 0; x < m_size.x; ++x, fx += 1.0, ++g) {
					ls[0] += g->x;
					ls[1] += g->y;
					ls[2] += g->z;
					ls[3] += -fz * g->y + fy * g->z;
					ls[4] += -fz * g->x + fx * g->z;
					ls[5] += -fy * g->x + fx * g->y;
				}
			}
		}
		return ls; 
	}; 

	auto sum_parts = [] (const dvect& a, const dvect& b) -> dvect {
		dvect result(a.size()); 
		std::transform(a.begin(), a.end(), b.begin(), result.begin(), 
			  [](double x, double y) { return x+y;}); 
		return result; 
	}; 
	
	dvect init(params.size(), 0.0); 
	auto sparams = tbb::parallel_reduce( tbb::blocked_range<unsigned int>(0, m_size.z, 1), init, sumslice, sum_parts); 
	std::copy(sparams.begin(), sparams.end(), params.begin()); 
}

C3DRigidTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DRigidTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DRigidTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						     const C3DBounds& end, const C3DBounds& size, 
						     const C3DRigidTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
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
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DRigidTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

C3DTransformation::const_iterator C3DRigidTransformation::begin() const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	lock.release(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	CScopedLock lock(m_mutex); 
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	lock.release(); 
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}


float C3DRigidTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAffineTransformation doesn't implement pertuberate."); 
}

class C3DRigidTransformCreator: public C3DTransformCreator {
public: 
	C3DRigidTransformCreator(const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf); 
private: 
	virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
	C3DFVector m_relative_rot_center; 
};

C3DRigidTransformCreator::C3DRigidTransformCreator(const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformCreator(ipf), 
	m_relative_rot_center(relative_rot_center)
{
}

P3DTransformation C3DRigidTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DRigidTransformation(size, m_relative_rot_center, ipf));
}

class C3DRigidTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	C3DRigidTransformCreatorPlugin();
private: 
	virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
	C3DFVector m_relative_rot_center; 
};

C3DRigidTransformCreatorPlugin::C3DRigidTransformCreatorPlugin():
	C3DTransformCreatorPlugin("rigid"), 
	m_relative_rot_center(0,0,0)
{
	add_parameter("rot-center", make_param(m_relative_rot_center, false, "Relative rotation center, i.e.  <0.5,0.5,0.5> corresponds "
					       "to the center of the volume"));
}

C3DTransformCreator *C3DRigidTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DRigidTransformCreator(m_relative_rot_center, ipf);
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
