/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#define VSTREAM_DOMAIN "ITERATOR"

#include <mia/internal/autotest.hh>
#include <mia/2d/vectorfield.hh>
#include <mia/2d/datafield.cxx>
#include <mia/2d/iterator.cxx>

NS_MIA_USE;

typedef C2DFVectorfield::range_iterator range2d_vfiterator; 
typedef C2DFVectorfield::const_range_iterator const_range2d_vfiterator; 


struct VFIteratorFixture {
	VFIteratorFixture(); 
	
	C2DBounds size; 
	C2DFVectorfield field;
}; 

BOOST_FIXTURE_TEST_CASE (test_lala, VFIteratorFixture) 
{
	range2d_vfiterator begin(C2DBounds(2,2), size, 
				 C2DBounds(2,2), C2DBounds(4,5), 
				 field.begin_at(2,2)); 
	
	range2d_vfiterator end(C2DBounds(4,5));

	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C2DFVector(begin.pos())); 
		++begin; 
	}

	range2d_vfiterator begin2(C2DBounds(0,0), size, 
				  C2DBounds(0,0), size, 
				  field.begin_at(0,0)); 
	
	range2d_vfiterator end2(size);
	while (begin2 != end2) {
		BOOST_CHECK_EQUAL(*begin2, C2DFVector(begin2.pos())); 
		++begin2; 
	}
	
}

BOOST_AUTO_TEST_CASE (test_fill_all) 
{
	C2DBounds size(3,4); 
	C2DFVectorfield field(size);


	range2d_vfiterator begin = field.begin_range(C2DBounds(0,0), size);
	range2d_vfiterator end = field.end_range(C2DBounds(0,0), size);
	while (begin != end) {
		*begin = C2DFVector(begin.pos()); 
		++begin; 
	}
	begin = field.begin_range(C2DBounds(0,0), size);
	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C2DFVector(begin.pos())); 
		++begin; 
	}
}


BOOST_AUTO_TEST_CASE (test_access_coordinates) 
{
	C2DBounds size(3,4); 
	T2DDatafield<C2DBounds> field(size);

	C2DBounds l; 
	
	for(l.y = 0; l.y < size.y; ++l.y) 
		for(l.x = 0; l.x < size.x; ++l.x) 
			field(l) = l; 


	auto begin = field.begin_range(C2DBounds(0,0), size);
	auto end = field.begin_range(C2DBounds(0,0), size);
	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, begin.pos());
		++begin; 
	}
}




BOOST_AUTO_TEST_CASE (test_iterator_boundaries) 
{
	C2DBounds size(7,5); 
	C2DFDatafield field(size);

	C2DBounds start(0,0);
	T2DDatafield<int> test(size); 
	
	fill(test.begin(), test.end(), C2DFDatafield::range_iterator::eb_none); 
	
	for (unsigned int  y = 0; y < size.y; ++y) {
		test(0,y) |= C2DFDatafield::range_iterator::eb_xlow; 
		test(size.x - 1,y) |= C2DFDatafield::range_iterator::eb_xhigh;
	}
	
	for (unsigned int  x = 0; x < size.x; ++x) {
		test(x,0) |= C2DFDatafield::range_iterator::eb_ylow; 
		test(x, size.y-1) |= C2DFDatafield::range_iterator::eb_yhigh;
	}
	auto ifield = field.begin_range(C2DBounds(0,0), size); 
	auto efield = field.end_range(C2DBounds(0,0), size); 
	auto itest = test.begin(); 
	
	for (;ifield != efield; ++ifield, ++itest) {
		BOOST_CHECK_EQUAL(ifield.get_boundary_flags(), *itest); 
	}
}

BOOST_AUTO_TEST_CASE (test_iterator_some_boundaries) 
{
	C2DBounds size(7,5); 
	C2DFDatafield field(size);

	auto ifield = field.begin_range(C2DBounds(1,1), size - C2DBounds(0,1)); 
	auto efield = field.end_range(C2DBounds(1,1), size - C2DBounds(0,1)); 
	
	for (;ifield != efield; ++ifield) {
		cvdebug() <<ifield.pos()<< " : " << ifield.get_boundary_flags() << "\n"; 
		BOOST_CHECK_EQUAL(ifield.pos().x == 0, bool(ifield.get_boundary_flags() & C2DFDatafield::range_iterator::eb_xlow)); 
		BOOST_CHECK_EQUAL(ifield.pos().y == 0, bool(ifield.get_boundary_flags() & C2DFDatafield::range_iterator::eb_ylow)); 
		
		BOOST_CHECK_EQUAL(ifield.pos().x == 6, bool(ifield.get_boundary_flags() & C2DFDatafield::range_iterator::eb_xhigh)); 
		BOOST_CHECK_EQUAL(ifield.pos().y == 4, bool(ifield.get_boundary_flags() & C2DFDatafield::range_iterator::eb_yhigh)); 
	}
}

BOOST_AUTO_TEST_CASE (test_iterator_no_boundaries) 
{
	C2DBounds size(7,5); 
	C2DFDatafield field(size);

	C2DBounds start(1,1);
	
	auto ifield = field.begin_range(C2DBounds(1,1), size - C2DBounds::_1); 
	auto efield = field.end_range(C2DBounds(1,1), size - C2DBounds::_1); 
	
	for (;ifield != efield; ++ifield) {
		BOOST_CHECK_EQUAL(ifield.get_boundary_flags(), 0); 
	}
}

	
BOOST_AUTO_TEST_CASE (test_fill_part) 
{
	C2DBounds size(7,5); 
	C2DFVectorfield field(size);
	C2DBounds start(1,2); 
	C2DBounds end(6,3); 


	range2d_vfiterator ibegin = field.begin_range(start, end);
	range2d_vfiterator iend = field.end_range(start, end);
	
	while (ibegin != iend) {
		*ibegin = C2DFVector(ibegin.pos()); 
		++ibegin; 
	}
	
	const_range2d_vfiterator ibegin2 = field.begin_range(C2DBounds(0,0), size);
	const_range2d_vfiterator iend2 = field.end_range(C2DBounds(0,0), size);

	while (ibegin2 != iend2) {
		if (start < ibegin.pos()  && ibegin.pos() < end)
			BOOST_CHECK_EQUAL(*ibegin2, C2DFVector(ibegin2.pos())); 
		else 
			BOOST_CHECK_EQUAL(*ibegin, C2DFVector::_0); 
		++ibegin2; 
	}
}


BOOST_FIXTURE_TEST_CASE (test_const_lala, VFIteratorFixture) 
{
	const_range2d_vfiterator begin(C2DBounds(2,2), size, 
				 C2DBounds(2,2), C2DBounds(4,5), 
				 field.begin_at(2,2)); 
	
	const_range2d_vfiterator end(C2DBounds(4,5));

	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, C2DFVector(begin.pos())); 
		++begin; 
	}

	const_range2d_vfiterator begin2(C2DBounds(0,0), size, 
				  C2DBounds(0,0), size, 
				  field.begin_at(0,0)); 
	
	const_range2d_vfiterator end2(size);
	while (begin2 != end2) {
		BOOST_CHECK_EQUAL(*begin2, C2DFVector(begin2.pos())); 
		++begin2; 
	}
	
}



VFIteratorFixture::VFIteratorFixture():
	size(5,6), 
	field(size)
{
	auto i = field.begin(); 
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++i) 
			*i = C2DFVector(x,y); 
	
}
