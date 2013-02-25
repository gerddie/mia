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

#include <mia/internal/autotest.hh>
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/fifof/label.hh>
#include <mia/2d/shape.hh>

NS_MIA_USE; 

using namespace std; 
using namespace label_2dstack_filter; 
namespace bfs=::boost::filesystem;


BOOST_FIXTURE_TEST_CASE( test_fifof_label , fifof_Fixture )
{
	CPathNameArray shape2dsearchpath;
	shape2dsearchpath.push_back(bfs::path("..")/bfs::path("..")/
				   bfs::path("2d")/bfs::path("shapes"));
	C2DShapePluginHandler::set_search_path(shape2dsearchpath);


	const size_t n_slices = 6; 
	const C2DBounds size(4,4); 
	
	bool input_data[n_slices * 4 * 4] = { 
		  0, 0, 0, 0,  
		  1, 0, 0, 0,   
		  0, 0, 0, 0,
		  0, 1, 0, 0,

		  0, 0, 0, 0,  
		  1, 0, 0, 0,
		  0, 0, 0, 0,
		  0, 1, 0, 1,
		  
		  1, 0, 0, 0, 
		  1, 0, 0, 0, 
		  0, 0, 0, 0, 
		  0, 1, 0, 1, 

		  0, 1, 0, 0, 
		  0, 0, 0, 0,  
		  0, 0, 1, 0,  
		  0, 1, 0, 1,  

		  0, 0, 0, 0,  
		  1, 0, 0, 0,   
		  0, 0, 1, 0,
		  0, 1, 1, 1,

		  0, 0, 0, 0,  
		  1, 0, 0, 0,
		  0, 0, 0, 0,
		  0, 1, 0, 1,

	};

	unsigned short test_data[n_slices * 4 * 4] = {
		  0, 0, 0, 0,  
		  1, 0, 0, 0,   
		  0, 0, 0, 0,
		  0, 2, 0, 0,

		  0, 0, 0, 0,  
		  1, 0, 0, 0,
		  0, 0, 0, 0,
		  0, 2, 0, 3,
		  
		  1, 0, 0, 0, 
		  1, 0, 0, 0, 
		  0, 0, 0, 0, 
		  0, 2, 0, 3, 

		  0, 4, 0, 0, 
		  0, 0, 0, 0,  
		  0, 0, 5, 0,  
		  0, 2, 0, 3,  

		  0, 0, 0, 0,  
		  6, 0, 0, 0,   
		  0, 0, 5, 0,
		  0, 5, 5, 5,

		  0, 0, 0, 0,  
		  6, 0, 0, 0,
		  0, 0, 0, 0,
		  0, 5, 0, 5,

	};

	prepare(input_data, test_data, size, n_slices); 

	auto shape = C2DShapePluginHandler::instance().produce("4n"); 
	C2DLabelStackFilter filter("", shape ); 
	call_test(filter);

}

class C1n2DShape: public C2DShape {
public:
	C1n2DShape() {
		insert(C2DShape::Flat::value_type( 0, 0));
	}
};


BOOST_AUTO_TEST_CASE( test_labelremap ) 
{
	CLabelRemapper remap; 

	remap.add_pair(1,2); 
	remap.add_pair(2,3); 
	remap.add_pair(3,4); 
	remap.add_pair(13,7); 
	remap.add_pair(13,1); 
	
	
	CLabelMap result = remap.get_map(); 
	
	BOOST_CHECK_EQUAL(result.size(), 5u); 
	BOOST_REQUIRE(result.find(3) != result.end()); 
	BOOST_REQUIRE(result.find(2) != result.end()); 
	BOOST_REQUIRE(result.find(4) != result.end()); 
	BOOST_REQUIRE(result.find(7) != result.end()); 
	BOOST_REQUIRE(result.find(13) != result.end()); 
	BOOST_CHECK_EQUAL(result[2], 1); 
	BOOST_CHECK_EQUAL(result[3], 1); 
	BOOST_CHECK_EQUAL(result[4], 1); 
	BOOST_CHECK_EQUAL(result[7], 1); 
	BOOST_CHECK_EQUAL(result[13], 1); 

}	


BOOST_AUTO_TEST_CASE( test_overflow ) 
{
	const C2DBounds size(300,300);
	C2DBitImage *img(new C2DBitImage(size)); 
	fill(img->begin(), img->end(), 1); 
	P2DImage pimg(img); 
	
	P2DShape shape(new C1n2DShape()); 
	
	C2DLabelStackFilter filter("", shape ); 

	typedef TFifoFilterSink<P2DImage> C2DImageFifoFilterSink;

	C2DImageFifoFilterSink::Pointer sink(new C2DImageFifoFilterSink());
	filter.append_filter(sink);
	
	BOOST_CHECK_THROW(filter.push(pimg), invalid_argument); 

}
