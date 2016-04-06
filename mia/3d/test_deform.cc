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

#include <cmath>
#include <memory>
#include <mia/internal/autotest.hh>
#include <mia/3d/deformer.hh>


NS_MIA_USE

using namespace std;

struct DeformFixture {
	DeformFixture();
	void check(EInterpolation ip);

	C3DFVector voxel;
	C3DBounds size;
	C3DFVectorfield transform;
	C3DFImage image;
};

DeformFixture::DeformFixture():
	voxel(1,2,3),
	size(8,7,9),
	transform(size),
	image(size)
{
	image(4,3,5) = 1;
	transform(4,4,5) = C3DFVector(0,1,0);
	transform(5,3,5) = C3DFVector(1,0,0);
	transform(4,3,6) = C3DFVector(0,0,1);
	image.set_voxel_size(voxel);
}

void DeformFixture::check(EInterpolation ip)
{
	unique_ptr<C3DInterpolatorFactory> ipf(create_3dinterpolation_factory(ip,bc_mirror_on_bounds));
	FDeformer3D d(transform, *ipf);
	C3DImage& img = image;
	P3DImage result = mia::filter(d, img);
	const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result);
	BOOST_REQUIRE(r.get_size() == size);
	BOOST_CHECK_EQUAL(r(4,4,5), 1);
	BOOST_CHECK_EQUAL(r(5,3,5), 1);
	BOOST_CHECK_EQUAL(r(4,3,6), 1);

	BOOST_CHECK_EQUAL(r.get_voxel_size(), voxel);
}

BOOST_FIXTURE_TEST_CASE( test_deform_nn, DeformFixture )
{
	check(ip_nn);
}


BOOST_FIXTURE_TEST_CASE( test_deform_bspline3, DeformFixture )
{
	check(ip_bspline3);
}
