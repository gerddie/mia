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

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/3d/transform/raffine.hh>

NS_MIA_USE

using namespace mia_3dtransform_raffine;
using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

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
        C3DFVector m_axis; 
        C3DBounds m_size; 
        C3DRAffineTransformation m_transform; 
}; 


BOOST_FIXTURE_TEST_CASE( test_raffine3d_rotation_is_fixed, Axis1Fixture )
{
        check_transformed_is_same(0, 0.0, m_origin); 
        check_transformed_is_same(0, 1.0, m_origin); 
        check_transformed_is_same(1, 1.0, m_origin); 
        check_transformed_is_same(2, 1.0, m_origin); 
}

BOOST_FIXTURE_TEST_CASE( test_raffine3d_rotation_origin_plus_minus_axis_is_same, Axis1Fixture )
{
        check_transformed_is_same(0, 1.0, m_origin - m_axis);
        check_transformed_is_same(0, 1.0, m_origin + m_axis); 
}

#if 0 
BOOST_FIXTURE_TEST_CASE( test_raffine3d_scaley_origin_plus_minus_axis_is_same, Axis1Fixture )
{
        check_transformed_is_same(1, 1.0, m_origin - m_axis);
        check_transformed_is_same(1, 1.0, m_origin + m_axis); 
}

BOOST_FIXTURE_TEST_CASE( test_raffine3d_scalez_origin_plus_minus_axis_is_same, Axis1Fixture )
{
        check_transformed_is_same(2, 1.0, m_origin - m_axis);
        check_transformed_is_same(2, 1.0, m_origin + m_axis); 
}
#endif 

BOOST_FIXTURE_TEST_CASE( test_raffine3d_sheary_origin_plus_minus_axis_is_same, Axis1Fixture )
{
        check_transformed_is_same(1, 1.0, m_origin - m_axis);
        check_transformed_is_same(1, 1.0, m_origin + m_axis); 
}

BOOST_FIXTURE_TEST_CASE( test_raffine3d_shearz_origin_plus_minus_axis_is_same, Axis1Fixture )
{
        check_transformed_is_same(2, 1.0, m_origin - m_axis);
        check_transformed_is_same(2, 1.0, m_origin + m_axis); 
}


// the test values are evaluated with octave helper programs 
BOOST_FIXTURE_TEST_CASE( test_raffine3d_rotation, Axis1Fixture )
{
	check_transformed_is_expected(0, M_PI/2.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2 ), 
				      m_origin + C3DFVector(0.3454, 0.08835,  -0.6426));
}


#if 0 
// note that the scaling parameters are added to the matrix diagonal elements 
// in order to ensure that zero change is indicated by a zero parameter 
BOOST_FIXTURE_TEST_CASE( test_raffine3d_scalex, Axis1Fixture )
{
	check_transformed_is_expected(1, 1.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2 ), 
				      m_origin + C3DFVector(-1.041, 0.8729, -0.1131));
	
}

BOOST_FIXTURE_TEST_CASE( test_raffine3d_scalez, Axis1Fixture )
{
	check_transformed_is_expected(2, 1.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2 ), 
				      m_origin + C3DFVector(-0.4585, 0.6271, -0.4869));
	
}
#endif 

BOOST_FIXTURE_TEST_CASE( test_raffine3d_sheary, Axis1Fixture )
{
	check_transformed_is_expected(1, 1.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2), 
				      m_origin + C3DFVector( - 0.678, 0.2712, - 0.3271));
}

BOOST_FIXTURE_TEST_CASE( test_raffine3d_shearz, Axis1Fixture )
{
	check_transformed_is_expected(2, 1.0, m_origin +  C3DFVector(-0.5, 0.5, -0.2), 
				      m_origin + C3DFVector(-0.4131, 0.7663,  -0.8011));
}


BOOST_FIXTURE_TEST_CASE( test_affine3d_iterator, ipfFixture )
{
	C3DBounds size(10,20,15);

	C3DRAffineTransformation t1(size, C3DFVector(5,9.0f,7.5f), C3DFVector(0,1.0f,1.0f), ipf);
	C3DRAffineTransformation::const_iterator ti = t1.begin();
	
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

	C3DRAffineTransformation t1(size, C3DFVector(5,9.0f,15.5f), C3DFVector(0,1.0f,1.0f), ipf);
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
        m_axis(0.7, 0.9, 0.5), 
        m_size(25, 47, 70), 
        m_transform(m_size, m_origin / C3DFVector(m_size), m_axis, ipf)
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
	CDoubleVector p(3, true); 
        p[idx] = param; 
        m_transform.set_parameters(p); 

        auto vt = m_transform(v);
        
        BOOST_CHECK_CLOSE(vt.x, expect.x, 0.01); 
        BOOST_CHECK_CLOSE(vt.y, expect.y, 0.01); 
        BOOST_CHECK_CLOSE(vt.z, expect.z, 0.01); 
	
}

#if 0 
BOOST_FIXTURE_TEST_CASE (test_invers, ipfFixture)
{
	C3DBounds size(10,20, 30); 
	C3DRAffineTransformation trans(size, C3DFVector(5,9.0f,17.5f), ipf); 

	auto params = trans.get_parameters(); 
	BOOST_REQUIRE(params.size()== 3); 

	const float init_matrix[6] = {1,2,3,4, 2,3}; 
	const float inv_matrix[6]  = { 
		-4.0f/13.0f, -2.0f/13.0f,  7.0f/13.0f,   6.0f/13.0f, 
		 1.0f/13.0f,  7.0f/13.0f, -5.0f/13.0f,  -8.0f/13.0f, 
		 5.0f/13.0f, -4.0f/13.0f,  1.0f/13.0f, -14.0f/13.0f
	}; 
	copy(init_matrix, init_matrix+6, params.begin()); 

	
	trans.set_parameters(params); 
	
	unique_ptr<C3DTransformation> inverse( trans.invert()); 
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	params = inverse->get_parameters(); 
	BOOST_REQUIRE(params.size()== 6); 
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_CLOSE(params[i], inv_matrix[i], 0.1); 
	
}
#endif 
