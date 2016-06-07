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


#include <mia/3d/shape.hh>

NS_BEGIN(basic_3dshape_creator)
NS_MIA_USE;
using namespace std;



class C6n3DShape: public C3DShape {
public:
	C6n3DShape();
};

class C6n3DShapeFactory: public C3DShapePlugin {
public:
	C6n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
};


class C18n3DShape: public C6n3DShape {
public:
	C18n3DShape();
};

class C18n3DShapeFactory: public C3DShapePlugin {
public:
	C18n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
};

class C26n3DShape: public C18n3DShape {
public:
	C26n3DShape();
};


class C26n3DShapeFactory: public C3DShapePlugin {
public:
	C26n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
};




NS_END
