/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <climits>

#define VSTREAM_DOMAIN "test-transio"
#include <mia/internal/autotest.hh>


#include <mia/core.hh>
#include <mia/3d/vectorfield.hh>

NS_MIA_USE; 
using namespace std; 


struct C3DVectorfieldFixture {
	C3DVectorfieldFixture(); 
	C3DBounds size; 
	C3DFVectorfield field; 
}; 

BOOST_FIXTURE_TEST_CASE(  test_3ddatafield_zslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.x * size.y * 3); 
	
	field.read_zslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,y,1).x);
			BOOST_CHECK_EQUAL(i[1], field(x,y,1).y);
			BOOST_CHECK_EQUAL(i[2], field(x,y,1).z);
		}
			
	field.write_zslice_flat(2, slice); 
	i = slice.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,y,2).x);
			BOOST_CHECK_EQUAL(i[1], field(x,y,2).y);
			BOOST_CHECK_EQUAL(i[2], field(x,y,2).z);
		}
}


BOOST_FIXTURE_TEST_CASE( test_3ddatafield_yslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.x * size.z * 3); 
	
	field.read_yslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,1,z).x);
			BOOST_CHECK_EQUAL(i[1], field(x,1,z).y);
			BOOST_CHECK_EQUAL(i[2], field(x,1,z).z);
		}

	field.write_yslice_flat(2, slice); 
	i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t x = 0; x < size.x; ++x, i+=3) {
			BOOST_CHECK_EQUAL(i[0], field(x,2,z).x);
			BOOST_CHECK_EQUAL(i[1], field(x,2,z).y);
			BOOST_CHECK_EQUAL(i[2], field(x,2,z).z);
		}
}

BOOST_FIXTURE_TEST_CASE( test_3ddatafield_xslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.y * size.z * 3); 
	
	field.read_xslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y, i+=3){
			BOOST_CHECK_EQUAL(i[0], field(1,y,z).x);
			BOOST_CHECK_EQUAL(i[1], field(1,y,z).y);
			BOOST_CHECK_EQUAL(i[2], field(1,y,z).z);
		}
		

	field.write_xslice_flat(0, slice); 
	i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y, i+=3){
			BOOST_CHECK_EQUAL(i[0], field(0,y,z).x);
			BOOST_CHECK_EQUAL(i[1], field(0,y,z).y);
			BOOST_CHECK_EQUAL(i[2], field(0,y,z).z);
		}
}


C3DVectorfieldFixture::C3DVectorfieldFixture():
	size(4,5,6), 
	field(size)
{
	auto i = field.begin(); 
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++i)
				*i = C3DFVector(x+1, y+1, z+1); 
}


NS_MIA_USE


