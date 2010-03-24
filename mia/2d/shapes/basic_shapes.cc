/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2005 - 2010
 *
 * Max-Planck-Institute for Evolutionary Anthropoloy
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include "sphere.hh"


NS_BEGIN(basic_2dshape_creator)
NS_MIA_USE;
using namespace std;


class C4n2DShape: public C2DShape {
public:
	C4n2DShape();
};

C4n2DShape::C4n2DShape()
{
	insert(C2DShape::Flat::value_type( 0, 0));
	insert(C2DShape::Flat::value_type(-1, 0));
	insert(C2DShape::Flat::value_type( 1, 0));
	insert(C2DShape::Flat::value_type( 0,-1));
	insert(C2DShape::Flat::value_type( 0, 1));
}

class C4n2DShapeFactory: public C2DShapePlugin {
public:
	C4n2DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C2DShapePlugin::ProductPtr do_create()const;
	virtual bool do_test() const;
};

C4n2DShapeFactory::C4n2DShapeFactory():
	C2DShapePlugin("4n")
{
}

C2DShapePlugin::ProductPtr C4n2DShapeFactory::do_create()const
{
	return C2DShapePlugin::ProductPtr(new C4n2DShape());
}


const string C4n2DShapeFactory::do_get_descr()const
{
	return string("4n neighborhood 2D shape creator");
}

bool C4n2DShapeFactory::do_test()const
{
	C4n2DShape shape;
	C4n2DShape::Mask mask = shape.get_mask();

	if (mask.get_size() != C2DBounds(3,3)) {
		cvfail() << get_name() << ": wrong mask size\n";
		assert("size failture");
	}

	if (! ( mask(1,1) && mask(1,2) && mask(0,1) && mask(1,0) &&
		mask(0,1) && mask(2,1) ) ||
	    mask(0,0) || mask(2,0) ||
	    mask(0,2) || mask(2,2)) {
		return false;
	}
	return true;
}


class C8n2DShape: public C4n2DShape {
public:
	C8n2DShape();
};

C8n2DShape::C8n2DShape()
{
	insert(C2DShape::Flat::value_type(-1,-1));
	insert(C2DShape::Flat::value_type(-1, 1));
	insert(C2DShape::Flat::value_type( 1,-1));
	insert(C2DShape::Flat::value_type( 1, 1));
}

class C8n2DShapeFactory: public C2DShapePlugin {
public:
	C8n2DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C2DShapePlugin::ProductPtr do_create()const;
	virtual bool  do_test() const;
};

C8n2DShapeFactory::C8n2DShapeFactory():
	C2DShapePlugin("8n")
{
}

C2DShapePlugin::ProductPtr C8n2DShapeFactory::do_create()const
{
	return C2DShapePlugin::ProductPtr(new C8n2DShape());
}


const string C8n2DShapeFactory::do_get_descr()const
{
	return string("8n neighborhood 2D shape creator");
}

bool C8n2DShapeFactory::do_test()const
{
	C8n2DShape shape;
	C8n2DShape::Mask mask = shape.get_mask();

	if (mask.get_size() != C2DBounds(3,3)) {
		cvfail() << get_name() << ": wrong mask size\n";
		assert(0);
	}

	if (mask(0,0) && mask(1,0) && mask(2,0) &&
	    mask(0,1) && mask(1,1) && mask(2,1) &&
	    mask(0,2) && mask(1,2) && mask(2,2))
		return true;

	return false;
}



extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *n6 = new C4n2DShapeFactory();
	n6->append_interface(new C8n2DShapeFactory());
	n6->append_interface(new CSphere2DShapeFactory());
	return n6;
}

NS_END
// end namespace _8n_2dshape_creator
