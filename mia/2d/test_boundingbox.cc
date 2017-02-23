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
#include <mia/2d/boundingbox.hh>

NS_MIA_USE;

struct BoundingboxFixture {
	void check(const C2DBoundingBox& value,
		   const C2DFVector& start,
		   const C2DFVector& end,
		   const C2DFVector& size,
		   const C2DIVector& grid_start,
		   const C2DIVector& grid_end,
		   const C2DBounds& grid_size);

	BoundingboxFixture();

	C2DFVector start1;
	C2DFVector end1;
	C2DBoundingBox bbox1;

};

BOOST_FIXTURE_TEST_CASE(test_boundingbox_empty, BoundingboxFixture )
{
	C2DBoundingBox bbox;
	BOOST_CHECK(bbox.empty());
}

BOOST_FIXTURE_TEST_CASE(test_boundingbox_basic, BoundingboxFixture )
{
	BOOST_REQUIRE(!bbox1.empty());
	check(bbox1, start1, end1, end1 - start1,
	      C2DIVector(10 ,-21), C2DIVector(21,21), C2DBounds(11, 42));
}

BOOST_FIXTURE_TEST_CASE(test_boundingbox_enlarge, BoundingboxFixture )
{
	bbox1.enlarge(2.5);
	check(bbox1, C2DFVector(8.0, -22.7),
	      C2DFVector(22.8,23.1), C2DFVector(14.8, 45.8),
	      C2DIVector(8 ,-23), C2DIVector(23,24), C2DBounds(15, 47));
}

BOOST_FIXTURE_TEST_CASE(test_boundingbox_all_internal, BoundingboxFixture )
{
	C2DFVector start2(11.1,-10.3);
	C2DFVector end2(14.5,14.5);
	C2DBoundingBox bbox2(start2,end2);

	bbox1.unite(bbox2);
	check(bbox1, start1, end1, end1 - start1,
	      C2DIVector(10 ,-21), C2DIVector(21,21), C2DBounds(11, 42));

}

BOOST_FIXTURE_TEST_CASE(test_boundingbox_extend_by_point, BoundingboxFixture )
{
	C2DFVector point(-2.3, 4.3);
	bbox1.add(point);
	check(bbox1, C2DFVector(point.x, start1.y), end1, end1 - C2DFVector(point.x, start1.y),
	      C2DIVector(-3 ,-21), C2DIVector(21,21), C2DBounds(24, 42));

}

BOOST_FIXTURE_TEST_CASE(test_boundingbox_entent_all, BoundingboxFixture )
{
	C2DFVector start2(5.2,-30.1);
	C2DFVector end2(30.3,45.6);
	C2DBoundingBox bbox2(start2,end2);

	bbox1.unite(bbox2);
	check(bbox1, start2, end2, end2 - start2,
	      C2DIVector(5 ,-31), C2DIVector(31,46), C2DBounds(26, 77));

}



BoundingboxFixture::BoundingboxFixture():
	start1(10.5,-20.2),
	end1(20.3,20.6),
	bbox1(start1, end1)
{
}

void BoundingboxFixture::check(const C2DBoundingBox& value,
			       const C2DFVector& start,
			       const C2DFVector& end,
			       const C2DFVector& size,
			       const C2DIVector& grid_start,
			       const C2DIVector& grid_end,
			       const C2DBounds& grid_size
			       )
{
	BOOST_CHECK_CLOSE(value.get_size().x, size.x, 0.1);
	BOOST_CHECK_CLOSE(value.get_size().y, size.y, 0.1);

	BOOST_CHECK_CLOSE(value.get_begin().x, start.x, 0.1);
	BOOST_CHECK_CLOSE(value.get_begin().y, start.y, 0.1);

	BOOST_CHECK_CLOSE(value.get_end().x, end.x, 0.1);
	BOOST_CHECK_CLOSE(value.get_end().y, end.y, 0.1);

	BOOST_CHECK_EQUAL(value.get_grid_size(), grid_size);
	BOOST_CHECK_EQUAL(value.get_grid_begin(), grid_start);
	BOOST_CHECK_EQUAL(value.get_grid_end(), grid_end);

}

