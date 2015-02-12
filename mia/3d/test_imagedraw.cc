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
#include <mia/3d/imagedraw.hh>

using namespace mia; 

class SimpleBitImageDrawFixture {
        
protected: 
        SimpleBitImageDrawFixture(); 

        C3DBitImageDrawTarget output; 

}; 


BOOST_FIXTURE_TEST_CASE( test_simple_draw_point, SimpleBitImageDrawFixture ) 
{
        output.draw_point(C3DFVector(4,5,6)); 
        auto& img = output.get_image(); 
        
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 
        
        for(; i != e; ++i) {
                BOOST_CHECK_EQUAL(*i, i.pos() == C3DBounds(4,5,6)); 
        }
}

BOOST_FIXTURE_TEST_CASE( test_simple_draw_point_outside, SimpleBitImageDrawFixture ) 
{
        output.draw_point(C3DFVector(-1,5,6)); 
        output.draw_point(C3DFVector(1,-5,6)); 
        output.draw_point(C3DFVector(1,5,-6)); 

        output.draw_point(C3DFVector(10,5,6)); 
        output.draw_point(C3DFVector(1,11,6)); 
        output.draw_point(C3DFVector(1,5,12)); 



        auto& img = output.get_image(); 
        
        
        for(auto i : img) {
                BOOST_CHECK(!i);
        }
}

BOOST_FIXTURE_TEST_CASE( test_simple_draw_point_corners, SimpleBitImageDrawFixture ) 
{
        output.draw_point(C3DFVector(-.4,-.4,-.4)); 
        output.draw_point(C3DFVector(9.4,-.4,-.4)); 
        output.draw_point(C3DFVector(-.4,10.4,-.4)); 
        output.draw_point(C3DFVector(-.4,-.4, 11.4)); 

        output.draw_point(C3DFVector(9.4, 10.4, 11.4)); 
        output.draw_point(C3DFVector(-.4, 10.4, 11.4)); 
        output.draw_point(C3DFVector(9.4, -.4,  11.4)); 
        output.draw_point(C3DFVector(9.4, 10.4, -.4)); 

        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 
        
        for(; i != e; ++i) {
                if (i.pos() == C3DBounds::_0 ||
                    i.pos() == C3DBounds(9, 0, 0) || 
                    i.pos() == C3DBounds(0, 10, 0) ||
                    i.pos() == C3DBounds(0, 0, 11) ||
                    i.pos() == C3DBounds(9, 10, 0) ||
                    i.pos() == C3DBounds(9,  0,11) ||
                    i.pos() == C3DBounds(0, 10,11) ||
                    i.pos() == C3DBounds(9, 10,11))
                        BOOST_CHECK(*i); 
                else 
                        BOOST_CHECK(!*i);
        }
}



SimpleBitImageDrawFixture::SimpleBitImageDrawFixture():
        output(C3DBounds(10,11,12), C3DFVector::_0, C3DFVector::_1)
{
}
