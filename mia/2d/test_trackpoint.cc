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

#include <mia/internal/autotest.hh>
#include <mia/2d/trackpoint.hh>
#include <mia/2d/transformfactory.hh>
#include <sstream>

using namespace std; 
using namespace mia; 

const char test_input[] = 
	"1;12;10;5;some text\n"
	"2;13;7;5;other text\n"
	"3;14;1;2\n";


BOOST_AUTO_TEST_CASE ( test_read_points ) 
{
	istringstream is(test_input); 
	std::vector<C2DTrackPoint> result; 

	while (is.good()) {
		string line; 
		getline( is, line); 

		C2DTrackPoint tp; 
		if (tp.read(line)) 
			result.push_back(tp); 
		else 
			is.setstate(ios_base::badbit); 
	}

	BOOST_CHECK(is.eof()); 

	BOOST_REQUIRE(result.size() == 3u); 
	
	const  C2DTrackPoint&  tp0 = result[0]; 
	
	BOOST_CHECK_EQUAL(tp0.get_id(), 1); 
	BOOST_CHECK_EQUAL(tp0.get_pos(), C2DFVector(10,5));
	BOOST_CHECK_EQUAL(tp0.get_time(), 12);
	BOOST_CHECK_EQUAL(tp0.get_reserved(), "some text");

	const  C2DTrackPoint&  tp1 = result[1]; 
	
	BOOST_CHECK_EQUAL(tp1.get_id(), 2); 
        BOOST_CHECK_EQUAL(tp1.get_pos(), C2DFVector(7,5));
	BOOST_CHECK_EQUAL(tp1.get_time(), 13);
	BOOST_CHECK_EQUAL(tp1.get_reserved(), "other text");

	const  C2DTrackPoint&  tp2 = result[2]; 
	
	BOOST_CHECK_EQUAL(tp2.get_id(), 3); 
        BOOST_CHECK_EQUAL(tp2.get_pos(), C2DFVector(1,2));
	BOOST_CHECK_EQUAL(tp2.get_time(), 14);
	BOOST_CHECK(tp2.get_reserved().empty());
	
	
}

BOOST_AUTO_TEST_CASE ( test_write_points ) 
{
	C2DTrackPoint tp0(5, 10.0, C2DFVector(12,3), "a string"); 
	C2DTrackPoint tp1(6, 11.0, C2DFVector(1.1,2), "a bling"); 
	C2DTrackPoint tp2(7, 12.0, C2DFVector(2.1,3), ""); 
	
	ostringstream os; 
	os << tp0 << "\n"; 
	os << tp1 << "\n";
	os << tp2 << "\n";

	BOOST_CHECK_EQUAL(os.str(), "5;10;12;3;a string\n"
			  "6;11;1.1;2;a bling\n"
			  "7;12;2.1;3\n"
		);

}

BOOST_AUTO_TEST_CASE ( test_move ) 
{
	C2DTrackPoint tp0(5, 10.0, C2DFVector(1,0), "a string"); 

	auto tc = produce_2dtransform_factory("vf"); 

	auto t = tc->create(C2DBounds(2,1)); 
	
	auto params = t->get_parameters(); 
	
	BOOST_REQUIRE(params.size() == 4); 

	params[0] = 2.0; 
	params[1] = -1.0; 
	params[2] = 1.0; 
	params[3] = 0.0;
	
	t->set_parameters(params); 
	
	tp0.move(1, *t); 
	
	BOOST_CHECK_EQUAL(tp0.get_time(), 11.0); 
	BOOST_CHECK_EQUAL(tp0.get_pos(), C2DFVector(0,0)); 
	
	tp0.move(0.4, *t); 

	BOOST_CHECK_CLOSE(tp0.get_time(), 11.4, 0.1); 

	const C2DFVector& p = tp0.get_pos(); 
	

	BOOST_CHECK_CLOSE(p.x, -0.8, 0.1); 
	BOOST_CHECK_CLOSE(p.y,  0.4, 0.1); 

	
}
