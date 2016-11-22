/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/2d/transform/rotation.hh>


NS_MIA_BEGIN
using namespace std;

C2DRotationTransformation::C2DRotationTransformation(const C2DBounds& size, 
					       const C2DFVector& relative_rot_center, 
					       const C2DInterpolatorFactory& ipf):
	C2DTransformation(ipf),
	m_size(size),
	m_rotation(0.0),
	m_relative_rot_center(relative_rot_center), 
	m_t(6),
	m_matrix_valid(false)
{
	initialize();
}

C2DRotationTransformation::C2DRotationTransformation(const C2DBounds& size, double rotation, 
					       const C2DFVector& relative_rot_center,
					       const C2DInterpolatorFactory& ipf):
	C2DTransformation(ipf),
	m_size(size),
	m_rotation(rotation),
	m_relative_rot_center(relative_rot_center), 
	m_t(6),
	m_matrix_valid(false)
{
	initialize(); 
}

C2DRotationTransformation::C2DRotationTransformation(const C2DRotationTransformation& other):
	C2DTransformation(other),
	m_size(other.m_size),
	m_rotation(other.m_rotation),
	m_relative_rot_center(other.m_relative_rot_center), 
	m_rot_center(other.m_rot_center), 
	m_t(other.m_t),
	m_matrix_valid(other.m_matrix_valid)
{
}

void C2DRotationTransformation::initialize()
{
	m_rot_center = C2DFVector(m_size -C2DBounds::_1) * m_relative_rot_center; 
}

C2DFVector C2DRotationTransformation::get_displacement_at(const C2DFVector& x) const
{
	return x - transform(x);
}



C2DFVector C2DRotationTransformation::transform(const C2DFVector& x)const
{
	if (!m_matrix_valid)
		evaluate_matrix();

	return C2DFVector(
		m_t[0] * x.x + m_t[1] * x.y + m_t[2],
		m_t[3] * x.x + m_t[4] * x.y + m_t[5]
			  );
}


C2DTransformation *C2DRotationTransformation::do_clone()const
{
	return new C2DRotationTransformation(*this);
}

C2DTransformation *C2DRotationTransformation::invert()const
{
	C2DRotationTransformation *result = new C2DRotationTransformation(*this); 
	result->m_rotation = -m_rotation;  
	result->m_matrix_valid = false; 
	return result;
}


size_t C2DRotationTransformation::degrees_of_freedom() const
{
	return 1;
}

void C2DRotationTransformation::update(float /*step*/, const C2DFVectorfield& /*a*/)
{
	assert(0 && "not implemented");
}

void C2DRotationTransformation::rotate(double angle)
{
	m_matrix_valid = false;
	m_rotation += angle;
}

CDoubleVector C2DRotationTransformation::get_parameters() const
{
	CDoubleVector result(degrees_of_freedom());
	result[0] = m_rotation;
	return result;
}

void C2DRotationTransformation::set_parameters(const CDoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());
	m_rotation = params[0];
	m_matrix_valid = false;
}


const C2DBounds& C2DRotationTransformation::get_size() const
{
	return m_size;
}

P2DTransformation C2DRotationTransformation::do_upscale(const C2DBounds& size) const
{
	return P2DTransformation(new C2DRotationTransformation(size, m_rotation, 
							    m_relative_rot_center, get_interpolator_factory()));
}

C2DFMatrix C2DRotationTransformation::derivative_at(const C2DFVector& MIA_PARAM_UNUSED(x)) const
{
	if (!m_matrix_valid)
		evaluate_matrix(); 
	
	return C2DFMatrix(C2DFVector(m_t[0], m_t[1]),
			  C2DFVector(m_t[3], m_t[4]));
}

C2DFMatrix C2DRotationTransformation::derivative_at(int MIA_PARAM_UNUSED(x), int MIA_PARAM_UNUSED(y)) const
{
	if (!m_matrix_valid)
		evaluate_matrix(); 
	
	return C2DFMatrix(C2DFVector(m_t[0], m_t[1]),
			  C2DFVector(m_t[3], m_t[4]));
}

void C2DRotationTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	m_rotation = 0.0;
}

