/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/internal/plugintester.hh> 
#include <mia/3d/creator/sphere.hh>

using namespace creator_sphere_3d;
using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_sphere ) 
{
	C3DBounds size(10,20,30); 
	auto f = BOOST_TEST_create_from_plugin<C3DSphereCreatorPlugin>("sphere:p=2,f=2");

	auto image = (*f)(size, it_float); 
	BOOST_CHECK_EQUAL(image->get_size(), size); 
	

}


