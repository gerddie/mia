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

#include <mia/internal/autotest.hh>
#include <mia/3d/3DVectorfield.hh>

NS_MIA_USE;

typedef C3DFVectorfield::range_iterator range3d_vfiterator; 
typedef C3DFVectorfield::const_range_iterator const_range3d_vfiterator; 


struct VFIteratorFixture {
	VFIteratorFixture(); 
	
	C3DBounds size; 
	C3DFVectorfield field;
}; 

BOOST_FIXTURE_TEST_CASE (test_lala, VFIteratorFixture) 
{
	range3d_vfiterator begin(C3DBounds(2,2,2), size, 
				 C3DBounds(2,2,2), C3DBounds(4,5,6), 
				 field.begin_at(2,2,2)); 
	
	range3d_vfiterator end(C3DBounds(4,5,6));

	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C3DFVector(begin.pos())); 
		++begin; 
	}

	range3d_vfiterator begin2(C3DBounds(0,0,0), size, 
				  C3DBounds(0,0,0), size, 
				  field.begin_at(0,0,0)); 
	
	range3d_vfiterator end2(size);
	while (begin2 != end2) {
		BOOST_CHECK_EQUAL(*begin2, C3DFVector(begin2.pos())); 
		++begin2; 
	}
	
}

BOOST_AUTO_TEST_CASE (test_fill_all) 
{
	C3DBounds size(3,4,5); 
	C3DFVectorfield field(size);


	range3d_vfiterator begin = field.begin_range(C3DBounds(0,0,0), size);
	range3d_vfiterator end = field.end_range(C3DBounds(0,0,0), size);
	while (begin != end) {
		*begin = C3DFVector(begin.pos()); 
		++begin; 
	}
	begin = field.begin_range(C3DBounds(0,0,0), size);
	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C3DFVector(begin.pos())); 
		++begin; 
	}
}

BOOST_AUTO_TEST_CASE (test_fill_part) 
{
	C3DBounds size(7,5,6); 
	C3DFVectorfield field(size);
	C3DBounds start(1,2,2); 
	C3DBounds end(6,3,5); 


	range3d_vfiterator ibegin = field.begin_range(start, end);
	range3d_vfiterator iend = field.end_range(start, end);
	
	while (ibegin != iend) {
		*ibegin = C3DFVector(ibegin.pos()); 
		++ibegin; 
	}
	
	const_range3d_vfiterator ibegin2 = field.begin_range(C3DBounds(0,0,0), size);
	const_range3d_vfiterator iend2 = field.end_range(C3DBounds(0,0,0), size);

	while (ibegin2 != iend2) {
		if (ibegin.pos() >= start && ibegin.pos() < end)
			BOOST_CHECK_EQUAL(*ibegin2, C3DFVector(ibegin2.pos())); 
		else 
			BOOST_CHECK_EQUAL(*ibegin, C3DFVector::_0); 
		++ibegin2; 
	}
}


BOOST_FIXTURE_TEST_CASE (test_const_lala, VFIteratorFixture) 
{
	const_range3d_vfiterator begin(C3DBounds(2,2,2), size, 
				 C3DBounds(2,2,2), C3DBounds(4,5,6), 
				 field.begin_at(2,2,2)); 
	
	const_range3d_vfiterator end(C3DBounds(4,5,6));

	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C3DFVector(begin.pos())); 
		++begin; 
	}

	const_range3d_vfiterator begin2(C3DBounds(0,0,0), size, 
				  C3DBounds(0,0,0), size, 
				  field.begin_at(0,0,0)); 
	
	const_range3d_vfiterator end2(size);
	while (begin2 != end2) {
		BOOST_CHECK_EQUAL(*begin2, C3DFVector(begin2.pos())); 
		++begin2; 
	}
	
}



VFIteratorFixture::VFIteratorFixture():
	size(5,6,7), 
	field(size)
{
	auto i = field.begin(); 
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++i) 
				*i = C3DFVector(x,y,z); 
	
}