void C2DRotationTransformation::evaluate_matrix() const
{
	const double cosa = cos(m_rotation);
	const double sina = sin(m_rotation);

	m_t[0] = cosa;
	m_t[1] = -sina;
	m_t[3] = sina;
	m_t[4] = cosa;

	C2DFVector d(m_t[0] * m_rot_center.x + m_t[1] * m_rot_center.y,
		     m_t[3] * m_rot_center.x + m_t[4] * m_rot_center.y); 
		
	m_t[2] = m_rot_center.x - d.x;
	m_t[5] = m_rot_center.y - d.y;

	m_matrix_valid = true;
}

float C2DRotationTransformation::get_max_transform() const
{
	// check the corners
	float m =      (C2DFVector(get_size() - C2DBounds::_1) -  transform(C2DFVector(get_size()  - C2DBounds::_1))).norm2();
	float test0Y = (C2DFVector(0, get_size().y - 1) - transform(C2DFVector(0, get_size().y - 1))).norm2();
	float testX0 = (C2DFVector(get_size().x - 1, 0) - transform(C2DFVector(get_size().x - 1, 0))).norm2();
	float test00 = transform(C2DFVector(0, 0)).norm2();

	if (m < test0Y)
		m = test0Y;

	if (m < testX0)
		m = testX0;

	if (m < test00)
		m = test00;
	return sqrt(m);
}


C2DFVector C2DRotationTransformation::operator () (const C2DFVector& x) const
{
	return transform(x); 
}

float C2DRotationTransformation::get_jacobian(const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C2DRotationTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(gradient.get_size() == m_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	double fy = - m_rot_center.y; 
	for (size_t y = 0; y < m_size.y; ++y, fy += 1.0) {
		double fx = - m_rot_center.x; 
		for (size_t x = 0; x < m_size.x; ++x, fx += 1.0, ++g) {
			r[0] += - fy * g->x + fx * g->y; 
		}
	}
	params[0] = r[0];
}



C2DRotationTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						      const C2DRotationTransformation& trans):
	C2DTransformation::iterator_impl(pos, size),
	m_trans(trans), 
	m_value(trans.transform(C2DFVector(pos)))
{
	m_dx = m_trans.transform(C2DFVector(pos.x + 1.0, pos.y)) - m_value;
}

C2DTransformation::iterator_impl * C2DRotationTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), m_trans); 
}

const C2DFVector&  C2DRotationTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C2DRotationTransformation::iterator_impl::do_x_increment()
{
	m_value += m_dx; 
}

void C2DRotationTransformation::iterator_impl::do_y_increment()
{
	m_value = m_trans.transform(C2DFVector(get_pos())); 
	m_dx = m_trans.transform(C2DFVector(get_pos().x + 1.0, get_pos().y)) - m_value;
}


C2DTransformation::const_iterator C2DRotationTransformation::begin() const
{
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), get_size(), *this)); 
}

C2DTransformation::const_iterator C2DRotationTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

float C2DRotationTransformation::pertuberate(C2DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

C2DRotationTransformCreator::C2DRotationTransformCreator(const C2DFVector& relative_rot_center, const C2DInterpolatorFactory& ipf):
	C2DTransformCreator(ipf), 
	m_relative_rot_center(relative_rot_center)
{
}

P2DTransformation C2DRotationTransformCreator::do_create(const C2DBounds& size, const C2DInterpolatorFactory& ipf) const
{
	return P2DTransformation(new C2DRotationTransformation(size, m_relative_rot_center, ipf));
}

C2DRotationTransformCreatorPlugin::C2DRotationTransformCreatorPlugin():
	C2DTransformCreatorPlugin("rotation")
{
	add_parameter("rot-center", make_param(m_relative_rot_center, false, 
					       "Relative rotation center, i.e.  <0.5,0.5> corresponds "
					       "to the center of the support rectangle"));
}

C2DTransformCreator *C2DRotationTransformCreatorPlugin::do_create(const C2DInterpolatorFactory& ipf) const
{
	return new C2DRotationTransformCreator(m_relative_rot_center, ipf);
}

const std::string C2DRotationTransformCreatorPlugin::do_get_descr() const
{
	return "Rotation transformations (i.e. rotation about a given center, one degree of freedom).";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRotationTransformCreatorPlugin();
}


NS_MIA_END
