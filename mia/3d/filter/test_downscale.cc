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
#include <mia/3d/filter/downscale.hh>
#include <mia/core/spacial_kernel.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace downscale_3dimage_filter;


BOOST_AUTO_TEST_CASE( test_downscale )
{
	const short init[64] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
		4, 4, 5, 5, /**/ 4, 4, 5, 5, /**/ 6, 6, 7, 7, /**/ 6, 6, 7, 7,
		4, 4, 5, 5, /**/ 4, 4, 5, 5, /**/ 6, 6, 7, 7, /**/ 6, 6, 7, 7
	};

	// todo should test with a do-nothing filter
	const short test[8] = {
		1, 2, 3, 4, 4, 5, 6, 7
	};


	C3DSSImage fimage(C3DBounds(4, 4, 4), init );

	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 1.0));

	CDownscale scaler(C3DBounds(2, 2, 2), "gauss");

	P3DImage scaled =scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(), C3DBounds(2, 2, 2));

	const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage&>(*scaled);
	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.0f, 2.0f));
	const short *t = test;
	for( C3DSSImage::const_iterator k = fscaled.begin(); k != fscaled.end(); ++k, ++t ) {
		BOOST_CHECK_EQUAL(*k, *t);
	}

}

BOOST_AUTO_TEST_CASE( test_downscale_nonskew_x )
{
	for (unsigned int dx = 1; dx < 4; ++dx)  {
		C3DSSImage *fimage  = new C3DSSImage(C3DBounds(10+dx, 20, 20));
		auto i = fimage->begin(); 
		for (size_t z= 0; z < 20; ++z)
			for (size_t y = 0; y < 20; ++y)
				for (size_t x = 0; x < 10+dx; ++x, ++i) {
					*i = x; 
				}
		

		fimage->set_voxel_size(C3DFVector(2.0, 3.0, 1.0));

		CDownscale scaler(C3DBounds(2, 2, 2), "gauss");

		P3DImage scaled =scaler.filter(*fimage);

		size_t tsize = (10+dx+1)/2; 
		BOOST_CHECK_EQUAL(scaled->get_size(), C3DBounds(tsize, 10, 10));
		
		const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage&>(*scaled);
		BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.0f, 2.0f));


		for (size_t z= 3; z < 8; ++z)
			for (size_t y = 3; y < 8; ++y)
				for (size_t x = 0; x < tsize; ++x) {
					BOOST_CHECK_EQUAL(fscaled(x,y,z), fscaled(x,2,2)); 
				}
		

	}
}

BOOST_AUTO_TEST_CASE( test_downscale_nonskew_y )
{
	for (unsigned int dx = 1; dx < 4; ++dx)  {
		C3DSSImage *fimage  = new C3DSSImage(C3DBounds(20, 10+dx, 20));
		auto i = fimage->begin(); 
		for (size_t z= 0; z < 20; ++z)
			for (size_t y = 0; y < 10+dx; ++y)
				for (size_t x = 0; x < 20; ++x, ++i) {
					*i = y; 
				}
		

		fimage->set_voxel_size(C3DFVector(2.0, 3.0, 1.0));

		CDownscale scaler(C3DBounds(2, 2, 2), "gauss");

		P3DImage scaled =scaler.filter(*fimage);

		size_t tsize = (10+dx+1)/2; 
		BOOST_CHECK_EQUAL(scaled->get_size(), C3DBounds(10, tsize, 10));
		
		const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage&>(*scaled);
		BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.0f, 2.0f));


		for (size_t z= 3; z < 8; ++z)
			for (size_t y = 3; y < tsize; ++y)
				for (size_t x = 0; x < 8; ++x) {
					BOOST_CHECK_EQUAL(fscaled(x,y,z), fscaled(2,y,2)); 
				}
		

	}
}
