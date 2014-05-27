/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/internal/autotest.hh>

#include <mia/3d/transform/rotbend.hh>

NS_MIA_USE


using namespace mia_3dtransform_rotbend;
using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

PrepareTestPluginPath plugin_path_init; 

struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C3DInterpolatorFactory ipf; 
}; 


struct Axis1Fixture : public ipfFixture {

        Axis1Fixture(); 

        void check_transformed_is_same(int idx, double param, const C3DFVector& v); 

        void check_transformed_is_expected(int idx, double param, const C3DFVector& v, const C3DFVector& expect); 

        C3DFVector m_origin; 
        C3DBounds m_size; 
        C3DRotBendTransformation m_transform; 
}; 


BOOST_FIXTURE_TEST_CASE( test_rotbend3d_rotation_is_fixed, Axis1Fixture )
{
        check_transformed_is_same(0, 0.0, m_origin); 
        check_transformed_is_same(0, 1.0, m_origin); 
        check_transformed_is_same(1, 1.0, m_origin); 
        check_transformed_is_same(2, 1.0, m_origin); 
        check_transformed_is_same(3, 1.0, m_origin); 
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d_x_rotation_origin_plus_minus_some_x_is_same, Axis1Fixture )
{
        check_transformed_is_same(0, 1.0, m_origin - C3DFVector(1,0,0));
        check_transformed_is_same(0, 1.0, m_origin + C3DFVector(1,0,0)); 
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d_y_rotation_origin_plus_minus_some_y_is_same, Axis1Fixture )
{
        check_transformed_is_same(1, 1.0, m_origin - C3DFVector(0, 1, 0));
        check_transformed_is_same(1, 1.0, m_origin + C3DFVector(0, 1, 0)); 
}


BOOST_FIXTURE_TEST_CASE( test_rotbend3d_shear_left_origin_plus_minus_y_is_same, Axis1Fixture )
{
        check_transformed_is_same(2, 1.0, m_origin - C3DFVector(0, 1, 0));
        check_transformed_is_same(2, 1.0, m_origin + C3DFVector(0, 1, 0)); 
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d_shear_right_origin_plus_minus_y_is_same, Axis1Fixture )
{
        check_transformed_is_same(3, 1.0, m_origin - C3DFVector(0, 1, 0));
        check_transformed_is_same(3, 1.0, m_origin + C3DFVector(0, 1, 0)); 
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d_rotation_y, Axis1Fixture )
{
	check_transformed_is_expected(0, M_PI/2.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2 ), 
				      m_origin + C3DFVector(-0.5, 0.2,  0.5));
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d_rotation_x, Axis1Fixture )
{
	check_transformed_is_expected(1, M_PI/2.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2 ), 
				      m_origin + C3DFVector(0.2, 0.5,  -0.5));
}


BOOST_FIXTURE_TEST_CASE( test_rotbend3d_bend_left, Axis1Fixture )
{
	check_transformed_is_expected(2, 2.0, m_origin +  C3DFVector(1, 2, -0.2), 
				      m_origin + C3DFVector(1, 2, -0.2 + 2.0/(13*13+1) ));
}

BOOST_FIXTURE_TEST_CASE( test_rotbend3d__bend_right, Axis1Fixture )
{
	check_transformed_is_expected(3, 3.0, m_origin +  C3DFVector(-3, 2, -0.2), 
				      m_origin + C3DFVector(-3, 2, -0.2 + 27.0 / (12*12+1)));
}

BOOST_FIXTURE_TEST_CASE( test_affine3d_iterator, ipfFixture )
{
	C3DBounds size(10,20,15);

	C3DRotBendTransformation t1(size, C3DFVector(5,9.0f,7.5f), ipf);
	C3DRotBendTransformation::const_iterator ti = t1.begin();
	
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}
	
	BOOST_CHECK(ti == t1.end());
}

BOOST_FIXTURE_TEST_CASE( test_affine3d_ranged_iterator, ipfFixture)
{
	C3DBounds size(10,20,30);
	C3DBounds delta(1,2,3); 

	C3DRotBendTransformation t1(size, C3DFVector(5,9.0f,15.5f), ipf);
	auto ti = t1.begin_range(delta, size - delta);

	for (size_t z = delta.z; z < size.z - delta.z; ++z)
		for (size_t y = delta.y; y < size.y - delta.y; ++y)
			for (size_t x = delta.x; x < size.x - delta.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}
	BOOST_CHECK(ti == t1.end_range(delta, size - delta));
}


Axis1Fixture::Axis1Fixture():
m_origin(12,23,32), 
        m_size(25, 47, 70), 
        m_transform(m_size, m_origin / C3DFVector(m_size), ipf)
{

}

void Axis1Fixture::check_transformed_is_same(int idx,  double param, const C3DFVector& v)
{
        check_transformed_is_expected(idx, param, v, v); 
}


void Axis1Fixture::check_transformed_is_expected(int idx, double param, 
						 const C3DFVector& v, const C3DFVector& expect)
{
        cvdebug() << "Check param[" << idx << "] = " << param << " and v = " << v << "\n";
	CDoubleVector p(4, true); 
        p[idx] = param; 
        m_transform.set_parameters(p); 

        auto vt = m_transform(v);
        
        BOOST_CHECK_CLOSE(vt.x, expect.x, 0.01); 
        BOOST_CHECK_CLOSE(vt.y, expect.y, 0.01); 
        BOOST_CHECK_CLOSE(vt.z, expect.z, 0.01); 
	
}

