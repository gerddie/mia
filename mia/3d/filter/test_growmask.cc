/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#include <mia/3d/filter/growmask.hh>
namespace bfs=boost::filesystem;

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace growmask_3dimage_filter;


struct GrowMaskFixture: public FilterTestFixtureBase {

	GrowMaskFixture();
	P3DImage    ref;
	C3DBitImage mask;

};

const size_t nx = 5;
const size_t ny = 4;
const size_t nz = 3;

GrowMaskFixture::GrowMaskFixture():
	FilterTestFixtureBase(C3DBounds(nx,ny,nz)),
	mask(size)
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("shapes"));
	C3DShapePluginHandler::set_search_path(kernelsearchpath);


	unsigned short init_ref[nx * ny * nz] = {
		0, 1, 2, 3, 1,
		5, 6, 7, 1, 0,
		3, 4, 1, 1, 3,
		4, 5, 1, 2, 6,

		0, 2, 4, 5, 1,
		5, 6, 4, 0, 1,
		5, 3, 0, 1, 5,
		7, 8, 1, 3, 4,

		0, 2, 4, 1, 1,
		5, 6, 1, 0, 1,
		5, 3, 0, 1, 5,
		7, 1, 1, 1, 4
	};

	bool init_mask[nx * ny * nz] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,

		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,

		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};

	ref.reset(new C3DUSImage(size, init_ref));

	CDatapool::instance().add("reference.datapool", create_image3d_vector(ref));

	copy(init_mask, init_mask + nx * ny * nz, mask.begin());
}


BOOST_FIXTURE_TEST_CASE( test_grow_mask, GrowMaskFixture )
{
	bool result_mask[nx * ny * nz] = {
		0, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 0, 0,

		0, 1, 1, 0, 1,
		1, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 0, 0,

		0, 1, 1, 1, 1,
		1, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 1, 0
	};

	P3DShape shape(C3DShapePluginHandler::instance().produce("6n"));
	C3DGrowmask f(C3DImageDataKey("reference.datapool"), shape, 1);

	P3DImage result = f.filter(mask);

	check_mask_result(result, result_mask, size);
}


