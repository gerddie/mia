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
#include <mia/3d/transform/rotbend.hh>

namespace mia_3dtransform_rotbend {

using namespace mia; 
using namespace std;



C3DFVector C3DRotBendTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DRotBendTransformation::transform(const C3DFVector& x)const
{
	auto y = m_pre_matrix * x; 

	if (y.x > 0) {
		const float max_distance = get_size().x - m_rotation_center.x; 
		y.z += y.x * y.x * m_params[2] / (max_distance * max_distance + 1); 
	} 
	else if (y.x < 0) {
		const float max_distance = m_rotation_center.x; 
		y.z += y.x * y.x * m_params[3] / (max_distance * max_distance + 1); 
	}

	return m_post_matrix * y;
}

C3DRotBendTransformation::C3DRotBendTransformation(const C3DBounds& size, const C3DFVector& orig, 
						   const C3DInterpolatorFactory& ipf):
	C3DTransformation(ipf), 
        m_params(4), 
        m_relative_origin(orig), 
	m_rotation_center(C3DFVector(size) * m_relative_origin), 
	m_size(size)
{
	m_pre_matrix.identity();
	m_post_matrix.identity();
}

C3DTransformation *C3DRotBendTransformation::do_clone()const
{
	return new C3DRotBendTransformation(*this);
}

C3DTransformation *C3DRotBendTransformation::invert()const
{
	C3DRotBendTransformation *result = new C3DRotBendTransformation(*this);
        
        assert(0 && "not implemented"); 

	return result; 
}

size_t C3DRotBendTransformation::degrees_of_freedom() const
{
	return m_params.size();
}

void C3DRotBendTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(0 && "not implemented");
}

CDoubleVector C3DRotBendTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	copy(m_params.begin(), m_params.end(), result.begin());
	return result;
}

void C3DRotBendTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	copy(params.begin(), params.end(), m_params.begin());
	cvmsg() <<"Params: "<< m_params << "\n"; 
         
        m_pre_matrix.identity(); 
	m_post_matrix.identity(); 
	m_pre_matrix.translate( -1.0f * m_rotation_center); 

	m_pre_matrix.rotate_x(m_params[0]); 
	m_pre_matrix.rotate_y(m_params[1]); 

        m_post_matrix.translate(m_rotation_center); 
}

const C3DBounds& C3DRotBendTransformation::get_size() const
{
	return m_size;
}

P3DTransformation C3DRotBendTransformation::do_upscale(const C3DBounds& size) const
{
	auto result = new C3DRotBendTransformation(size, m_relative_origin, get_interpolator_factory()); 
	result->set_parameters(m_params); 
	return P3DTransformation(result);
}


C3DFMatrix C3DRotBendTransformation::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
	assert(0 && "not implemented"); 
}

C3DFMatrix C3DRotBendTransformation::derivative_at(int /*x*/, int /*y*/, int /*z*/) const
{
	assert(0 && "not implemented"); 
}

void C3DRotBendTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_pre_matrix.identity(); 
	m_post_matrix.identity(); 
	fill(m_params.begin(), m_params.end(), 0.0); 
}

float C3DRotBendTransformation::get_max_transform() const
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


C3DFVector C3DRotBendTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DRotBendTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DRotBendTransformation doesn't implement a jacobian."); 
}

void C3DRotBendTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
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
				
				// scaling 
				r[1] += 0; 
				r[2] += 0;
				


				// shear 
				r[3] += 0; 
				r[4] += 0; 
				r[5] += 0; 

				
			}
		}
	}
	std::copy(r.begin(), r.end(), params.begin());
}



C3DRotBendTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DRotBendTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DRotBendTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						      const C3DBounds& end, const C3DBounds& size, 
						      const C3DRotBendTransformation& trans):
	C3DTransformation::iterator_impl(pos, begin, end, size),
	m_trans(trans), 
	m_value(trans.apply(C3DFVector(pos)))
{
	m_dx = m_trans.apply(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - m_value;
}

C3DTransformation::iterator_impl * C3DRotBendTransformation::iterator_impl::clone() const
{
	return new iterator_impl(*this); 
}

const C3DFVector&  C3DRotBendTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C3DRotBendTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C3DRotBendTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}

void C3DRotBendTransformation::iterator_impl::do_z_increment()
{
	m_value = m_trans.apply(C3DFVector(get_pos())); 
	m_dx = m_trans.apply(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - m_value;
}


C3DTransformation::const_iterator C3DRotBendTransformation::begin() const
{
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotBendTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotBendTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRotBendTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
	return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(), *this)); 
}



float C3DRotBendTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	DEBUG_ASSERT_RELEASE_THROW(false, "C3DRotBendTransformation doesn't implement pertuberate."); 
}

C3DRotBendTransformCreator::C3DRotBendTransformCreator(const C3DFVector& origin, 
						       const C3DInterpolatorFactory& ipf):
C3DTransformCreator(ipf), 
	m_origin(origin)
{
}

P3DTransformation C3DRotBendTransformCreator::do_create(const C3DBounds& size, 
							const C3DInterpolatorFactory& ipf) const
{
	return P3DTransformation(new C3DRotBendTransformation(size, m_origin, ipf));
}




C3DRotBendTransformCreatorPlugin::C3DRotBendTransformCreatorPlugin():
        C3DTransformCreatorPlugin("rotbend")
{
	add_parameter("origin", new C3DFVectorParameter(m_origin, true, "center of the transformation"));
}


C3DTransformCreator *C3DRotBendTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
	return new C3DRotBendTransformCreator(m_origin, ipf);
}

const std::string C3DRotBendTransformCreatorPlugin::do_get_descr() const
{
	return "Restricted transformation (4 degrees of freedom). "
		"The transformation is restricted to the rotation around the x and y axis "
		"and a bending along the x axis, independedn in each direction, with the bending "
		"increasing with the squared distance from the rotation axis."
		;
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DRotBendTransformCreatorPlugin();
}



}
