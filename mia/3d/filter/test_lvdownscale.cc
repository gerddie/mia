/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/3d/filter/lvdownscale.hh>

NS_MIA_USE
using namespace std;
using namespace lvdownscale_3dimage_filter;

class LVDownscaleFixture {
protected: 
	void check(const short *init, const short *expect, 
		   const C3DBounds& init_siize, const C3DBounds& block_size,  const C3DBounds& expect_size, 
		   const C3DFVector& init_voxel, const C3DFVector& expect_voxel); 

}; 



BOOST_FIXTURE_TEST_CASE( test_lvdownscale_all_zero,  LVDownscaleFixture )
{
	const short init[64] = {
		0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0,
		0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0,
		0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0,
		0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0
	};

	// todo should test with a do-nothing filter
	const short test[8] = {
		0, 0, 0, 0, 0, 0, 0, 0
	};

	check(init, test, C3DBounds(4, 4, 4), C3DBounds(2, 2, 2), C3DBounds(2, 2, 2), 
	      C3DFVector(2.0, 3.0, 1.0), C3DFVector(4.0f, 6.0f, 2.0f)); 
}

BOOST_FIXTURE_TEST_CASE( test_lvdownscale_result_all_one,  LVDownscaleFixture )
{
	const short init[64] = {
		1, 0, 0, 0, /**/ 0, 0, 0, 1, /**/ 0, 0, 0, 0, /**/ 0, 0, 1, 0,
		0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 1, 0, 0, /**/ 0, 0, 0, 0,
		0, 1, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 1, 0, /**/ 1, 0, 0, 0,
		0, 0, 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0
	};

	// todo should test with a do-nothing filter
	const short test[8] = {
		1, 1, 1, 1, 1, 1, 1, 1
	};

	check(init, test, C3DBounds(4, 4, 4), C3DBounds(2, 2, 2), C3DBounds(2, 2, 2), 
	      C3DFVector(2.0, 3.0, 1.0), C3DFVector(4.0f, 6.0f, 2.0f)); 
}

BOOST_FIXTURE_TEST_CASE( test_lvdownscale_result_some_real_voting,  LVDownscaleFixture )
{
	const short init[64] = {
		1, 2, 0, 0, /**/ 2, 2, 0, 1, /**/ 0, 0, 0, 0, /**/ 0, 0, 1, 0,
		1, 1, 0, 0, /**/ 2, 2, 0, 0, /**/ 0, 1, 0, 0, /**/ 0, 0, 0, 0,
		0, 1, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 2, 2, /**/ 1, 0, 1, 1,
		0, 0, 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 2, 2, /**/ 0, 0, 1, 1
	};

	// todo should test with a do-nothing filter
	const short test[8] = {
		2, 1, 1, 1, 1, 1, 1, 1
	};

	check(init, test, C3DBounds(4, 4, 4), C3DBounds(2, 2, 2), C3DBounds(2, 2, 2), 
	      C3DFVector(2.0, 3.0, 1.0), C3DFVector(4.0f, 6.0f, 2.0f)); 
}

BOOST_FIXTURE_TEST_CASE( test_lvdownscale_result_some_real_voting2,  LVDownscaleFixture )
{
	const short init[64] = {
		1, 2, 0, 0, /**/ 2, 2, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 1, 0,
		1, 1, 0, 0, /**/ 2, 2, 0, 0, /**/ 0, 1, 0, 0, /**/ 0, 0, 0, 0,
		0, 1, 0, 0, /**/ 0, 0, 3, 3, /**/ 0, 0, 2, 2, /**/ 1, 0, 1, 1,
		0, 0, 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 2, 2, /**/ 0, 0, 1, 1
	};

	// todo should test with a do-nothing filter
	const short test[8] = {
		2, 0, 1, 1, 1, 3, 1, 1
	};

	check(init, test, C3DBounds(4, 4, 4), C3DBounds(2, 2, 2), C3DBounds(2, 2, 2), 
	      C3DFVector(2.0, 3.0, 1.0), C3DFVector(4.0f, 6.0f, 2.0f)); 
}


void LVDownscaleFixture::check(const short *init, const short *expect, 
			       const C3DBounds& init_size, const C3DBounds& block_size,  const C3DBounds& expect_size, 
			       const C3DFVector& init_voxel, const C3DFVector& expect_voxel)
{

	C3DSSImage fimage(init_size, init );

	fimage.set_voxel_size(init_voxel);

	C3DLVDownscale scaler(block_size);

	P3DImage scaled = scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(), expect_size);

	const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage&>(*scaled);
	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), expect_voxel);
	const short *t = expect;
	for( C3DSSImage::const_iterator k = fscaled.begin(); k != fscaled.end(); ++k, ++t ) {
		BOOST_CHECK_EQUAL(*k, *t);
	}
}
