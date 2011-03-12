/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "ITERATOR"

#include <mia/3d/iterator.hh>
#include <mia/3d/iterator.cxx>

#include <mia/3d/3DVectorfield.hh>

NS_MIA_USE;

typedef range3d_iterator<C3DFVectorfield> range3d_vfiterator; 


BOOST_AUTO_TEST_CASE (test_lala) 
{
	C3DBounds size(5,6,7); 
	C3DFVectorfield field(size);
	
	auto i = field.begin(); 
	for (size_z z = 0; z < size.z; ++z) 
		for (size_z y = 0; y < size.y; ++y) 
			for (size_z x = 0; x < size.x; ++x, ++i) 
				*i = C3FVector(x,y,z); 

	
	range3d_vfiterator begin(C3DBounds(2,2,2), size, 
				 C3DBounds(2,2,2), C3DBounds(4,5,6), 
				 field.begin_at(2,2,2)); 

	range3d_vfiterator end(C3DBounds(4,5,6), size, 
			       C3DBounds(2,2,2), C3DBounds(4,5,6), 
			       field.begin_at(4,5,6)); 

	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, begin->pos()); 
		++begin; 
	}

		
}
