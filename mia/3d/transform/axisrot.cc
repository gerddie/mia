/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <cmath>
#include <mia/core/msgstream.hh>
#include <mia/3d/transform/axisrot.hh>

namespace mia_3dtransform_axisrot {

using namespace mia; 
using namespace std;



C3DFVector C3DAxisrotTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DAxisrotTransformation::transform(const C3DFVector& x)const
{
	return m_matrix * x;
}

C3DAxisrotTransformation::C3DAxisrotTransformation(const C3DBounds& size, const C3DFVector& orig, 
						   const C3DFVector& rot_axis, 
						   const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
        m_angle(0.0), 
        m_relative_origin(orig), 
	m_rotation_center(C3DFVector(size) * m_relative_origin), 
	m_rotation_axis(rot_axis), 
	m_size(size), 
	m_y_align_rot_needed(false)
{
	cvinfo() << "Transform: center=" << m_rotation_center << " of size=" << m_size << "\n"; 
	double n = m_rotation_axis.norm(); 
	if (n == 0.0)
		throw invalid_argument("C3DAxisrotTransformation: rotation axis has zero length"); 

	m_rotation_axis /= n; 
	
	// create a pre-post rotation quaternion to get the afs-pfs line into alignment with the 
	// y axis. 

	// one vector is 0,1,0, the other is m_rotation_axis
	C3DFVector help_axis(-m_rotation_axis.z, 0, m_rotation_axis.x); 
	if (help_axis.norm2() > 0.0) {
		help_axis /= help_axis.norm(); 
		m_y_align_rot_needed = true; 
		double angle = acos(m_rotation_axis.y) / 2.0; 
		double sina, cosa; 
		sincos(angle, &sina, &cosa); 
		m_y_align_rot = Quaternion( cosa, sina * help_axis.x, 0, sina * help_axis.z); 
		cvinfo() << "Align-rotation = " << m_y_align_rot << "\n"; 

		m_y_align_rot_inverse = m_y_align_rot.inverse(); 
		
	}
	m_matrix.identity();
}

C3DTransformation *C3DAxisrotTransformation::do_clone()const
{
	return new C3DAxisrotTransformation(*this);
}

C3DTransformation *C3DAxisrotTransformation::invert()const
{
	C3DAxisrotTransformation *result = new C3DAxisrotTransformation(*this);
        
        assert(0 && "not implemented"); 

	return result; 
}

size_t C3DAxisrotTransformation::degrees_of_freedom() const
{
	return 1;
}

void C3DAxisrotTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(0 && "not implemented");
}

CDoubleVector C3DAxisrotTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	result[0] = m_angle;
	return result;
}

void C3DAxisrotTransformation::apply_parameters()
{
	m_matrix.identity(); 
        m_matrix.translate( -1.0f * m_rotation_center); 

	if (m_y_align_rot_needed) {
		m_matrix.rotate(m_y_align_rot); 
	}

	m_matrix.rotate_y(m_angle); 
	
	if (m_y_align_rot_needed) {
		m_matrix.rotate(m_y_align_rot_inverse); 
	}

	
        m_matrix.translate(m_rotation_center); 
}

void C3DAxisrotTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	m_angle = params[0]; 
	apply_parameters(); 
}

const C3DBounds& C3DAxisrotTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DAxisrotTransformation::do_upscale(const C3DBounds& size) const
{
	auto result = new C3DAxisrotTransformation(size, m_relative_origin, 
						   m_rotation_axis, get_interpolator_factory()); 
	result->apply_parameters();
	return P3DTransformation(result);
}


C3DFMatrix C3DAxisrotTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	assert(0 && "not implemented"); 
}

C3DFMatrix C3DAxisrotTransformation::derivative_at(int /*x*/, int /*y*/, int /*z*/) const
{
	assert(0 && "not implemented"); 
}

void C3DAxisrotTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_matrix.identity(); 
}

float C3DAxisrotTransformation::get_max_transform() const
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


C3DFVector C3DAxisrotTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DAxisrotTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAxisrotTransformation doesn't implement a jacobian."); 
}

void C3DAxisrotTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(0 && !"not yet implemented"); 
	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t z = 0; z < m_size.z; ++z) {
		double fy =  - m_rotation_center.y; 
		for (size_t y = 0; y < m_size.y; ++y, fy += 1.0) {
			double fx =  - m_rotation_center.x;
			for (size_t x = 0; x < m_size.x; ++x, ++g, fx += 1.0) {
				r[0] += -fy * g->x + fx * g->y;
			}
		}
	}
	std::copy(r.begin(), r.end(), params.begin());
}



C3DAxisrotTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DAxisrotTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DAxisrotTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						      const C3DBounds& end, const C3DBounds& size, 
						      const C3DAxisrotTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DTransformation::iterator_impl * C3DAxisrotTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DAxisrotTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DAxisrotTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DAxisrotTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DAxisrotTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}


C3DTransformation::const_iterator C3DAxisrotTransformation::begin() const
{
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAxisrotTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAxisrotTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DAxisrotTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}



float C3DAxisrotTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DAxisrotTransformation doesn't implement pertuberate."); 
}

C3DAxisrotTransformCreator::C3DAxisrotTransformCreator(const C3DFVector& origin, 
						       const C3DFVector& rot_axis, 
						       const C3DInterpolatorFactory& ipf):
C3DTransformCreator(ipf), 
	m_origin(origin), 
	m_rotation_axis(rot_axis)
{
}

P3DTransformation C3DAxisrotTransformCreator::do_create(const C3DBounds& size, 
							const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DAxisrotTransformation(size, m_origin, m_rotation_axis, ipf));
}




C3DAxisrotTransformCreatorPlugin::C3DAxisrotTransformCreatorPlugin():
	C3DTransformCreatorPlugin("axisrot")
{
	add_parameter("origin", new C3DFVectorParameter(m_origin, true, "center of the transformation"));
	add_parameter("axis", new C3DFVectorParameter(m_rotation_axis, true, "rotation axis"));
}


C3DTransformCreator *C3DAxisrotTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DAxisrotTransformCreator(m_origin, m_rotation_axis, ipf);
}

const std::string C3DAxisrotTransformCreatorPlugin::do_get_descr() const
{
	return "Restricted rotation transformation (1 degrees of freedom). "
		"The transformation is restricted to the rotation around the given "
		"axis about the given rotation center";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DAxisrotTransformCreatorPlugin();
}



}
