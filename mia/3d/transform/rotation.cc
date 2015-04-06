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

#include <fstream>
#include <mia/core/msgstream.hh>
#include <mia/core/utils.hh>
#include <mia/3d/transformfactory.hh>

#include <mia/3d/transform/rotation.hh>

NS_MIA_BEGIN
using namespace std;

C3DRotationTransformation::C3DRotationTransformation(const C3DBounds& size, const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(9),
	m_size(size),
	m_rotation(0.0,0.0,0.0),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	initialize(); 
}

C3DRotationTransformation::C3DRotationTransformation(const C3DRotationTransformation& other):
	C3DTransformation(other), 
	m_t(other.m_t),
	m_size(other.m_size),
	m_rotation(other.m_rotation),
	m_relative_rot_center(other.m_relative_rot_center), 
	m_rot_center(other.m_rot_center), 
	m_matrix_valid(other.m_matrix_valid)
{
}

C3DRotationTransformation::C3DRotationTransformation(const C3DBounds& size,const C3DFVector& rotation, 
						     const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
	m_t(9),
	m_size(size),
	m_rotation(rotation),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	initialize(); 
}

void C3DRotationTransformation::initialize()
{
	m_rot_center = C3DFVector(m_size - C3DBounds::_1) * m_relative_rot_center; 
}

C3DFVector C3DRotationTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DRotationTransformation::transform(const C3DFVector& x)const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C3DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2] * x.z,
		m_t[3] * x.x + m_t[4] * x.y + m_t[5] * x.z,
		m_t[6] * x.x + m_t[7] * x.y + m_t[8] * x.z) + m_shift;
}


C3DTransformation *C3DRotationTransformation::do_clone()const
{
	return new C3DRotationTransformation(*this);
}

C3DTransformation *C3DRotationTransformation::invert()const
{
	
	C3DRotationTransformation *result = new C3DRotationTransformation(*this); 
	cverr() << "to be implemented\n"; 
	result->m_matrix_valid = false; 
	return result;
}



size_t C3DRotationTransformation::degrees_of_freedom() const
{
	return 3;
}

void C3DRotationTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C3DRotationTransformation::rotate(float xy, float xz, float yz)
{
	m_matrix_valid = false;
	m_rotation.z += xy;
	m_rotation.y += xz;
	m_rotation.x += yz;
	float sx,cx,sy,cy,sz,cz; 
	sincosf(m_rotation.z, &sz, &cz); 
	sincosf(m_rotation.y, &sy, &cy);
	sincosf(m_rotation.x, &sx, &cx);
}

CDoubleVector C3DRotationTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	result[0] = m_rotation.x;
	result[1] = m_rotation.y;
	result[2] = m_rotation.z;

	return result;
}

void C3DRotationTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());

	m_rotation.x    = params[0];
	m_rotation.y    = params[1];
	m_rotation.z    = params[2];
	cvdebug() << "Rotation transform = (" << m_rotation << ")\n"; 

	m_matrix_valid = false;
}

const C3DBounds& C3DRotationTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DRotationTransformation::do_upscale(const C3DBounds& size) const
{
	return P3DTransformation(new C3DRotationTransformation(size, m_rotation, get_interpolator_factory()));
}

C3DFMatrix C3DRotationTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	if (!m_matrix_valid)
		evaluate_matrix();
	return C3DFMatrix(
		C3DFVector(m_t[0], m_t[3], m_t[6]), 
		C3DFVector(m_t[1], m_t[4], m_t[7]), 
		C3DFVector(m_t[2], m_t[5], m_t[8]));
}

C3DFMatrix C3DRotationTransformation::derivative_at(int MIA_PARAM_UNUSED(x), int MIA_PARAM_UNUSED(y), 
						 int MIA_PARAM_UNUSED(z)) const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C3DFMatrix(
		C3DFVector(m_t[0], m_t[3], m_t[6]), 
		C3DFVector(m_t[1], m_t[4], m_t[7]), 
		C3DFVector(m_t[2], m_t[5], m_t[8]));
}

void C3DRotationTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_rotation    = C3DFVector(); 
}

