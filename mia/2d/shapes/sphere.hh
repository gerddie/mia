/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#ifndef _mia_3d_sphere_creator_hh
#define _mia_3d_sphere_creator_hh

#include <mia/2d/shape.hh>

NS_MIA_BEGIN

class CSphere2DShapeFactory: public C2DShapePlugin {
public:
	CSphere2DShapeFactory();
private:
	virtual const string do_get_descr() const;
	virtual C2DShape *do_create()const;
	virtual bool do_test() const;
	float m_r;
};

NS_MIA_END
#endif
