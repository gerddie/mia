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
#include <mia/core/msgstream.hh>
#include <mia/2d/transform/rigid.hh>


NS_MIA_BEGIN
using namespace std;

C2DRigidTransformation::C2DRigidTransformation(const C2DBounds& size, 
					       const C2DFVector& relative_rot_center, 
					       const C2DInterpolatorFactory& ipf):
	C2DTransformation(ipf),
	m_t(6),
	m_size(size),
	m_translation(0.0, 0.0),
	m_rotation(0.0),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	initialize();
}

C2DRigidTransformation::C2DRigidTransformation(const C2DBounds& size,const C2DFVector& translation,
					       float rotation, 
					       const C2DFVector& relative_rot_center,
					       const C2DInterpolatorFactory& ipf):
	C2DTransformation(ipf),
	m_t(6),
	m_size(size),
	m_translation(translation),
	m_rotation(rotation),
	m_relative_rot_center(relative_rot_center), 
	m_matrix_valid(false)
{
	initialize(); 
}

C2DRigidTransformation::C2DRigidTransformation(const C2DRigidTransformation& other):
	C2DTransformation(other),
	m_t(other.m_t),
	m_size(other.m_size),
	m_translation(other.m_translation),
	m_rotation(other.m_rotation),
	m_relative_rot_center(other.m_relative_rot_center), 
	m_rot_center(other.m_rot_center), 
	m_matrix_valid(other.m_matrix_valid)
{
}

void C2DRigidTransformation::initialize()
{
	m_rot_center = C2DFVector(m_size -C2DBounds::_1) * m_relative_rot_center; 
	cvdebug() << "m_rot_center= " << m_rot_center << "\n"; 

}

C2DFVector C2DRigidTransformation::apply(const C2DFVector& x) const
{
	return transform(x);
}



C2DFVector C2DRigidTransformation::transform(const C2DFVector& x)const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C2DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2],
		m_t[3] * x.x + m_t[4] * x.y + m_t[5]
			  );
}


C2DTransformation *C2DRigidTransformation::do_clone()const
{
	return new C2DRigidTransformation(*this);
}

C2DTransformation *C2DRigidTransformation::invert()const
{
	C2DRigidTransformation *result = new C2DRigidTransformation(*this); 
	result->m_rotation = -m_rotation;  
	const double sina = sin(m_rotation); 
	const double cosa = cos(m_rotation); 
	result->m_translation.x = - cosa * m_translation.x - sina * m_translation.y; 
	result->m_translation.y =   sina * m_translation.x - cosa * m_translation.y; 
	result->m_matrix_valid = false; 
	return result;
}


size_t C2DRigidTransformation::degrees_of_freedom() const
{
	return 3;
}

void C2DRigidTransformation::update(float /*step*/, const C2DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C2DRigidTransformation::translate(float x, float y)
{
	m_matrix_valid = false;
	m_translation.x +=  x;
	m_translation.y +=  y;
}

void C2DRigidTransformation::rotate(float angle)
{
	m_matrix_valid = false;
	m_rotation += angle;
	const double sina = sin(angle);
	const double cosa = cos(angle);

	const double tx      = cosa * m_translation.x - sina * m_translation.y;
	m_translation.y = sina * m_translation.x + cosa * m_translation.y;
	m_translation.x = tx;
}

CDoubleVector C2DRigidTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	result[0] = m_translation.x;
	result[1] = m_translation.y;
	result[2] = m_rotation;
	return result;
}

void C2DRigidTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());

	m_translation.x = params[0];
	m_translation.y = params[1];
	m_rotation = params[2];
	cvdebug() << "Rigid transform = (" << m_translation << ", " << m_rotation << ")\n"; 

	m_matrix_valid = false;
}

const C2DBounds& C2DRigidTransformation::get_size() const
{
	return m_size;
}

P2DTransformation C2DRigidTransformation::do_upscale(const C2DBounds& size) const
{
	C2DFVector new_trans(float(size.x) / (float)get_size().x * m_translation.x,
			     float(size.y) / (float)get_size().y * m_translation.y);
	return P2DTransformation(new C2DRigidTransformation(size, new_trans, m_rotation, 
							    m_relative_rot_center, get_interpolator_factory()));
}

C2DFMatrix C2DRigidTransformation::derivative_at(const C2DFVector& MIA_PARAM_UNUSED(x)) const
{
	if (!m_matrix_valid)
		evaluate_matrix(); 
	
	return C2DFMatrix(C2DFVector(m_t[0], m_t[1]),
			  C2DFVector(m_t[3], m_t[4]));
}

