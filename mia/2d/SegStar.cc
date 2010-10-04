/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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


#include <sstream>
#include <stdexcept>
#include <cassert>
#include <mia/core/msgstream.hh>
#include <mia/core/tools.hh>

#include <mia/2d/SegStar.hh>
#include <libxml++/libxml++.h>


NS_MIA_BEGIN
using namespace std;

CSegStar::CSegStar()
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
	assert(n.get_name() == "star");
	const xmlpp::Element& node = dynamic_cast<const xmlpp::Element&>(n);


	m_center = CSegPoint2D(node);
	xmlpp::Attribute *rx = node.get_attribute ("r");
	if (!rx)
		throw invalid_argument("CSegStar: attribute r not found");

	m_radius = from_string<float>(rx->get_value());

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
	// in theory one should evaluate the center as mean of the 
	// ray points (at least for non-rigid transformations)

	C2DFVector old_center = m_center; 
	m_center.transform(t); 
	// transform the rays 
	for (size_t i = 0; i < 3; ++i) {
		C2DFVector help = t(old_center + m_radius * m_directions[i]) - m_center;
		float n =  help.norm(); 
		m_directions[i].x = help.x / n; 
		m_directions[i].y = help.y / n; 
	}
}

void CSegStar::inv_transform(const C2DTransformation& t)
{
	C2DFVector old_center = m_center; 
	m_center.inv_transform(t); 
	// transform the rays 
	for (size_t i = 0; i < 3; ++i) {
		C2DFVector help = old_center + m_radius * m_directions[i]; 
		CSegPoint2D h(help.x, help.y); 
		h.inv_transform(t); 
		help = h - m_center; 
		float n =  help.norm(); 
		m_directions[i].x = help.x / n; 
		m_directions[i].y = help.y / n; 
	}
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
