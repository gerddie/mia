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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>
#include <mia/3d/shape.hh>
#include "sphere.hh"


NS_BEGIN(basic_3dshape_creator)
NS_MIA_USE;
using namespace std;


class C6n3DShape: public C3DShape {
public:
	C6n3DShape();
};

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

class C6n3DShapeFactory: public C3DShapePlugin {
public:
	C6n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
	virtual bool do_test() const;
};

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

bool C6n3DShapeFactory::do_test()const
{
	C6n3DShape shape;
	C6n3DShape::Mask mask = shape.get_mask();

	if (mask.get_size() != C3DBounds(3,3,3)) {
		cvfail() << get_name() << ": wrong mask size\n";
		assert("size failture");
	}

	if (! ( mask(1,1,1) && mask(1,1,0) && mask(0,1,1) && mask(1,0,1) &&
		mask(1,2,1) && mask(2,1,1) && mask(1,1,2) ) ||
	    mask(0,0,0) || mask(1,0,0) || mask(2,0,0) ||
	    mask(0,1,0) ||                mask(2,1,0) ||
	    mask(0,2,0) || mask(1,2,0) || mask(2,2,0) ||
	    mask(0,0,1) ||                mask(2,0,1) ||
	    mask(0,2,1) ||                mask(2,2,1) ||
	    mask(0,0,2) || mask(1,0,2) || mask(2,0,2) ||
	    mask(0,1,2) ||                mask(2,1,2) ||
	    mask(0,2,2) || mask(1,2,2) || mask(2,2,2)) {
		return false;
	}
	return true;
}


class C18n3DShape: public C6n3DShape {
public:
	C18n3DShape();
};

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

class C18n3DShapeFactory: public C3DShapePlugin {
public:
	C18n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
	virtual bool  do_test() const;
};

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

bool C18n3DShapeFactory::do_test()const
{
	C18n3DShape shape;
	C18n3DShape::Mask mask = shape.get_mask();

	if (mask.get_size() != C3DBounds(3,3,3)) {
		cvfail() << get_name() << ": wrong mask size\n";
		assert(0);
	}

	if (mask(0,0,0) || !mask(1,0,0) || mask(2,0,0) ||
	   !mask(0,1,0) || !mask(1,1,0) ||!mask(2,1,0) ||
	    mask(0,2,0) || !mask(1,2,0) || mask(2,2,0) ||
	   !mask(0,0,1) || !mask(1,0,1) ||!mask(2,0,1) ||
	   !mask(0,1,1) || !mask(1,1,1) ||!mask(2,1,1) ||
	   !mask(0,2,1) || !mask(1,2,1) ||!mask(2,2,1) ||
	    mask(0,0,2) || !mask(1,0,2) || mask(2,0,2) ||
	   !mask(0,1,2) || !mask(1,1,2) ||!mask(2,1,2) ||
	    mask(0,2,2) || !mask(1,2,2) || mask(2,2,2)) {
		return false;
	}
	return true;
}


class C26n3DShape: public C18n3DShape {
public:
	C26n3DShape();
};

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

class C26n3DShapeFactory: public C3DShapePlugin {
public:
	C26n3DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C3DShape *do_create()const;
	virtual bool do_test() const;
};

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

bool C26n3DShapeFactory::do_test()const
{
	C26n3DShape shape;
	C26n3DShape::Mask mask = shape.get_mask();

	assert(mask.get_size() == C3DBounds(3,3,3));

	for (C26n3DShape::Mask::const_iterator i = mask.begin(), e = mask.end();
	     i != e; ++i)
		if (!*i) {
			return false;
		}
	return true;
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
