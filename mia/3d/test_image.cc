/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <climits>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/3d/image.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/boundary_conditions.hh>



NS_MIA_USE
using namespace boost;
using namespace std;

CSplineBoundaryConditionTestPath sbc_test_path; 

class CCopyFilter: public TFilter<std::shared_ptr<C3DImage>  > {
public:

	template <typename T>
	result_type operator () (const T3DImage<T>& image) const  {
		return result_type(new T3DImage<T>(image));
	}
};


template <typename T>
static void creat_and_check(EPixelType type)
{
	const T init_data[8] = {0,  1,  0,  1, 0,  1,  0,  1 };

	T3DImage<T> *timg = new T3DImage<T>(C3DBounds(2,2, 2), init_data);
	std::shared_ptr<C3DImage > image(timg);
	BOOST_CHECK(image->get_pixel_type() == type);

	CCopyFilter copy_filter;
	std::shared_ptr<C3DImage > result = filter(copy_filter, *image);

	BOOST_CHECK(result->get_pixel_type() == type);

	C3DFVector voxel(1.2f,2.3f,4.5f);
	image->set_voxel_size(voxel);
	BOOST_CHECK(voxel == image->get_voxel_size());

	image->set_orientation(ior_saggital);
	BOOST_CHECK_EQUAL(image->get_orientation(), ior_saggital);

	image->set_orientation(ior_axial);
	BOOST_CHECK_EQUAL(image->get_orientation(), ior_axial);


	T3DImage<T> image2(*timg);
	BOOST_CHECK(image2.get_size()  == C3DBounds(2,2, 2));

	BOOST_CHECK_EQUAL(image2.get_orientation(), ior_axial);

	BOOST_CHECK(voxel == image2.get_voxel_size());
	C3DFVector voxel2(3.2f,4.3f,1.5f);
	image2.set_voxel_size(voxel2);

	*timg = image2;
	BOOST_CHECK(voxel2 == timg->get_voxel_size());
}

static void check_data_types()
{
	creat_and_check<bool>(it_bit);
	creat_and_check<unsigned char>(it_ubyte);
	creat_and_check<signed char>(it_sbyte);
	creat_and_check<unsigned short>(it_ushort);
	creat_and_check<signed short>(it_sshort);
	creat_and_check<unsigned int>(it_uint);
	creat_and_check<signed int>(it_sint);
#ifdef LONG_64BIT
	creat_and_check<unsigned long>(it_ulong);
	creat_and_check<signed long>(it_slong);
#endif
	creat_and_check<float>(it_float);
	creat_and_check<double>(it_double);
}



