/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/3d/shapes/basic_shapes.hh>
#include <mia/3d/shapes/sphere.hh>
using namespace mia; 
using namespace basic_3dshape_creator; 

struct CShapeTestFixture {
        void check(const C3DShape& shape, const C3DBounds& size, const bool *mask); 
        
}; 

void CShapeTestFixture::check(const C3DShape& shape, const C3DBounds& size, const bool *test_mask)
{
        auto mask = shape.get_mask();
        BOOST_CHECK_EQUAL(mask.get_size(), size);

        auto im = mask.begin();
        auto em = mask.end();

        const bool *tm = test_mask;
	int i = 0; 
        while (im != em) {
                BOOST_CHECK_EQUAL(*im, *tm);
		++im;
		++tm;
		++i; 
        }
        
}

BOOST_FIXTURE_TEST_CASE( test_6n_shape, CShapeTestFixture )
{
        auto shape = BOOST_TEST_create_from_plugin<C6n3DShapeFactory>("6n");

        const bool test_mask [] = {0,0,0, /**/  0,1,0, /**/ 0,0,0,
                                   0,1,0, /**/  1,1,1, /**/ 0,1,0,
                                   0,0,0, /**/  0,1,0, /**/ 0,0,0}; 
        
	check(*shape, C3DBounds(3,3,3), test_mask); 
       
}

BOOST_FIXTURE_TEST_CASE( test_18n_shape, CShapeTestFixture )
{
        auto shape = BOOST_TEST_create_from_plugin<C18n3DShapeFactory>("18n");

        const bool test_mask [] = {0,1,0, /**/  1,1,1, /**/ 0,1,0,
                                   1,1,1, /**/  1,1,1, /**/ 1,1,1,
                                   0,1,0, /**/  1,1,1, /**/ 0,1,0}; 
        
	check(*shape, C3DBounds(3,3,3), test_mask); 
}

BOOST_FIXTURE_TEST_CASE( test_27n_shape, CShapeTestFixture )
{
        auto shape = BOOST_TEST_create_from_plugin<C26n3DShapeFactory>("26n");

        const bool test_mask [] = {1,1,1, /**/  1,1,1, /**/ 1,1,1,
                                   1,1,1, /**/  1,1,1, /**/ 1,1,1,
                                   1,1,1, /**/  1,1,1, /**/ 1,1,1}; 
        
	check(*shape, C3DBounds(3,3,3), test_mask); 
}

BOOST_FIXTURE_TEST_CASE( test_sphere_2_shape, CShapeTestFixture )
{
        auto shape = BOOST_TEST_create_from_plugin<CSphere3DShapeFactory>("sphere:r=2");

        const bool test_mask [] = {0,0,0,0,0, /**/  0,0,0,0,0, /**/ 0,0,1,0,0, /**/ 0,0,0,0,0, /**/ 0,0,0,0,0,
				   0,0,0,0,0, /**/  0,1,1,1,0, /**/ 0,1,1,1,0, /**/ 0,1,1,1,0, /**/ 0,0,0,0,0,
				   0,0,1,0,0, /**/  0,1,1,1,0, /**/ 1,1,1,1,1, /**/ 0,1,1,1,0, /**/ 0,0,1,0,0,
				   0,0,0,0,0, /**/  0,1,1,1,0, /**/ 0,1,1,1,0, /**/ 0,1,1,1,0, /**/ 0,0,0,0,0,
				   0,0,0,0,0, /**/  0,0,0,0,0, /**/ 0,0,1,0,0, /**/ 0,0,0,0,0, /**/ 0,0,0,0,0}; 
        
	check(*shape, C3DBounds(5,5,5), test_mask); 
}




