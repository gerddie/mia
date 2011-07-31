/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#include <mia/3d/filter/filtertest.hh>
#include <mia/3d/filter/mask.hh>
#include <mia/core/datapool.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace mask_3dimage_filter;


const size_t nx = 3;
const size_t ny = 3;
const size_t nz = 2;
const size_t s = nx * ny * nz;


struct MaskFixture: public FilterTestFixtureBase {
	MaskFixture();
	P3DImage src;
	P3DImage mask;

};


MaskFixture::MaskFixture():
	FilterTestFixtureBase(C3DBounds(nx,ny,nz))
{
	const unsigned int init_src[s] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,
					   9, 10, 11, 12, 13, 14, 15, 16, 17 };
	const bool init_mask[s] = { 0, 0, 1, 1, 1, 0, 1, 0, 1,
				    0, 1, 1, 1, 0, 1, 0, 1, 0 };
	mask.reset(new C3DBitImage(size,init_mask ));
	src.reset(new C3DUIImage(size, init_src));



	CDatapool::instance().add("binary", create_image3d_vector(mask));
	CDatapool::instance().add("uint", create_image3d_vector(src));
}

BOOST_FIXTURE_TEST_CASE( test_mask ,MaskFixture )
{
	const unsigned int test_data[s] = { 0,  0,  2,  3,  4,  0,  6,  0,  8,
					    0, 10, 11, 12, 0, 14,  0, 16,  0 };
	C3DMask f(C3DImageDataKey("binary"));
	check_mask_result(f.filter(*src), test_data, size);
}


BOOST_FIXTURE_TEST_CASE( test_rmask ,MaskFixture )
{
	const unsigned int test_data[s] = { 0,  0,  2,  3,  4,  0,  6,  0,  8,
					    0, 10, 11, 12, 0, 14,  0, 16,  0 };
	C3DMask f(C3DImageDataKey("uint"));

	check_mask_result(f.filter(*mask), test_data, size);
}

