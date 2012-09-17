/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 David Paster, Gert Wollny
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
#include <mia/3d/trackpoint.hh>
#include <mia/3d/transformfactory.hh>
#include <sstream>

using namespace std; 
using namespace mia; 

C3DTransformCreatorHandlerTestPath test_creator_path; 


const char test_input[] = 
	"1;12;10;5;12;some text\n"
	"2;13;7;5;2;other text\n"
	"3;14;1;2;3\n";


BOOST_AUTO_TEST_CASE ( test_read_points ) 
{
	istringstream is(test_input); 
	std::vector<C3DTrackPoint> result; 

	while (is.good()) {
		string line; 
		getline( is, line); 

		C3DTrackPoint tp; 
		if (tp.read(line)) 
			result.push_back(tp); 
		else 
			is.setstate(ios_base::badbit); 
	}

	BOOST_CHECK(is.eof()); 

	BOOST_REQUIRE(result.size() == 3u); 
	
	const  C3DTrackPoint&  tp0 = result[0]; 
	
	BOOST_CHECK_EQUAL(tp0.get_id(), 1); 
	BOOST_CHECK_EQUAL(tp0.get_pos(), C3DFVector(10,5,12));
	BOOST_CHECK_EQUAL(tp0.get_time(), 12);
	BOOST_CHECK_EQUAL(tp0.get_reserved(), "some text");

	const  C3DTrackPoint&  tp1 = result[1]; 
	
	BOOST_CHECK_EQUAL(tp1.get_id(), 2); 
        BOOST_CHECK_EQUAL(tp1.get_pos(), C3DFVector(7,5,2));
	BOOST_CHECK_EQUAL(tp1.get_time(), 13);
	BOOST_CHECK_EQUAL(tp1.get_reserved(), "other text");

	const  C3DTrackPoint&  tp2 = result[2]; 
	
	BOOST_CHECK_EQUAL(tp2.get_id(), 3); 
        BOOST_CHECK_EQUAL(tp2.get_pos(), C3DFVector(1,2,3));
	BOOST_CHECK_EQUAL(tp2.get_time(), 14);
	BOOST_CHECK(tp2.get_reserved().empty());
	
	
}

BOOST_AUTO_TEST_CASE ( test_write_points ) 
{
	C3DTrackPoint tp0(5, 10.0, C3DFVector(12,3,16.2), "a string"); 
	C3DTrackPoint tp1(6, 11.0, C3DFVector(1.1,2,1.2), "a bling"); 
	C3DTrackPoint tp2(7, 12.0, C3DFVector(2.1,3,4.2), ""); 
	
	ostringstream os; 
	os << tp0 << "\n"; 
	os << tp1 << "\n";
	os << tp2 << "\n";

	BOOST_CHECK_EQUAL(os.str(), "5;10;12;3;16.2;a string\n"
			  "6;11;1.1;2;1.2;a bling\n"
			  "7;12;2.1;3;4.2\n"
		);

}

BOOST_AUTO_TEST_CASE ( test_move ) 
{
	C3DTrackPoint tp0(5, 10.0, C3DFVector(1,0,0), "a string"); 

	auto tc = produce_3dtransform_creator("vf"); 

	auto t = tc->create(C3DBounds(2,1,1)); 
	
	auto params = t->get_parameters(); 
	
	BOOST_REQUIRE(params.size() == 6); 

	params[0] = 2.0; 
	params[1] = -1.0; 
	params[2] = 4.0; 
	params[3] = 1.0;
	
	t->set_parameters(params); 
	
	tp0.move(1, *t); 
	
	BOOST_CHECK_EQUAL(tp0.get_time(), 11.0); 
	BOOST_CHECK_EQUAL(tp0.get_pos(), C3DFVector(0,0,0)); 
	
	tp0.move(0.4, *t); 

	BOOST_CHECK_CLOSE(tp0.get_time(), 11.4, 0.1); 

	const C3DFVector& p = tp0.get_pos(); 
	

	BOOST_CHECK_CLOSE(p.x, -0.8, 0.1); 
	BOOST_CHECK_CLOSE(p.y,  0.4, 0.1); 
	BOOST_CHECK_CLOSE(p.z, -1.6, 0.1); 

	
}
