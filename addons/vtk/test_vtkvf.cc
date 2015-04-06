/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#define VSTREAM_DOMAIN "vtkVfIOtest"

#include <mia/internal/autotest.hh>
#include <vtk/vtkvf.hh>
#include <unistd.h>

using namespace mia; 
using namespace std; 
using namespace vtkvf; 


BOOST_AUTO_TEST_CASE( test_simple_write_read ) 
{
	C3DBounds size(2,3,4);
	C3DIOVectorfield vf(size); 

	auto iv = vf.begin_range(C3DBounds::_0, size); 
	auto ev = vf.end_range(C3DBounds::_0, size);

	while (iv != ev) {
		*iv = C3DFVector(iv.pos());
		++iv; 
	}

	CVtk3DVFIOPlugin io; 
	BOOST_REQUIRE(io.save("testvf.vtk", vf)); 
	
	auto loaded = io.load("testvf.vtk"); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->get_size() == size); 
	
	iv = vf.begin_range(C3DBounds::_0, size); 
	auto il = loaded->begin_range(C3DBounds::_0, size); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}
        unlink("testvf.vtk"); 
}
