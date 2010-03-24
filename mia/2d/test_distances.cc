/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/internal/autotest.hh>
#include <mia/2d/distances.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;

struct FixturePointLineDistance {

	FixturePointLineDistance();

	void check_point(const C2DFVector& point, float result);

	C2DFVector a;
	C2DFVector b;
};

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_end_point, FixturePointLineDistance )
{
	check_point(a, 0.0f);
	check_point(b, 0.0f);
}


BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_line, FixturePointLineDistance )
{
	check_point(C2DFVector(7, 10), 5.0f);
	check_point(C2DFVector(-2, -2), 5.0f);

}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_segment, FixturePointLineDistance )
{
	check_point(C2DFVector(2.5, 4), 0.0f);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_a, FixturePointLineDistance )
{
	check_point(C2DFVector(1, 0), 2);
	check_point(C2DFVector(1, 1), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_b, FixturePointLineDistance )
{
	check_point(C2DFVector(4, 7), 1);
	check_point(C2DFVector(5, 6), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_normal, FixturePointLineDistance )
{
	check_point(C2DFVector(1, 6), 2.4 );
}



FixturePointLineDistance::FixturePointLineDistance():
	a(1,2),
	b(4,6)
{
}

void FixturePointLineDistance::check_point(const C2DFVector& point, float result)
{
	BOOST_CHECK_CLOSE(distance_point_line(point, a, b) + 1.0f, result + 1.0f, 0.1f);
}

