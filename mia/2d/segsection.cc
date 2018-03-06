/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#define VSTREAM_DOMAIN "SegSection"
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/2d/segsection.hh>
#include <mia/core/xmlinterface.hh>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

CSegSection::CSegSection(const CXMLElement& elm, int version):
       m_is_open(false)
{
       TRACE("CSegSection::CSegSection");
       m_id = elm.get_attribute ("color");

       if (m_id.empty())
              throw invalid_argument("CSegSection::CSegSection: node without id");

       auto points = elm.get_children("point");

       for (auto i : points)
              m_points.push_back(CSegPoint2D(*i));

       if (version > 1) {
              read_attribute_from_node(elm, "open", m_is_open, false);
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
       for (auto& i : m_points)
              i -= delta;
}

void CSegSection::transform(const C2DTransformation& t)
{
       for (auto& i : m_points)
              i.transform(t);
}


void CSegSection::inv_transform(const C2DTransformation& t)
{
       for (auto& i : m_points)
              i.inv_transform(t);
}


void CSegSection::write(CXMLElement& node, int version) const
{
       auto nodeChild = node.add_child("section");
       nodeChild->set_attribute("color", m_id);

       if (version > 1) {
              nodeChild->set_attribute("open", m_is_open ? "true" : "false");
       }

       for (auto ip : m_points) {
              ip.write(*nodeChild);
       }
}

const C2DBoundingBox CSegSection::get_boundingbox() const
{
       C2DBoundingBox result;

       for (auto ip : m_points)
              result.add(ip);

       return result;
}

void CSegSection::append_to(C2DPolygon& polygon)const
{
       for (auto i : m_points)
              polygon.append(i);
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
       for (size_t y = 0; y < mask.get_size().y; y++) {
              vector<int> nodeX;
              int j = points.size() - 1;

              for (size_t i = 0; i < points.size(); i++) {
                     if ((points[i].y <= y && points[j].y > y) ||
                         (points[j].y <= y && points[i].y > y) ) {
                            nodeX.push_back( (int) (points[i].x +
                                                    ( y - points[i].y) / (points[j].y - points[i].y)
                                                    * (points[j].x - points[i].x)) + 0.5);
                     }

                     j = i;
              }

              sort(nodeX.begin(), nodeX.end());

              //  Fill the pixels between node pairs.
              for (size_t i = 0; i < nodeX.size(); i += 2) {
                     if   (nodeX[i  ] >= (int)mask.get_size().x)
                            break;

                     if   (nodeX[i + 1] > 0 ) {
                            if (nodeX[i  ] < 0 )
                                   nodeX[i  ] = 0;

                            if (nodeX[i + 1] > (int)mask.get_size().x)
                                   nodeX[i + 1] = mask.get_size().x;

                            for (int j = nodeX[i]; j < nodeX[i + 1]; j++)
                                   op(mask(j, y));
                     }
              }
       }
}

void CSegSection::draw_xor(C2DUBImage& mask)const
{
       if (m_is_open)
              throw invalid_argument("CSegSection: Section not closed, hence no filled polygon can be drawn");

       auto xor_draw = [](unsigned char& pixel) {
              pixel ^= 1 ;
       };
       draw_private(mask, m_points, xor_draw);
}

void CSegSection::draw(C2DUBImage& mask, unsigned char color)const
{
       if (m_is_open)
              throw invalid_argument("CSegSection: Section not closed, hence no filled polygon can be drawn");

       auto color_draw = [color](unsigned char& pixel) {
              pixel = color;
       };
       draw_private(mask, m_points, color_draw);
}

bool CSegSection::is_open() const
{
       return m_is_open;
}

NS_MIA_END
