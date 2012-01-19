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

#include <mia/2d/shapes/rect.hh>

NS_MIA_BEGIN

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
	add_parameter("width", new CIntParameter(m_width, 0, numeric_limits<int>::max(), false, "width of rectangle"));
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
	add_parameter("width", new CIntParameter(m_width, 0, numeric_limits<int>::max(), false, "width of rectangle"));
	add_parameter("height", new CIntParameter(m_height, 0, numeric_limits<int>::max(), false, "height of rectangle"));
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
