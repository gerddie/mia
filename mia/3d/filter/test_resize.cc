/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/internal/plugintester.hh> 
#include <mia/3d/filter/resize.hh>
#include <mia/template/filtertest.hh>


using namespace mia; 
using namespace std; 
using namespace resize_3d_filter; 

struct ResizeTestFixture : public TFiltertestFixture<T3DImage>{
	ResizeTestFixture(); 
	
	C3DBounds size; 
	C3DUIImage input; 
}; 

BOOST_FIXTURE_TEST_CASE ( test_resize, ResizeTestFixture ) 
{
	C3DBounds out_size_request(7, 2, 0);
	C3DBounds out_size_real(7, 2, 5);
	ostringstream fdescr; 
	fdescr << "resize:size=[" << out_size_request << "]"; 

	auto filter = BOOST_TEST_create_from_plugin<C3DResizeImageFilterPlugin>(fdescr.str().c_str());
	
	unsigned int test_data[7 * 2 * 5] = {
		0,   0,1010,1110,1210,   0,   0, 
		0,   0,1020,1120,1220,   0,   0, 

		0,   0,1011,1111,1211,   0,   0, 
		0,   0,1021,1121,1221,   0,   0, 

		0,   0,1012,1112,1212,   0,   0, 
		0,   0,1022,1122,1222,   0,   0, 

		0,   0,1013,1113,1213,   0,   0, 
		0,   0,1023,1123,1223,   0,   0, 

		0,   0,1014,1114,1214,   0,   0, 
		0,   0,1024,1124,1224,   0,   0, 
	}; 
	
	run(size, &input(0,0,0), out_size_real, test_data, *filter);
	
}

BOOST_FIXTURE_TEST_CASE ( test_resize_keep, ResizeTestFixture ) 
{
	C3DBounds out_size_request(0, 0, 0);
	ostringstream fdescr; 
	fdescr << "resize:size=[" << out_size_request << "]"; 

	auto filter = BOOST_TEST_create_from_plugin<C3DResizeImageFilterPlugin>(fdescr.str().c_str());
	
	
	run(size, &input(0,0,0), size, &input(0,0,0), *filter);
	
}


BOOST_FIXTURE_TEST_CASE ( test_resize_reduce_x, ResizeTestFixture ) 
{
	C3DBounds out_size_request(1, 0, 0);
	ostringstream fdescr; 
	fdescr << "resize:size=[" << out_size_request << "]"; 

	auto filter = BOOST_TEST_create_from_plugin<C3DResizeImageFilterPlugin>(fdescr.str().c_str());

	
	unsigned int test_data[1 * 4 * 5] = {
		1100, 1110, 1120, 1130,
		1101, 1111, 1121, 1131,
		1102, 1112, 1122, 1132,
		1103, 1113, 1123, 1133,
		1104, 1114, 1124, 1134,
	}; 

	run(size, &input(0,0,0), C3DBounds(1,4,5), test_data, *filter);
	
}

BOOST_FIXTURE_TEST_CASE ( test_resize_reduce_y, ResizeTestFixture ) 
{
	C3DBounds out_size_request(0, 2, 0);
	ostringstream fdescr; 
	fdescr << "resize:size=[" << out_size_request << "]"; 

	auto filter = BOOST_TEST_create_from_plugin<C3DResizeImageFilterPlugin>(fdescr.str().c_str());

	
	unsigned int test_data[3 * 2 * 5] = {
		1010, 1110, 1210, 
		1020, 1120, 1220, 

		1011, 1111, 1211, 
		1021, 1121, 1221, 

		1012, 1112, 1212, 
		1022, 1122, 1222, 

		1013, 1113, 1213, 
		1023, 1123, 1223, 

		1014, 1114, 1214, 
		1024, 1124, 1224
		
	}; 

	run(size, &input(0,0,0), C3DBounds(3,2,5), test_data, *filter);
	
}

BOOST_FIXTURE_TEST_CASE ( test_resize_reduce_z, ResizeTestFixture ) 
{
	C3DBounds out_size_request(0, 0, 3);
	ostringstream fdescr; 
	fdescr << "resize:size=[" << out_size_request << "]"; 

	auto filter = BOOST_TEST_create_from_plugin<C3DResizeImageFilterPlugin>(fdescr.str().c_str());

	
	unsigned int test_data[3 * 4* 3] = {
		1001, 1101, 1201, 
		1011, 1111, 1211, 
		1021, 1121, 1221, 
		1031, 1131, 1231, 

		1002, 1102, 1202, 
		1012, 1112, 1212, 
		1022, 1122, 1222, 
		1032, 1132, 1232,

		1003, 1103, 1203, 
		1013, 1113, 1213, 
		1023, 1123, 1223, 
		1033, 1133, 1233 
		
	}; 

	run(size, &input(0,0,0), C3DBounds(3,4,3), test_data, *filter);
	
}


ResizeTestFixture::ResizeTestFixture():
	size(3, 4, 5), 
	input(size)
{

	auto i =  input.begin_range(C3DBounds::_0, size); 
	auto e =  input.end_range(C3DBounds::_0, size); 
	
	while (i != e) {
		*i = 1000 + 100 * i.pos().x + 10 * i.pos().y + i.pos().z; 
		++i;
	}
}
	
