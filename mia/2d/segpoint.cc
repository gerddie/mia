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

#include <sstream>
#include <stdexcept>
#include <cassert>
#include <mia/2d/segpoint.hh>
#include <mia/core/tools.hh>
#include <mia/core/xmlinterface.hh>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


NS_MIA_BEGIN

using namespace std;

void read_attribute_from_node(const CXMLElement& elm, const std::string& key, bool& out_value, bool required)
{
       auto attr = elm.get_attribute(key);

       if (attr.empty()) {
              if (required)
                     throw create_exception<runtime_error>( elm.get_name(), ":required attribute '", key, "' not found");
              else
                     return;
       }

       if (attr == string("false"))
              out_value = false;
       else if (attr == string("true"))
              out_value = true;
       else
              throw create_exception<runtime_error>( elm.get_name(), ":attribute '", key, "' has bogus value '",
                                                     attr, "'");
}

CSegPoint2D::CSegPoint2D()
{
}

CSegPoint2D::CSegPoint2D(const C2DFVector& org): C2DFVector(org)
{
}


CSegPoint2D& CSegPoint2D::operator = (const C2DFVector& org)
{
       C2DFVector::operator =(org);
       return *this;
}


CSegPoint2D::CSegPoint2D(float x, float y):
       C2DFVector(x, y)
{
}

CSegPoint2D::CSegPoint2D(const CXMLElement& elm)
{
       auto ax = elm.get_attribute ("x");
       auto ay = elm.get_attribute ("y");

       if (ax.empty() || ay.empty())
              throw runtime_error("SegSection:Point attribute x or y not found");

       if (!from_string(ax, x))
              throw create_exception<runtime_error>( "CSegPoint2D: x attribute '",
                                                     ax, "' is not a floating point value");

       if (!from_string(ay, y))
              throw create_exception<runtime_error>( "CSegPoint2D: y attribute '",
                                                     ay, "' is not a floating point value");
}

void CSegPoint2D::write(CXMLElement& node) const
{
       auto point = node.add_child("point");
       point->set_attribute("y", to_string<float>(y));
       point->set_attribute("x", to_string<float>(x));
}

void CSegPoint2D::transform(const C2DTransformation& t)
{
       const C2DFVector r =  t(*this);
       x = r.x;
       y = r.y;
}

void CSegPoint2D::inv_transform(const C2DTransformation& t)
{
       C2DFVector r(x, y);
       cvdebug() << r << "\n";
       int niter = 0;
       C2DFVector delta = t(r) - *this;

       while (delta.norm2() > 0.000001 && niter++ < 100) {
              r -= 0.1 * delta;
              delta = t(r) - *this;
              cvdebug() << r << delta << "\n";
       }

       x = r.x;
       y = r.y;
}


NS_MIA_END
