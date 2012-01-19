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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>
#include <mia/2d/shape.hh>

NS_BEGIN(basic_2dshape_creator)

class C1n2DShape: public mia::C2DShape {
public:
	C1n2DShape();
};

class C1n2DShapeFactory: public mia::C2DShapePlugin {
public:
	C1n2DShapeFactory();
private:
	virtual const std::string do_get_descr() const;
	virtual mia::C2DShape *do_create()const;
};

class C4n2DShape: public C1n2DShape {
public:
	C4n2DShape();
};

class C4n2DShapeFactory: public mia::C2DShapePlugin {
public:
	C4n2DShapeFactory();
private:
	virtual const std::string do_get_descr() const;
	virtual mia::C2DShape *do_create()const;
};

class C8n2DShape: public C4n2DShape {
public:
	C8n2DShape();
};

class C8n2DShapeFactory: public mia::C2DShapePlugin {
public:
	C8n2DShapeFactory();
private:
	virtual const std::string do_get_descr() const;
	virtual mia::C2DShape *do_create()const;
};

NS_END
// basic_2dshape_creator
