/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/3d/vectorfield.hh>
#include <mia/3d/datafield.cxx>
#include <mia/3d/iterator.cxx>

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


BOOST_AUTO_TEST_CASE (test_access_coordinates) 
{
	C3DBounds size(3,4,5); 
	T3DDatafield<C3DBounds> field(size);

	C3DBounds l; 
	for(l.z = 0; l.z < size.z; ++l.z) 
		for(l.y = 0; l.y < size.y; ++l.y) 
			for(l.x = 0; l.x < size.x; ++l.x) 
				field(l) = l; 


	auto begin = field.begin_range(C3DBounds(0,0,0), size);
	auto end = field.begin_range(C3DBounds(0,0,0), size);
	while (begin != end) {
		BOOST_CHECK_EQUAL(*begin, begin.pos());
		++begin; 
	}
}




BOOST_AUTO_TEST_CASE (test_iterator_boundaries) 
{
	C3DBounds size(7,5,6); 
	C3DFDatafield field(size);

	C3DBounds start(0,0,0);
	T3DDatafield<int> test(size); 
	
	fill(test.begin(), test.end(), C3DFDatafield::range_iterator_with_boundary_flag::eb_none); 

	fill(test.begin_at(0,0,0), test.begin_at(0,0,1), C3DFDatafield::range_iterator_with_boundary_flag::eb_zlow); 
	fill(test.begin_at(0,0,size.z - 1), test.begin_at(0,0,size.z), C3DFDatafield::range_iterator_with_boundary_flag::eb_zhigh); 

	for (unsigned int  z = 0; z < size.z; ++z) 
		for (unsigned int  y = 0; y < size.y; ++y) {
			test(0,y,z) |= C3DFDatafield::range_iterator_with_boundary_flag::eb_xlow; 
			test(size.x - 1,y,z) |= C3DFDatafield::range_iterator_with_boundary_flag::eb_xhigh;
		}

	for (unsigned int  z = 0; z < size.z; ++z) 
		for (unsigned int  x = 0; x < size.x; ++x) {
			test(x,0,z) |= C3DFDatafield::range_iterator_with_boundary_flag::eb_ylow; 
			test(x, size.y-1,z) |= C3DFDatafield::range_iterator_with_boundary_flag::eb_yhigh;
		}
	auto ifield = field.begin_range(C3DBounds(0,0,0), size).with_boundary_flag(); 
	auto efield = field.end_range(C3DBounds(0,0,0), size).with_boundary_flag(); 
	auto itest = test.begin(); 
	
	for (;ifield != efield; ++ifield, ++itest) {
		BOOST_CHECK_EQUAL(ifield.get_boundary_flags(), *itest); 
	}
}

BOOST_AUTO_TEST_CASE (test_iterator_some_boundaries) 
{
	C3DBounds size(7,5,6); 
	C3DFDatafield field(size);

	auto ifield = field.begin_range(C3DBounds(1,1,0), size - C3DBounds(0,1,1)).with_boundary_flag(); 
	auto efield = field.end_range(C3DBounds(1,1,0), size - C3DBounds(0,1,1)).with_boundary_flag(); 
	
	for (;ifield != efield; ++ifield) {
		cvdebug() <<ifield.pos()<< " : " << ifield.get_boundary_flags() << "\n"; 
		BOOST_CHECK_EQUAL(ifield.pos().x == 0, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_xlow)); 
		BOOST_CHECK_EQUAL(ifield.pos().y == 0, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_ylow)); 
		BOOST_CHECK_EQUAL(ifield.pos().z == 0, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_zlow)); 
		
		BOOST_CHECK_EQUAL(ifield.pos().x == 6, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_xhigh)); 
		BOOST_CHECK_EQUAL(ifield.pos().y == 4, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_yhigh)); 
		BOOST_CHECK_EQUAL(ifield.pos().z == 5, bool(ifield.get_boundary_flags() & 
							    C3DFDatafield::range_iterator_with_boundary_flag::eb_zhigh)); 
	}
}

BOOST_AUTO_TEST_CASE (test_iterator_no_boundaries) 
{
	C3DBounds size(7,5,6); 
	C3DFDatafield field(size);

	C3DBounds start(1,1,1);
	
	auto ifield = field.begin_range(C3DBounds(1,1,1), size - C3DBounds::_1).with_boundary_flag(); 
	auto efield = field.end_range(C3DBounds(1,1,1), size - C3DBounds::_1).with_boundary_flag(); 
	
	for (;ifield != efield; ++ifield) {
		BOOST_CHECK_EQUAL(ifield.get_boundary_flags(), 0); 
	}
}

// this tests whether the traits are properly set
BOOST_AUTO_TEST_CASE (test_std_copy) 
{
	C3DBounds size(7,5,6); 
	C3DFVectorfield in_field(size);
	C3DBounds start(1,2,2); 
	C3DBounds end(6,3,5); 
	C3DBounds out_size = end - start; 
	C3DFVectorfield out_field(out_size);

	auto ibegin = in_field.begin_range(start, end);
	auto iend = in_field.end_range(start, end);
	
	while (ibegin != iend) {
		*ibegin = C3DFVector(ibegin.pos()); 
		++ibegin; 
	}

	std::copy(in_field.begin_range(start, end), in_field.end_range(start, end), out_field.begin()); 
	
	auto ofb = out_field.begin_range(C3DBounds::_0, out_size); 
	auto ofe = out_field.end_range(C3DBounds::_0, out_size); 
	
	while (ofb != ofe) {
		BOOST_CHECK_EQUAL(*ofb, C3DFVector(ofb.pos() + start)); 
		++ofb; 
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
