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
#include <mia/3d/filter/reorient.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace reorient_3dimage_filter;

struct ReorientMock {
	ReorientMock();
	void check_result(P3DImage result, const C3DUSImage& test_image,
			  const C3DFVector& test_voxel);

	C3DBounds size;
	C3DUSImage src;
};

ReorientMock::ReorientMock():
	size(2,3,5),
	src(size)
{
	C3DUSImage::iterator i = src.begin();
	for (size_t k = 0; k < src.size(); ++k, ++i)
		*i = k;

	src.set_voxel_size(C3DFVector(1.0, 2.0, 3.0));
}

void ReorientMock::check_result(P3DImage result, const C3DUSImage& test_image, const C3DFVector& test_voxel)
{
	const C3DUSImage *r = dynamic_cast<const C3DUSImage *>(result.get());
	BOOST_REQUIRE(r);
	BOOST_CHECK_EQUAL(r->get_size(), test_image.get_size());
	BOOST_REQUIRE(r->get_size() == test_image.get_size());
	for (C3DUSImage::const_iterator ir = r->begin(), it = test_image.begin();
	     ir != r->end(); ++ir, ++it){
		BOOST_CHECK_EQUAL(*ir, *it);
	}

	BOOST_CHECK_EQUAL(r->get_voxel_size(), test_voxel);
}


BOOST_FIXTURE_TEST_CASE( test_reorient_traversal_to_traversal, ReorientMock )
{
	C3DReorient f(C3DReorient::xyz);
	check_result(f.filter(src), src, src.get_voxel_size().xyz());
}

BOOST_FIXTURE_TEST_CASE( test_reorient_xzy, ReorientMock )
{
	C3DReorient f(C3DReorient::xzy);
	C3DBounds test_size(size.x, size.z, size.y);

	C3DUSImage  test_image(test_size, src);

	for (size_t z = 0; z < src.get_size().z; ++z)
		for (size_t y = 0; y < src.get_size().y; ++y)
			for (size_t x = 0; x < src.get_size().x; ++x) {
				test_image(x,z,y) = src(x,y,z);
			}

	check_result(f.filter(src), test_image, src.get_voxel_size().xzy());
}


BOOST_FIXTURE_TEST_CASE( test_reorient_yxz, ReorientMock )
{
	C3DReorient f(C3DReorient::yxz);
	C3DBounds test_size(size.y, size.x, size.z);

	C3DUSImage  test_image(test_size, src);

	for (size_t z = 0; z < src.get_size().z; ++z)
		for (size_t y = 0; y < src.get_size().y; ++y)
			for (size_t x = 0; x < src.get_size().x; ++x) {
				test_image(y,x,z) = src(x,y,z);
			}
	check_result(f.filter(src), test_image, test_image.get_voxel_size().yxz());
}


BOOST_FIXTURE_TEST_CASE( test_reorient_yzx, ReorientMock )
{
	C3DReorient f(C3DReorient::yzx);
	C3DBounds test_size(size.y, size.z, size.x);

	C3DUSImage  test_image(test_size, src);

	for (size_t z = 0; z < src.get_size().z; ++z)
		for (size_t y = 0; y < src.get_size().y; ++y)
			for (size_t x = 0; x < src.get_size().x; ++x) {
				test_image(y,z,x) = src(x,y,z);
			}
	check_result(f.filter(src), test_image, test_image.get_voxel_size().yzx());
}


BOOST_FIXTURE_TEST_CASE( test_reorient_zxy, ReorientMock )
{
	C3DReorient f(C3DReorient::zxy);
	C3DBounds test_size(size.z, size.x, size.y);

	C3DUSImage  test_image(test_size, src);

	for (size_t z = 0; z < src.get_size().z; ++z)
		for (size_t y = 0; y < src.get_size().y; ++y)
			for (size_t x = 0; x < src.get_size().x; ++x) {
				test_image(z,x,y) = src(x,y,z);
			}
	check_result(f.filter(src), test_image, test_image.get_voxel_size().zxy());
}

BOOST_FIXTURE_TEST_CASE( test_reorient_zyx, ReorientMock )
{
	C3DReorient f(C3DReorient::zyx);
	C3DBounds test_size(size.z, size.y, size.x);

	C3DUSImage  test_image(test_size, src);

	for (size_t z = 0; z < src.get_size().z; ++z)
		for (size_t y = 0; y < src.get_size().y; ++y)
			for (size_t x = 0; x < src.get_size().x; ++x) {
				test_image(z,y,x) = src(x,y,z);
			}
	check_result(f.filter(src), test_image, test_image.get_voxel_size().zyx());
}


struct ReorientFromAxialMock {
	ReorientFromAxialMock();
	void check_result(const C3DReorient& f, E3DImageOrientation new_orientation, const char *faces);

	C3DBounds size;
	C3DSBImage src;
};

ReorientFromAxialMock::ReorientFromAxialMock():
	size(3,3,3),
	src(size)
{
	fill(src.begin(), src.end(), '\0');
	src.set_orientation(ior_axial);
	src(0,1,1) = 'l';
	src(2,1,1) = 'r';

	src(1,0,1) = 'f';
	src(1,2,1) = 'b';

	src(1,1,0) = 'u';
	src(1,1,2) = 'd';

}

void ReorientFromAxialMock::check_result(const C3DReorient& f, E3DImageOrientation new_orientation, const char *faces)
{
	P3DImage result = f.filter(src);
	const C3DSBImage *r = dynamic_cast<const C3DSBImage *>(result.get());
	BOOST_REQUIRE(r);

	BOOST_CHECK_EQUAL(r->get_orientation(), new_orientation);

	BOOST_CHECK_EQUAL((*r)(0,1,1), faces[0]);
	BOOST_CHECK_EQUAL((*r)(2,1,1), faces[1]);
	BOOST_CHECK_EQUAL((*r)(1,0,1), faces[2]);
	BOOST_CHECK_EQUAL((*r)(1,2,1), faces[3]);
	BOOST_CHECK_EQUAL((*r)(1,1,0), faces[4]);
	BOOST_CHECK_EQUAL((*r)(1,1,2), faces[5]);

}



BOOST_FIXTURE_TEST_CASE( test_reorient_axial_to_axial, ReorientFromAxialMock )
{
	C3DReorient f(C3DReorient::axial);
	check_result(f, ior_axial, "lrfbud");
}

BOOST_FIXTURE_TEST_CASE( test_reorient_axial_to_saggital, ReorientFromAxialMock )
{
	C3DReorient f(C3DReorient::saggital);
	check_result(f, ior_saggital, "udfblr");
}
