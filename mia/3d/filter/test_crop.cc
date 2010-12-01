/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Evolutionary Anthropoloy
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


#include <mia/core/shared_ptr.hh>
#include <mia/internal/autotest.hh>
#include <mia/3d/filter/crop.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace crop_3d_filter;

struct CropFixture {
	CropFixture();

	C3DBounds size;
	C3DSIImage src;

	void check_result(C3DCrop& f, const C3DBounds& start, C3DBounds rsize);

};

CropFixture::CropFixture():
	size(10,12,13),
	src(size)
{
	C3DSIImage::iterator i = src.begin();
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				*i = (x+1 ) * (y +1) * (z+1);
			}
}

void CropFixture::check_result(C3DCrop& f, const C3DBounds& start, C3DBounds rsize)
{
	P3DImage result_wrap = f.filter(src);
	const C3DSIImage *result = dynamic_cast<const C3DSIImage *>(result_wrap.get());
	BOOST_REQUIRE(result);
	BOOST_REQUIRE(result->get_size() == rsize);

	C3DSIImage::const_iterator i = result->begin();
	for (size_t z = 0; z < rsize.z; ++z)
		for (size_t y = 0; y < rsize.y; ++y)
			for (size_t x = 0; x < rsize.x; ++x, ++i) {
				BOOST_CHECK_EQUAL(static_cast<size_t>(*i), 
						  (x+1+start.x ) * (y +1+start.y) * (z+1+start.z));
			}
}

BOOST_FIXTURE_TEST_CASE( test_crop_inside, CropFixture )
{
	C3DBounds start(2,3,4);
	C3DBounds end(10,9,11);

	C3DCrop f(start, end);
	check_result(f, start, end-start);
}

BOOST_FIXTURE_TEST_CASE( test_crop_x_outside, CropFixture )
{
	C3DBounds start(10,3,4);
	C3DBounds end(10,9,11);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE( test_crop_y_outside, CropFixture )
{
	C3DBounds start(3,20,4);
	C3DBounds end(10,9,11);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE( test_crop_z_outside, CropFixture )
{
	C3DBounds start(3,4,40);
	C3DBounds end(10,9,11);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE( test_crop_limit_x, CropFixture )
{
	C3DBounds start(2,3,4);
	C3DBounds end(-1,9,11);

	C3DCrop f(start, end);
	check_result(f, start, C3DBounds(size.x-start.x,end.y-start.y,end.z-start.z));
}

BOOST_FIXTURE_TEST_CASE( test_crop_limit_y, CropFixture )
{
	C3DBounds start(2,3,4);
	C3DBounds end(10,-1,11);

	C3DCrop f(start, end);
	check_result(f, start, C3DBounds(end.x-start.x,size.y-start.y,end.z-start.z));
}

BOOST_FIXTURE_TEST_CASE( test_crop_limit_z, CropFixture )
{
	C3DBounds start(2,3,4);
	C3DBounds end(10,9,-1);

	C3DCrop f(start, end);
	check_result(f, start, C3DBounds(end.x-start.x,end.y-start.y,size.z-start.z));
}

BOOST_FIXTURE_TEST_CASE( test_crop_range_error_x, CropFixture )
{
	C3DBounds start(6,3,4);
	C3DBounds end(5,9,11);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE( test_crop_range_error_y, CropFixture )
{
	C3DBounds start(6,7,4);
	C3DBounds end(8,2,11);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE( test_crop_range_error_z, CropFixture )
{
	C3DBounds start(6,2,4);
	C3DBounds end(8,7,1);

	C3DCrop f(start, end);
	BOOST_CHECK_THROW(f.filter(src), std::invalid_argument);
}
