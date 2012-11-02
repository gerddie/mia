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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <iostream>

#include <mia/core.hh>
#include <mia/2d/datafield.hh>
NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_2ddatafield )
{
	const float values[20]=
		{ 0,  1,  2,  3,  4,
		  2,  3,  4,  5,  6,
		  4,  5,  6,  7,  8,
		  6,  7,  8,  9, 10 };

	C2DFDatafield data(C2DBounds(5,4), values);

	BOOST_CHECK(data.get_size()  == C2DBounds(5,4));
	BOOST_CHECK(data.size() == 20);
	BOOST_CHECK(data(0,0) == 0);
	BOOST_CHECK(data(2,2) == 6);

	// 4.25 * 0.25 + 6.25 *0.75
	float ip = data.get_interpol_val_at(C2DFVector(2.25, 1.75));
	BOOST_CHECK(fabs(ip - 5.75) < 0.0001);

	std::vector<float> xbuffer;
	data.get_data_line_x(2, xbuffer);
	BOOST_CHECK(xbuffer.size() == 5);
	BOOST_CHECK(xbuffer[0] == 4 && xbuffer[2] == 6);

	std::vector<float> ybuffer;
	data.get_data_line_y(1, ybuffer);
	BOOST_CHECK(ybuffer.size()== 4);
	BOOST_CHECK(ybuffer[0] == 1 && ybuffer[2] == 5);



	data.put_data_line_x(0, xbuffer);
	BOOST_CHECK(data(0,0) == 4);
	BOOST_CHECK(data(2,0) == 6);

	data.put_data_line_y(2, ybuffer);
	BOOST_CHECK(data(2,0) == 1);
	BOOST_CHECK(data(2,3) == 7);


	C2DFDatafield data2(data);
	BOOST_CHECK(data2(2,0) == 1);
	BOOST_CHECK(data2(2,3) == 7);

	data2.clear();
	BOOST_CHECK(data(2,0) == 1);
	BOOST_CHECK(data(2,3) == 7);
	BOOST_CHECK(data2(2,0) == 0);
	BOOST_CHECK(data2(2,3) == 0);

	C2DFDatafield::data_array indata(values, values + 20);
	C2DFDatafield data3(C2DBounds(5,4), indata);

	BOOST_CHECK(data3.get_size()  == C2DBounds(5,4));
	BOOST_CHECK(data3.size() == 20);
	BOOST_CHECK(data3(0,0) == 0);
	BOOST_CHECK(data3(2,2) == 6);

}

