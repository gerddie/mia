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

#ifndef _mia_3d_rect_creator_hh
#define _mia_3d_rect_creator_hh

#include <mia/2d/shape.hh>

NS_MIA_BEGIN

class CRectangle2DShape: public C2DShape {
public:
	CRectangle2DShape(int width, int height, bool filled);
};


class CSquare2DShapePlugin: public C2DShapePlugin {
public:
	CSquare2DShapePlugin();
protected: 
	int get_width() const;
	bool get_filled() const; 
private:
	
	virtual const std::string do_get_descr() const;
	virtual C2DShape *do_create()const;

	int m_width; 
	bool m_filled; 
};

class CRectangle2DShapePlugin: public C2DShapePlugin {
public:
	CRectangle2DShapePlugin();
private:
	
	virtual const std::string do_get_descr() const;
	virtual C2DShape *do_create()const;
	int m_width; 
	int m_height; 
	bool m_filled; 
};


NS_MIA_END
#endif
