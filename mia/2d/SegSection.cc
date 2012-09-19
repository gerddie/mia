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

#define VSTREAM_DOMAIN "SegSection"
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/2d/SegSection.hh>
#include <libxml++/libxml++.h>


NS_MIA_BEGIN
using namespace std;

CSegSection::CSegSection():
	m_is_open(false)
{
}

CSegSection::CSegSection(const string& id, const Points& points, bool is_open):
	m_id(id),
	m_points(points), 
	m_is_open(is_open)
{
}

CSegSection::CSegSection(xmlpp::Node& node, int version)
{
	TRACE("CSegSection::CSegSection");

	xmlpp::Element& elm = dynamic_cast<xmlpp::Element&>(node);
	xmlpp::Attribute *id = elm.get_attribute ("color");

	if (!id)
		throw invalid_argument("CSegSection::CSegSection: node without id");
	m_id = id->get_value();

	xmlpp::Node::NodeList points = node.get_children("point");

	for (auto i = points.begin(); i != points.end(); ++i)
		m_points.push_back(CSegPoint2D(**i));

	if (version > 1) {
		read_attribute_from_node(elm, "open", m_is_open);  
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
	for(auto i = m_points.begin(); i != m_points.end(); ++i) 
		i->transform(t);
}


void CSegSection::inv_transform(const C2DTransformation& t)
{
	for(auto i = m_points.begin(); i != m_points.end(); ++i) 
		i->inv_transform(t);
}


void CSegSection::write(xmlpp::Node& node, int version) const
{
	xmlpp::Element* nodeChild = node.add_child("section");
	nodeChild->set_attribute("color", m_id);

	if (version > 1) {
		nodeChild->set_attribute("open", m_is_open ? "true" : "false");
	}

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

template <typename FDrawOperator> 
void draw_private(C2DUBImage& mask, const CSegSection::Points& points,  const FDrawOperator& op)
{
	for (size_t y=0; y < mask.get_size().y; y++) {
		vector<int> nodeX; 
		int j=points.size()-1;
		for (size_t i=0; i<points.size(); i++) {
			if ((points[i].y <= y && points[j].y > y) || 
			    (points[j].y <= y && points[i].y > y) ) {
				nodeX.push_back( (int) (points[i].x + 
						       ( y - points[i].y)/(points[j].y-points[i].y)
							*(points[j].x - points[i].x)) + 0.5); 
			}
			j=i; 
		}
		
		sort(nodeX.begin(), nodeX.end()); 
		
		//  Fill the pixels between node pairs.
		for (size_t i=0; i<nodeX.size(); i+=2) {
			if   (nodeX[i  ]>=(int)mask.get_size().x) 
				break;
			if   (nodeX[i+1]> 0 ) {
				if (nodeX[i  ] < 0 ) 
					nodeX[i  ]=0;
				if (nodeX[i+1] > (int)mask.get_size().x) 
					nodeX[i+1]=mask.get_size().x;
				for (int j=nodeX[i]; j<nodeX[i+1]; j++) 
					op(mask(j,y));
			}
		}
	}
	
}

void CSegSection::draw_xor(C2DUBImage& mask)const
{
	auto xor_draw = [](unsigned char& pixel) { pixel ^= 1 ;}; 
	draw_private(mask, m_points, xor_draw); 
}

void CSegSection::draw(C2DUBImage& mask, unsigned char color)const
{
	auto color_draw = [color](unsigned char& pixel) { pixel = color;};
	draw_private(mask, m_points, color_draw); 
}

bool CSegSection::is_open() const
{
	return m_is_open; 
}

NS_MIA_END
