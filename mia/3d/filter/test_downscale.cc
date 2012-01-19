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

#include <mia/internal/autotest.hh>
#include <mia/3d/filter/downscale.hh>
#include <mia/core/spacial_kernel.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace downscale_3dimage_filter;


BOOST_AUTO_TEST_CASE( test_downscale )
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("..")/bfs::path("core")/bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);

	list< bfs::path> filtersearchpath;
	filtersearchpath.push_back(bfs::path("."));
	C3DFilterPluginHandler::set_search_path(filtersearchpath);

	const short init[64] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
		4, 4, 5, 5, /**/ 4, 4, 5, 5, /**/ 6, 6, 7, 7, /**/ 6, 6, 7, 7,
		4, 4, 5, 5, /**/ 4, 4, 5, 5, /**/ 6, 6, 7, 7, /**/ 6, 6, 7, 7
	};

	// todo should test with a do-nothing filter
//	const short test[8] = {
//		0, 1, 2, 3, 4, 5, 6, 7
//	};

	const short test[8] = {
		1, 1, 2, 2, 4, 4, 5, 5
	};

	C3DSSImage fimage(C3DBounds(4, 4, 4), init );

	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 1.0));

	CDownscale scaler(C3DBounds(2, 2, 2), "gauss");

	P3DImage scaled =scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(), C3DBounds(2, 2, 2));

	const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage&>(*scaled);
	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.0f, 1.5f, 0.5f));
	const short *t = test;
	for( C3DSSImage::const_iterator k = fscaled.begin(); k != fscaled.end(); ++k, ++t ) {
		BOOST_CHECK_EQUAL(*k, *t);
	}

}