void C3DRotationTransformation::evaluate_matrix() const
{
	CScopedLock lock(m_mutex); 
	
	float sx,cx,sy,cy,sz,cz; 
	sincosf(m_rotation.z, &sz, &cz); 
	sincosf(m_rotation.y, &sy, &cy);
	sincosf(m_rotation.x, &sx, &cx);

	m_t[0] =    cy*cz;
	m_t[1] =  - cx*sz-cz*sx*sy; 
	m_t[2] =    sx*sz-cx*cz*sy;
	
	m_t[3] = cy*sz;
	m_t[4] = cx*cz-sx*sy*sz;
	m_t[5] = - cx*sy*sz-cz*sx;

	m_t[6] = sy;
	m_t[7] = cy*sx;
	m_t[8]= cx*cy;


	m_shift.x = (1.0 - m_t[0])* m_rot_center.x -        m_t[1]  * m_rot_center.y -        m_t[2]  * m_rot_center.z; 
	m_shift.y =      - m_t[3] * m_rot_center.x + (1.0 - m_t[4]) * m_rot_center.y -        m_t[5]  * m_rot_center.z; 
	m_shift.z =      - m_t[6] * m_rot_center.x -        m_t[7]  * m_rot_center.y + (1.0 - m_t[8]) * m_rot_center.z; 

	m_matrix_valid = true;
}

float C3DRotationTransformation::get_max_transform() const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	C3DFVector corners[7] = {
		C3DFVector(get_size().x-1, 0, 0), 
		C3DFVector(get_size().x-1, get_size().y-1,            0), 
		C3DFVector(           0, get_size().y-1,            0), 
		C3DFVector(           0, get_size().y-1, get_size().z-1), 
		C3DFVector(get_size().x-1,            0, get_size().z-1), 
		C3DFVector(           0,            0, get_size().z-1), 
		C3DFVector(get_size()) - C3DFVector::_1
	};

	float result = apply(C3DFVector()).norm2(); 
	for(int i = 0; i < 7; ++i) {
		float h = (apply(corners[i]) - corners[i]).norm2(); 
		if (result < h) 
			result = h; 
	}

	return sqrt(result);

}


C3DFVector C3DRotationTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DRotationTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	// a rotation transformation doesn't introduce a volume change
	return 1.0; 
}

void C3DRotationTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	//

	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	double fz = - m_rot_center.z; 
	for (size_t z = 0; z < m_size.z; ++z, fz += 1.0) {
		double fy = - m_rot_center.y; 
		for (size_t y = 0; y < m_size.y; ++y, fy += 1.0) {
			double fx = - m_rot_center.x; 
			for (size_t x = 0; x < m_size.x; ++x, fx += 1.0, ++g) {
				r[0] += -fz * g->y + fy * g->z; 
				r[1] += -fz * g->x + fx * g->z; 
				r[2] += -fy * g->x + fx * g->y; 
			}
		}
	}
	copy(r.begin(), r.end(), params.begin()); 
}



C3DRotationTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DRotationTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.transform(C3DFVector(pos)))
{
	m_dx = m_trans.transform(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DRotationTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						     const C3DBounds& end, const C3DBounds& size, 
						     const C3DRotationTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.transform(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}
		
C3DTransformation::iterator_impl * C3DRotationTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DRotationTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DRotationTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DRotationTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.transform(C3DFVector(get_pos())); 
	m_dx = m_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DRotationTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.transform(C3DFVector(get_pos())); 
	m_dx = m_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

C3DTransformation::const_iterator C3DRotationTransformation::begin() const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotationTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotationTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	if (!m_matrix_valid) 
		evaluate_matrix(); 
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotationTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}


float C3DRotationTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAffineTransformation doesn't implement pertuberate."); 
}

class C3DRotationTransformCreator: public C3DTransformCreator {
public: 
	C3DRotationTransformCreator(const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf); 
private: 
	virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
	C3DFVector m_relative_rot_center; 
};

C3DRotationTransformCreator::C3DRotationTransformCreator(const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf):
	C3DTransformCreator(ipf), 
	m_relative_rot_center(relative_rot_center)
{
}

P3DTransformation C3DRotationTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DRotationTransformation(size, m_relative_rot_center, ipf));
}

class C3DRotationTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	C3DRotationTransformCreatorPlugin();
private: 
	virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
	C3DFVector m_relative_rot_center; 
};

C3DRotationTransformCreatorPlugin::C3DRotationTransformCreatorPlugin():
	C3DTransformCreatorPlugin("rotation")
{
	add_parameter("origin", make_param(m_relative_rot_center, false, 
					       "Relative rotation center, i.e.  <0.5,0.5,0.5> corresponds "
					       "to the center of the volume"));

}

C3DTransformCreator *C3DRotationTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DRotationTransformCreator(m_relative_rot_center, ipf);
}

const std::string C3DRotationTransformCreatorPlugin::do_get_descr() const
{
	return "Rotation transformation (three degrees of freedom).";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DRotationTransformCreatorPlugin();
}


NS_MIA_END
