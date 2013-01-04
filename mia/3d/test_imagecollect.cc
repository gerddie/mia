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

#include <mia/internal/autotest.hh>
#include <mia/3d/imagecollect.hh>
using namespace mia; 

BOOST_AUTO_TEST_CASE( test_collect_slices )
{
	C2DBounds size(2,3); 
	C2DUIImage slice0(size); 
	fill(slice0.begin(), slice0.end(), 1); 
	
	unsigned char k = 0; 
	C2DUIImage slice1(size); 
	for (auto i = slice1.begin(); i != slice1.end(); ++i, ++k) 
		*i =k; 
	
	C3DImageCollector ic(2);
        ic.add(slice0); 
        ic.add(slice1);

	auto result = ic.get_result(); 

	BOOST_REQUIRE(result->get_pixel_type() == it_uint); 
	BOOST_REQUIRE(result->get_size() == C3DBounds(2,3,2));

	auto r = dynamic_cast<const C3DUIImage&>(*result); 

	const unsigned int test[12] = {
		1,1,1,1,1,1,
		0,1,2,3,4,5
	}; 
	const unsigned int *t = test; 
	
	for (auto i = r.begin(); i != r.end(); ++i, ++t)
		BOOST_CHECK_EQUAL(*i,*t); 
	
}

