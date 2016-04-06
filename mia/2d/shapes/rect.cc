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

#include <mia/2d/shapes/rect.hh>

NS_MIA_BEGIN
using namespace std; 

CRectangle2DShape::CRectangle2DShape(int width, int height, bool filled) 
{
	if (filled)  {
		for (int y = -height; y <= height; ++y)
			for (int x = -width; x <= width; ++x)
				insert(C2DShape::Flat::value_type(x,y));
	}else {
		for (int y = -height; y <= height; ++y) {
			insert(C2DShape::Flat::value_type(-width,y));
			insert(C2DShape::Flat::value_type(width,y));
		}
		for (int x = -width; x <= width; ++x) {
			insert(C2DShape::Flat::value_type(x,height));
			insert(C2DShape::Flat::value_type(x,-height));
		}
	}
}

CSquare2DShapePlugin::CSquare2DShapePlugin():
	C2DShapePlugin("square"),
	m_width(2), 
	m_filled(true)
{
	add_parameter("width", make_lc_param(m_width, 1, false, "width of rectangle"));
	add_parameter("fill", new CBoolParameter(m_filled, false, "create a filled shape"));
}

C2DShape *CSquare2DShapePlugin::do_create()const
{
	return new CRectangle2DShape(m_width, m_width, m_filled);
}

const string CSquare2DShapePlugin::do_get_descr()const
{
	return "square shape mask creator";
}



CRectangle2DShapePlugin::CRectangle2DShapePlugin():
	C2DShapePlugin("rectangle"),
	m_width(2), 
	m_height(2), 
	m_filled(true)
{
	add_parameter("width", make_lc_param(m_width, 1, false, "width of rectangle"));
	add_parameter("height", make_lc_param(m_height, 1, false, "height of rectangle"));
	add_parameter("fill", new CBoolParameter(m_filled, false, "create a filled shape"));
}

C2DShape *CRectangle2DShapePlugin::do_create()const
{
	return new CRectangle2DShape(m_width, m_height, m_filled);
}


const string CRectangle2DShapePlugin::do_get_descr()const
{
	return "rectangle shape mask creator";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *plugin = new CRectangle2DShapePlugin();
	plugin->append_interface(new CSquare2DShapePlugin());
	return plugin; 
}

NS_MIA_END