C2DFMatrix C2DRigidTransformation::derivative_at(int MIA_PARAM_UNUSED(x), int MIA_PARAM_UNUSED(y)) const
{
	if (!m_matrix_valid)
		evaluate_matrix(); 
	
	return C2DFMatrix(C2DFVector(m_t[0], m_t[1]),
			  C2DFVector(m_t[3], m_t[4]));
}

void C2DRigidTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_translation.x = m_translation.y = m_rotation = 0.0;
}

void C2DRigidTransformation::evaluate_matrix() const
{
	const double cosa = cos(m_rotation);
	const double sina = sin(m_rotation);

	m_t[0] = cosa;
	m_t[1] = -sina;
	m_t[3] = sina;
	m_t[4] = cosa;

	C2DFVector d(m_t[0] * m_rot_center.x + m_t[1] * m_rot_center.y,
		     m_t[3] * m_rot_center.x + m_t[4] * m_rot_center.y); 
		

	m_t[2] = m_translation.x - d.x + m_rot_center.x;
	m_t[5] = m_translation.y - d.y + m_rot_center.y;

	m_matrix_valid = true;
}

float C2DRigidTransformation::get_max_transform() const
{
	// check the corners
	float m =      (C2DFVector(get_size() - C2DBounds::_1) -  apply(C2DFVector(get_size()  - C2DBounds::_1))).norm2();
	float test0Y = (C2DFVector(0, get_size().y - 1) - apply(C2DFVector(0, get_size().y - 1))).norm2();
	float testX0 = (C2DFVector(get_size().x - 1, 0) - apply(C2DFVector(get_size().x - 1, 0))).norm2();
	float test00 = apply(C2DFVector(0, 0)).norm2();

	if (m < test0Y)
		m = test0Y;

	if (m < testX0)
		m = testX0;

	if (m < test00)
		m = test00;
	return sqrt(m);
}


C2DFVector C2DRigidTransformation::operator () (const C2DFVector& x) const
{
	return apply(x); 
}

float C2DRigidTransformation::get_jacobian(const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C2DRigidTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	//

	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();

	double fy = - m_rot_center.y; 
	for (size_t y = 0; y < m_size.y; ++y, fy += 1.0) {
		double fx = - m_rot_center.x; 
		for (size_t x = 0; x < m_size.x; ++x, fx += 1.0, ++g) {
			r[0] += g->x;
			r[1] += g->y;
			r[2] += - fy * g->x + fx * g->y; 
		}
	}
	copy(r.begin(), r.end(), params.begin()); 
}



C2DRigidTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						      const C2DRigidTransformation& trans):
	C2DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.transform(C2DFVector(pos)))
{
	m_dx = m_trans.transform(C2DFVector(pos.x + 1.0, pos.y)) - m_value;
}

C2DTransformation::iterator_impl * C2DRigidTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), m_trans); 
}

const C2DFVector&  C2DRigidTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C2DRigidTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C2DRigidTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.transform(C2DFVector(get_pos())); 
	m_dx = m_trans.transform(C2DFVector(get_pos().x + 1.0, get_pos().y)) - m_value;
}


C2DTransformation::const_iterator C2DRigidTransformation::begin() const
{
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), get_size(), *this)); 
}

C2DTransformation::const_iterator C2DRigidTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

float C2DRigidTransformation::pertuberate(C2DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

C2DRigidTransformCreator::C2DRigidTransformCreator(const C2DFVector& relative_rot_center, const C2DInterpolatorFactory& ipf):
	C2DTransformCreator(ipf), 
	m_relative_rot_center(relative_rot_center)
{
}

P2DTransformation C2DRigidTransformCreator::do_create(const C2DBounds& size, const C2DInterpolatorFactory& ipf) const
{
	return P2DTransformation(new C2DRigidTransformation(size, m_relative_rot_center, ipf));
}

C2DRigidTransformCreatorPlugin::C2DRigidTransformCreatorPlugin():
	C2DTransformCreatorPlugin("rigid")
{
	add_parameter("rot-center", make_param(m_relative_rot_center, false, 
					       "Relative rotation center, i.e.  <0.5,0.5> corresponds "
					       "to the center of the support rectangle"));
}

C2DTransformCreator *C2DRigidTransformCreatorPlugin::do_create(const C2DInterpolatorFactory& ipf) const
{
	return new C2DRigidTransformCreator(m_relative_rot_center, ipf);
}

const std::string C2DRigidTransformCreatorPlugin::do_get_descr() const
{
	return "Rigid transformations (i.e. rotation and translation, three degrees of freedom).";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRigidTransformCreatorPlugin();
}


NS_MIA_END
