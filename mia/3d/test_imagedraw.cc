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
#include <mia/3d/imageio.hh>
#include <set>

using namespace mia; 
using std::set; 

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


struct compare_coordinate  {
	bool operator () (const C3DBounds& lhs, const C3DBounds& rhs) {
		return (lhs.z < rhs.z) || 
			((lhs.z == rhs.z) && ((lhs.y < rhs.y) ||
					      ((lhs.y == rhs.y) && (lhs.x < rhs.x)))); 
	}
}; 

BOOST_FIXTURE_TEST_CASE( test_simple_draw_line_z_pivot, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(0,0,0), C3DFVector(10,11,12)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 

	pixels.insert(C3DBounds::_0); 
	pixels.insert(img.get_size() - C3DBounds::_1);
	
	C3DFVector dir(10.0 / 24, 11.0 / 24, 0.5f);
	
	C3DFVector p(0,0,0);
	for (int k = 0; k < 24; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}

BOOST_FIXTURE_TEST_CASE( test_simple_draw_line_pivot_x, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(0,4,5), C3DFVector(10,10,12)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.5, 0.3, 0.35f);
	
	C3DFVector p(0,4,5);
	for (int k = 0; k < 22; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}

BOOST_FIXTURE_TEST_CASE( test_simple_draw_line_pivot_y, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(4,0,5), C3DFVector(10,10,9)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.3, .5, 0.2f);
	
	C3DFVector p(4,0,5);
	for (int k = 0; k < 22; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}

BOOST_FIXTURE_TEST_CASE( test_draw_line_pivot_x_outside_ends, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(-4, 2, 5), C3DFVector(16, 14, 9)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.5, 0.3, 0.1f);
	
	C3DFVector p(-4,2,5);
	for (int k = 0; k < 42; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		if (ip.x < img.get_size().x) 
			pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}


BOOST_FIXTURE_TEST_CASE( test_draw_line_pivot_y_outside_ends, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(4,-6, 5), C3DFVector(10,14,9)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.15, 0.5, 0.1f);
	
	C3DFVector p(4,-6,5);
	for (int k = 0; k < 42; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		if (ip.y < img.get_size().y) 
			pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}

BOOST_FIXTURE_TEST_CASE( test_draw_line_pivot_z_outside_ends, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(4, 4, -5), C3DFVector(10, 14, 15)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.15, 0.25, 0.5f);
	
	C3DFVector p(5.5, 6.5, 0);
	for (int k = 0; k < 41 && p.z < 12; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		if (ip.z < img.get_size().z) 
			pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}


BOOST_FIXTURE_TEST_CASE( test_draw_line_pivot_x_outside_ends_dz_is_zero, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(-4, 2, 5), C3DFVector(16, 14, 5)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.5, 0.3, 0.0);
	
	C3DFVector p(-4,2,5);
	for (int k = 0; k < 42; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		if (ip.x < img.get_size().x) 
			pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}


BOOST_FIXTURE_TEST_CASE( test_draw_line_pivot_z_outside_ends_dy_is_zero, SimpleBitImageDrawFixture ) 
{
        output.draw_line(C3DFVector(4, 4, -5), C3DFVector(10, 4, 15)); 
        auto& img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	set<C3DBounds, compare_coordinate> pixels; 


	C3DFVector dir(0.15, 0.0, 0.5f);
	
	C3DFVector p(5.5, 4, 0);
	for (int k = 0; k < 41 && p.z < 12; ++k, p += dir) {
		C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
			     static_cast<unsigned>(floor(p.y + 0.5)), 
			     static_cast<unsigned>(floor(p.z + 0.5))); 
		cvdebug() << "test about to draw " << ip << " from " << p << "\n"; 
		if (ip.z < img.get_size().z) 
			pixels.insert(ip); 
	}
        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end())
                        BOOST_CHECK(!*i); 
                else 
                        BOOST_CHECK(*i);
        }
}


