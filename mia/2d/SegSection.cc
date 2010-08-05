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


#define VSTREAM_DOMAIN "SegSection"
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/2d/SegSection.hh>
#include <libxml++/libxml++.h>


NS_MIA_BEGIN
using namespace std;

CSegSection::CSegSection()
{
}

CSegSection::CSegSection(const string& id, const Points& points):
	m_id(id),
	m_points(points)
{
}

CSegSection::CSegSection(xmlpp::Node& node)
{
	TRACE("CSegSection::CSegSection");

	xmlpp::Element& elm = dynamic_cast<xmlpp::Element&>(node);
	xmlpp::Attribute *id = elm.get_attribute ("color");

	if (!id)
		throw invalid_argument("CSegSection::CSegSection: node without id");
	m_id = id->get_value();

	xmlpp::Node::NodeList points = node.get_children("point");

	for (xmlpp::Node::NodeList::const_iterator i = points.begin();
	     i != points.end(); ++i) {
		m_points.push_back(CSegPoint2D(**i));
	}
}

const string& CSegSection::get_id() const
{
	return m_id;
}

const CSegSection::Points& CSegSection::get_points()const
{
	return m_points;
}

void CSegSection::shift(const C2DFVector& delta)
{
	Points::iterator ip = m_points.begin();
	Points::iterator ep = m_points.end();

	while (ip != ep) {
		*ip -= delta;
		++ip;
	}
}

void CSegSection::transform(const C2DTransformation& t)
{
	Points::iterator ip = m_points.begin();
	Points::iterator ep = m_points.end();

	while (ip != ep) {
		ip->transform(t);
		++ip;
	}
}


void CSegSection::write(xmlpp::Node& node) const
{
	xmlpp::Element* nodeChild = node.add_child("section");
	nodeChild->set_attribute("color", m_id);

	Points::const_iterator ip = m_points.begin();
	Points::const_iterator ep = m_points.end();

	while (ip != ep) {
		ip->write(*nodeChild);
		++ip;
	}
}

const C2DBoundingBox CSegSection::get_boundingbox() const
{

	C2DBoundingBox result;

	Points::const_iterator ip = m_points.begin();
	Points::const_iterator ep = m_points.end();

	while (ip != ep) {
		result.add(*ip++);
	}
	return result;
}

void CSegSection::append_to(C2DPolygon& polygon)const
{
	typedef std::vector<CSegPoint2D>::const_iterator point_iterator;
	for(point_iterator i = m_points.begin(); i != m_points.end(); ++i)
		polygon.append(*i);

}

float CSegSection::get_hausdorff_distance(const CSegSection& other) const
{

	C2DPolygon p1;
	append_to(p1);

	C2DPolygon p2;
	other.append_to(p2);

	return p1.get_hausdorff_distance(p2);
}


NS_MIA_END
#if 0
void CSegSection::transform(const MSplineSignalList& defo, const std::vector<REAL>& scale)
{
	// copy the points and scale them
	KdataSplinesList p(m_points->size());

	for (size_t i = 0; i < m_points->size(); ++i) {
		PKdataSplines s = (*m_points)[i]->copy();
		for (KdataSplines::iterator c = s->begin(); c != s->end(); ++c)
			*c *= scale[i];
		p.insertAt(i, s);
	}

	KdataSplinesList pres = defo.evalat(p, NULL);

	for (size_t i = 0; i < m_points->size(); ++i) {
		KdataSplines::const_iterator i_res = pres[i]->begin();
		KdataSplines::const_iterator e_res = pres[i]->end();
		KdataSplines::iterator i_pnt = (*m_points)[i]->begin();

		while (i_res != e_res) {
			*i_pnt++ -= *i_res++;

		}
	}
}
#endif
