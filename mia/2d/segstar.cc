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

#include <sstream>
#include <stdexcept>
#include <cassert>
#include <mia/core/msgstream.hh>
#include <mia/core/tools.hh>

#include <mia/2d/segstar.hh>
#include <libxml++/libxml++.h>


NS_MIA_BEGIN
using namespace std;

CSegStar::CSegStar():
	m_radius(0.0)
{
}

CSegStar::CSegStar(const CSegPoint2D& center, float r, const CSegPoint2D& d1, 
		   const CSegPoint2D& d2, const CSegPoint2D& d3):
	m_center(center),
	m_radius(r)
{
	m_directions[0] = d1;
	m_directions[1] = d2;
	m_directions[2] = d3;
}


CSegStar::CSegStar(const xmlpp::Node& n)
{
	TRACE("CSegStar::CSegStar");

	if (n.get_name() != "star")
		throw create_exception<runtime_error>("CSegStar: expect node of type 'star', but got '", n.get_name(), "'");

	const xmlpp::Element& node = dynamic_cast<const xmlpp::Element&>(n);


	m_center = CSegPoint2D(node);
	xmlpp::Attribute *rx = node.get_attribute ("r");
	if (!rx)
		throw runtime_error("CSegStar: attribute r not found");

	if (!from_string(rx->get_value(), m_radius)) 
		throw create_exception<runtime_error>("CSegStar: radius attribute '", rx->get_value(), "' is not a floating point value"); 

	cvdebug() << "Got star center (" << m_center.x << ", " << m_center.y << " @ " << m_radius << ")\n";

	xmlpp::Node::NodeList points = node.get_children("point");
	size_t npoints  = points.size();

	if (npoints != 3)
		throw invalid_argument("Bogus: Star should have 3 direction points");

	size_t k = 0;
	for (xmlpp::Node::NodeList::const_iterator i = points.begin();
	     i != points.end(); ++i, ++k) {
		xmlpp::Element& node = dynamic_cast<xmlpp::Element&>(**i);
		m_directions[k] = CSegPoint2D(node);
	}
}

void CSegStar::shift(const C2DFVector& delta)
{
	m_center -= delta;
}

void CSegStar::transform(const C2DTransformation& t)
{
	cvdebug() << "CSegStar::transform: " << m_center << "@" << m_radius << "\n"; 
	for (size_t i = 0; i < 3; ++i) {
		cvdebug() << "CSegStar::transform:" << i << ":" << m_center + m_radius * m_directions[i] << "\n"; 
		m_directions[i] = t(m_center + m_radius * m_directions[i]);
		cvdebug() << "CSegStar::transform:" << i << ":" << m_directions[i] << "\n"; 
	}
	recenter_rays(); 
}

inline double  __calc_bc(double a, double b, double c)
{
	return a * a *( b * b + c * c - a *a ); 
}
		

// re-evaluate the center after a transformation has been applied 
// since the transformation may be non-rigid, base this evaluation on the 
// points located at the circumfence 
void CSegStar::reeval_center() 
{
	double a = (m_directions[1] - m_directions[2]).norm(); 
	double b = (m_directions[2] - m_directions[0]).norm(); 
	double c = (m_directions[0] - m_directions[1]).norm(); 
	
	vector<double> x(3); 
	x[0] = __calc_bc(a, b, c); 
	x[1] = __calc_bc(b, c, a); 
	x[2] = __calc_bc(c, a, b); 
	
        double sum = x[0] + x[1] + x[2]; 
        for (size_t i = 0; i < 3; ++i) {
		x[i] /= sum; 
	}
	C2DFVector result(0,0); 
	
	for (size_t i = 0; i < 3; ++i) 
		result += x[i] * m_directions[i]; 
        m_center = result; 
}



void CSegStar::recenter_rays()
{
	reeval_center(); 
	m_directions[0] -= m_center; 
	float n = m_directions[0].norm(); 
	m_radius = n; 
	m_directions[0] /= n; 

        float c = -0.5; 
        float s = sqrt(0.75); 

	m_directions[1] = C2DFVector(  c * m_directions[0].x +  s * m_directions[0].y,  
				       c * m_directions[0].y -  s * m_directions[0].x); 
	if (m_directions[1].x < 0) 
		m_directions[1] *= -1; 
	
	m_directions[2] = C2DFVector(  c * m_directions[0].x -  s * m_directions[0].y,  
				       c * m_directions[0].y +  s * m_directions[0].x); 
	
	if (m_directions[2].x > 0) 
		m_directions[2] *= -1; 
	

}

void CSegStar::inv_transform(const C2DTransformation& t)
{
	for (size_t i = 0; i < 3; ++i) {
		m_directions[i] = m_center + m_radius * m_directions[i];
		m_directions[i].inv_transform(t);
	}
	recenter_rays();
}

void CSegStar::write(xmlpp::Node& node) const
{
	xmlpp::Element* nodeChild = node.add_child("star");

	nodeChild->set_attribute("y", to_string<float>(m_center.y));
	nodeChild->set_attribute("x", to_string<float>(m_center.x));
	nodeChild->set_attribute("r", to_string<float>(m_radius));

	for (size_t i = 0; i < 3; ++i)
		m_directions[i].write(*nodeChild);
}

NS_MIA_END
