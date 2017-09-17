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

#include <mia/internal/autotest.hh>
#include <mia/core/distance.hh>
#include <mia/2d/distance.hh>


NS_MIA_USE

using namespace std;

struct PointLineDistanceFixture {

	PointLineDistanceFixture();

	void check_point(const C2DFVector& point, float result);

	C2DFVector a;
	C2DFVector b;
};

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_end_point, PointLineDistanceFixture )
{
	check_point(a, 0.0f);
	check_point(b, 0.0f);
}


BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_line, PointLineDistanceFixture )
{
	check_point(C2DFVector(7, 10), 5.0f);
	check_point(C2DFVector(-2, -2), 5.0f);

}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_segment, PointLineDistanceFixture )
{
	check_point(C2DFVector(2.5, 4), 0.0f);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_a, PointLineDistanceFixture )
{
	check_point(C2DFVector(1, 0), 2);
	check_point(C2DFVector(1, 1), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_b, PointLineDistanceFixture )
{
	check_point(C2DFVector(4, 7), 1);
	check_point(C2DFVector(5, 6), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_normal, PointLineDistanceFixture )
{
	check_point(C2DFVector(1, 6), 2.4 );
}



PointLineDistanceFixture::PointLineDistanceFixture():
	a(1,2),
	b(4,6)
{
}

void PointLineDistanceFixture::check_point(const C2DFVector& point, float result)
{
	BOOST_CHECK_CLOSE(distance_point_line(point, a, b) + 1.0f, result + 1.0f, 0.1f);
}



BOOST_AUTO_TEST_CASE( test_distance_from_inf ) 
{
	bool in_2d[16] =  { 0, 0, 0, 1, 
			     0, 0, 0, 0, 
			     0, 0, 1, 0, 
			     0, 0, 0, 0
 }; 
	float out_2d[16] = { 8, 4, 1, 0, 
			     5, 2, 1, 1, 
			     4, 1, 0, 1, 
			     5, 2, 1, 2 }; 
	

	C2DFImage src_img(C2DBounds(4,4)); 
	
	distance_transform_prepare(&in_2d[0], &in_2d[16],src_img.begin(), true); 
	
	C2DFImage result =  distance_transform(src_img); 

	float *o = out_2d; 
	for(auto i = result.begin(); i != result.end(); ++i, ++o )
		BOOST_CHECK_CLOSE(*i, *o, 0.1); 
}


BOOST_AUTO_TEST_CASE( test_distance_from_func)
{
	float in_2d[16] =  { 4, 3, 1, 0, 
			     2, 2, 5, 7, 
			     4, 3, 0, 2, 
			     5, 6, 2, 1 }; 

	float out_2d[16] = { 5, 2, 1, 0, 
			     4, 2, 1, 1, 
			     4, 1, 0, 1, 
			     5, 2, 1, 1 }; 
	

	C2DFImage src_img(C2DBounds(4,4)); 

	distance_transform_prepare(&in_2d[0], &in_2d[16],src_img.begin(), false); 
	
	C2DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		BOOST_CHECK_CLOSE(*i, out_2d[k], 0.1); 
		cvdebug() << "k=" << k << ", " << *i << ", " << out_2d[k] << "\n"; 
		
	}
}