BOOST_FIXTURE_TEST_CASE( test_draw_line_triangle_inside, SimpleBitImageDrawFixture ) 
{
	C3DFVector a(4, 4, 4); 
	C3DFVector b(9, 10, 11); 
	C3DFVector c(5, 8, 1); 

        output.draw_triangle(a, b, c);
        auto img = output.get_image(); 
                
        auto i = img.begin_range(C3DBounds::_0, img.get_size()); 
        auto e = img.end_range(C3DBounds::_0, img.get_size()); 


	// visually inspecting the pixels shows that this is 
	// indeed the approximation of a triangle in 3D 
	
	set<C3DBounds, compare_coordinate> pixels; 
	pixels.insert(C3DBounds(9,10,11)); 
	

	pixels.insert(C3DBounds(8,9,10)); 
	pixels.insert(C3DBounds(8,10,10));
	pixels.insert(C3DBounds(9,10,10)); 	

	pixels.insert(C3DBounds(8,9,9)); 
	pixels.insert(C3DBounds(8,10,9)); 

	pixels.insert(C3DBounds(7,7,8));
	pixels.insert(C3DBounds(7,9,8));
	pixels.insert(C3DBounds(8,9,8)); 
	pixels.insert(C3DBounds(7,8,8)); 

	pixels.insert(C3DBounds(6,7,7)); 
	pixels.insert(C3DBounds(7,9,7)); 
	pixels.insert(C3DBounds(7,8,7)); 
	pixels.insert(C3DBounds(7,7,7));

	pixels.insert(C3DBounds(7,9,6)); 
	pixels.insert(C3DBounds(6,7,6)); 
	pixels.insert(C3DBounds(6,8,6)); 
	pixels.insert(C3DBounds(6,6,6)); 
	pixels.insert(C3DBounds(5,6,6));
	pixels.insert(C3DBounds(7,8,6));


	pixels.insert(C3DBounds(6,9,5));
	pixels.insert(C3DBounds(7,9,5)); 
	pixels.insert(C3DBounds(6,8,5)); 
	pixels.insert(C3DBounds(6,7,5)); 
	pixels.insert(C3DBounds(5,5,5)); 
	pixels.insert(C3DBounds(5,6,5)); 


	pixels.insert(C3DBounds(4,5,4));
	pixels.insert(C3DBounds(6,7,4));
	pixels.insert(C3DBounds(6,8,4)); 
	pixels.insert(C3DBounds(6,9,4)); 
	pixels.insert(C3DBounds(5,5,4)); 
	pixels.insert(C3DBounds(5,6,4)); 
	pixels.insert(C3DBounds(5,7,4)); 
	pixels.insert(C3DBounds(4,4,4)); 

	pixels.insert(C3DBounds(5,6,3));
	pixels.insert(C3DBounds(5,8,3));

	pixels.insert(C3DBounds(4,5,3)); 
	pixels.insert(C3DBounds(4,6,3)); 
	pixels.insert(C3DBounds(5,7,3)); 
	pixels.insert(C3DBounds(6,8,3)); 

	pixels.insert(C3DBounds(5,6,2));
	pixels.insert(C3DBounds(5,7,2)); 
	pixels.insert(C3DBounds(5,8,2)); 

	pixels.insert(C3DBounds(5,8,1)); 

        
	cvdebug() << "Expect " << pixels.size() << " pixels to be set\n"; 
        for(; i != e; ++i) {
		cvdebug() << i.pos() <<" = "  <<*i << "\n"; 
                if (pixels.find(i.pos()) == pixels.end()) {
                        BOOST_CHECK(!*i); 
                }else 
                        BOOST_CHECK(*i);
        }

}




SimpleBitImageDrawFixture::SimpleBitImageDrawFixture():
        output(C3DBounds(10,11,12), C3DFVector::_0, C3DFVector::_1)
{
}