static void check_gradient()
{
	const float init_data[64] = {
		0, 1, 2, 3, /**/ 4, 6, 7, 9, /**/ 4, 3, 2, 1, /**/ 0, 7, 8, 9,
		2, 3, 2, 2, /**/ 3, 3, 1, 2, /**/ 1, 8, 6, 5, /**/ 2, 3, 2, 6,
		4, 3, 2, 1, /**/ 2, 3, 7, 4, /**/ 3, 4, 6, 8, /**/ 3, 4, 5, 2,
		2, 5, 6, 3, /**/ 4, 1, 2, 3, /**/ 6, 4, 4, 1, /**/ 1, 5, 3, 3,
	};

	const C3DFVector vf[64] = {

		C3DFVector(0.0f, 0.0f, 0.0f), C3DFVector( 1.0f, 0.0f, 0.0f), C3DFVector( 1.0f, 0.0f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),
		C3DFVector(0.0f, 2.0f, 0.0f), C3DFVector( 1.5f, 1.0f, 0.0f), C3DFVector( 1.5f, 0.0f, 0.0f), C3DFVector(0.0f,-1.0f, 0.0f),
		C3DFVector(0.0f,-2.0f, 0.0f), C3DFVector(-1.0f, 0.5f, 0.0f), C3DFVector(-1.0f, 0.5f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),
		C3DFVector(0.0f, 0.0f, 0.0f), C3DFVector( 4.0f, 0.0f, 0.0f), C3DFVector( 1.0f, 0.0f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),

		C3DFVector(0.0f, 0.0f, 2.0f), C3DFVector( 0.0f, 0.0f, 1.0f), C3DFVector(-0.5f, 0.0f, 0.0f), C3DFVector(0.0f, 0.0f,-1.0f),
		C3DFVector(0.0f,-0.5f,-1.0f), C3DFVector(-1.0f, 2.5f,-1.5f), C3DFVector(-0.5f, 2.0f, 0.0f), C3DFVector(0.0f, 1.5f,-2.5f),
		C3DFVector(0.0f,-0.5f,-0.5f), C3DFVector( 2.5f, 0.0f, 0.5f), C3DFVector(-1.5f, 0.5f, 2.0f), C3DFVector(0.0f, 2.0f, 3.5f),
		C3DFVector(0.0f, 0.0f, 1.5f), C3DFVector( 0.0f, 0.0f,-1.5f), C3DFVector( 1.5f, 0.0f,-1.5f), C3DFVector(0.0f, 0.0f,-3.5f),
		
		C3DFVector(0.0f, 0.0f, 0.0f), C3DFVector(-1.0f, 0.0f, 1.0f), C3DFVector(-1.0f, 0.0f, 2.0f), C3DFVector(0.0f, 0.0f, 0.5f),
		C3DFVector(0.0f,-0.5f, 0.5f), C3DFVector( 2.5f, 0.5f,-1.0f), C3DFVector( 0.5f, 2.0f, 0.5f), C3DFVector(0.0f, 3.5f, 0.5f),
		C3DFVector(0.0f, 0.5f, 2.5f), C3DFVector( 1.5f, 0.5f,-2.0f), C3DFVector( 2.0f,-1.0f,-1.0f), C3DFVector(0.0f,-1.0f,-2.0f),
		C3DFVector(0.0f, 0.0f,-0.5f), C3DFVector( 1.0f, 0.0f, 1.0f), C3DFVector(-1.0f, 0.0f, 0.5f), C3DFVector(0.0f, 0.0f,-1.5f),

		C3DFVector(0.0f, 0.0f, 0.0f), C3DFVector( 2.0f, 0.0f, 0.0f), C3DFVector(-1.0f, 0.0f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),
		C3DFVector(0.0f, 2.0f, 0.0f), C3DFVector(-1.0f,-0.5f, 0.0f), C3DFVector( 1.0f,-1.0f, 0.0f), C3DFVector(0.0f,-1.0f, 0.0f),
		C3DFVector(0.0f,-1.5f, 0.0f), C3DFVector(-1.0f, 2.0f, 0.0f), C3DFVector(-1.5f, 0.5f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),
		C3DFVector(0.0f, 0.0f, 0.0f), C3DFVector( 1.0f, 0.0f, 0.0f), C3DFVector(-1.0f, 0.0f, 0.0f), C3DFVector(0.0f, 0.0f, 0.0f),

	};

	C3DBounds size(4,4,4);
	C3DFImage image(size, init_data);

	C3DFVectorfield gradient = get_gradient(image);

	auto ig = gradient.begin_range(C3DBounds::_0, image.get_size()); 
	auto eg = gradient.end_range(C3DBounds::_0, image.get_size()); 
	const C3DFVector * v = vf; 
	while (ig != eg) {
		BOOST_CHECK_EQUAL(*ig, *v);
		cvdebug() << ig.pos() << ": got " << *ig << " expect:" << *v << "\n"; 
		++ig; 
		++v; 
	}
	
	C3DFVector g1= image.data().get_gradient<float>(C3DFVector(1.5, 1.5, 1.5));
	float gx1 =
		((vf[21].x * 0.5 + vf[22].x * 0.5) * 0.5 +
		 (vf[25].x * 0.5 + vf[26].x * 0.5) * 0.5) * 0.5 +
		((vf[37].x * 0.5 + vf[38].x * 0.5) * 0.5 +
		 (vf[41].x * 0.5 + vf[42].x * 0.5) * 0.5) * 0.5;
	cvdebug() << gx1  << " vs. " <<  g1 << "\n";

	BOOST_CHECK_CLOSE(gx1, g1.x, 0.0001);

	C3DFVector g = image.data().get_gradient<float>(C3DFVector(1.2, 1.7, 1.4));

	float gx =
		((vf[21].x * 0.8f + vf[22].x * 0.2f) * 0.3f +
		 (vf[25].x * 0.8f + vf[26].x * 0.2f) * 0.7f) * 0.6f +
		((vf[37].x * 0.8f + vf[38].x * 0.2f) * 0.3f +
		 (vf[41].x * 0.8f + vf[42].x * 0.2f) * 0.7f) * 0.4f;
	cvdebug() << gx  << " vs. " <<  g << "\n";
	BOOST_CHECK_CLOSE(gx, g.x, 0.0001);

}

void add_3dimage_tests( boost::unit_test::test_suite* suite)
{
	suite->add( BOOST_TEST_CASE(&check_data_types));
	suite->add( BOOST_TEST_CASE(&check_gradient));
}


BOOST_AUTO_TEST_CASE( test_rotation_attribute )
{
	C3DRotationAttributeTranslate::register_for("rotation3d");
	string value("rot-quaternion=0.5,0.1,0.5,0.7");

	PAttribute attr = CStringAttrTranslatorMap::instance().to_attr("rotation3d", value);
	
	const C3DRotationAttribute& ra = dynamic_cast<C3DRotationAttribute&>(*attr);
	C3DRotation r = ra; 
	auto q = r.as_quaternion(); 

	BOOST_CHECK_EQUAL(q.w(), 0.5);
	BOOST_CHECK_EQUAL(q.x(), 0.1);
	BOOST_CHECK_EQUAL(q.y(), 0.5);
	BOOST_CHECK_EQUAL(q.z(), 0.7);
}

BOOST_AUTO_TEST_CASE( test_voxel_attribute )
{
	string value("1 2 3");

	CVoxelAttributeTranslator::register_for("voxel");
	PAttribute attr = CStringAttrTranslatorMap::instance().to_attr("voxel", value);

	CVoxelAttribute *va = dynamic_cast<CVoxelAttribute*>(attr.get());
	BOOST_REQUIRE(va);

	C3DFVector vv = *va;
	BOOST_CHECK_EQUAL(vv.x, 1.0);
	BOOST_CHECK_EQUAL(vv.y, 2.0);
	BOOST_CHECK_EQUAL(vv.z, 3.0);

	BOOST_CHECK_EQUAL(attr->as_string(), value);
}

BOOST_AUTO_TEST_CASE( test_3dint_attribute )
{
	string value("1 2 3");

	C3DIntAttributeTranslator::register_for("3dint");
	PAttribute attr = CStringAttrTranslatorMap::instance().to_attr("3dint", value);

	C3DIntAttribute *va = dynamic_cast<C3DIntAttribute*>(attr.get());
	BOOST_REQUIRE(va);

	T3DVector<int> vv = *va;
	BOOST_CHECK_EQUAL(vv.x, 1);
	BOOST_CHECK_EQUAL(vv.y, 2);
	BOOST_CHECK_EQUAL(vv.z, 3);

	BOOST_CHECK_EQUAL(attr->as_string(), value);
}
