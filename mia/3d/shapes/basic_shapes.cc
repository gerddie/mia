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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>
#include <mia/3d/shapes/sphere.hh>
#include <mia/3d/shapes/basic_shapes.hh>

NS_BEGIN(basic_3dshape_creator)
NS_MIA_USE;
using namespace std;

C6n3DShape::C6n3DShape()
{
	insert(C3DShape::Flat::value_type( 0, 0, 0));
	insert(C3DShape::Flat::value_type(-1, 0, 0));
	insert(C3DShape::Flat::value_type( 1, 0, 0));
	insert(C3DShape::Flat::value_type( 0,-1, 0));
	insert(C3DShape::Flat::value_type( 0, 1, 0));
	insert(C3DShape::Flat::value_type( 0, 0, 1));
	insert(C3DShape::Flat::value_type( 0, 0,-1));
}

C6n3DShapeFactory::C6n3DShapeFactory():
	C3DShapePlugin("6n")
{
}

C3DShape *C6n3DShapeFactory::do_create()const
{
	return new C6n3DShape();
}


const string C6n3DShapeFactory::do_get_descr()const
{
	return string("6n neighborhood 3D shape creator");
}


C18n3DShape::C18n3DShape()
{
	insert(C3DShape::Flat::value_type(-1,-1, 0));
	insert(C3DShape::Flat::value_type(-1, 1, 0));
	insert(C3DShape::Flat::value_type(-1, 0,-1));
	insert(C3DShape::Flat::value_type(-1, 0, 1));

	insert(C3DShape::Flat::value_type( 0, 1,-1));
	insert(C3DShape::Flat::value_type( 0, 1, 1));
	insert(C3DShape::Flat::value_type( 0,-1,-1));
	insert(C3DShape::Flat::value_type( 0,-1, 1));

	insert(C3DShape::Flat::value_type( 1,-1, 0));
	insert(C3DShape::Flat::value_type( 1, 1, 0));
	insert(C3DShape::Flat::value_type( 1, 0,-1));
	insert(C3DShape::Flat::value_type( 1, 0, 1));
}

C18n3DShapeFactory::C18n3DShapeFactory():
	C3DShapePlugin("18n")
{
}

C3DShape *C18n3DShapeFactory::do_create()const
{
	return new C18n3DShape();
}


const string C18n3DShapeFactory::do_get_descr()const
{
	return string("18n neighborhood 3D shape creator");
}


C26n3DShape::C26n3DShape()
{
	insert(C3DShape::Flat::value_type(-1,-1,-1));
	insert(C3DShape::Flat::value_type(-1,-1, 1));
	insert(C3DShape::Flat::value_type(-1, 1,-1));
	insert(C3DShape::Flat::value_type(-1, 1, 1));
	insert(C3DShape::Flat::value_type( 1,-1,-1));
	insert(C3DShape::Flat::value_type( 1,-1, 1));
	insert(C3DShape::Flat::value_type( 1, 1,-1));
	insert(C3DShape::Flat::value_type( 1, 1, 1));
}

C26n3DShapeFactory::C26n3DShapeFactory():
	C3DShapePlugin("26n")
{
}

C3DShape *C26n3DShapeFactory::do_create()const
{
	return new C26n3DShape();
}


const string C26n3DShapeFactory::do_get_descr()const
{
	return string("26n neighborhood 3D shape creator");
}

extern "C" {
	EXPORT CPluginBase *get_plugin_interface()
	{
		CPluginBase *n6 = new C6n3DShapeFactory();
		n6->append_interface(new C18n3DShapeFactory());
		n6->append_interface(new C26n3DShapeFactory());
		n6->append_interface(new CSphere3DShapeFactory());
		return n6;
	}
}
NS_END
// end namespace _18n_3dshape_creator
