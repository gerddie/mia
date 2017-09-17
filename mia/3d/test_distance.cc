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

#include <mia/internal/autotest.hh>
#include <mia/core/distance.hh>
#include <mia/3d/distance.hh>

using namespace mia; 


struct Distance3DInfFixture {
	static const bool src_init[64]; 
	static const float test_val[64]; 
}; 

struct Distance3DFuncFixture {
	static const float src_init[64]; 
	static const float test_val[64]; 
}; 



 

BOOST_FIXTURE_TEST_CASE( test_distance_full3d_inf, Distance3DInfFixture ) 
{
	C3DFImage src_img(C3DBounds(4,4,4)); 
	
	distance_transform_prepare(&src_init[0], &src_init[64], src_img.begin(), true); 
	
	C3DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		BOOST_CHECK_CLOSE(*i, test_val[k], 0.1); 
	}
}


BOOST_FIXTURE_TEST_CASE( test_distance_per_slice3d_inf, Distance3DInfFixture ) 
{
	C3DDistance slice_based_distance; 
	
	C2DFImage slice(C2DBounds(4,4)); 
	
	for (int i = 0; i < 4; ++i) {
		distance_transform_prepare(&src_init[16 * i], &src_init[16 * (i+1)], slice.begin(), true);
		slice_based_distance.push_slice(i, slice); 
	}


	int k = 0; 
	for (int i = 0; i< 4; ++i) {
		auto out_slice = slice_based_distance.get_distance_slice(i);
		for(auto i = out_slice.begin(); i != out_slice.end(); ++i, ++k ) {
			cvdebug() << "k=" << k << ":" << *i << ", vs " << test_val[k] << "\n"; 
			BOOST_CHECK_CLOSE(*i, test_val[k], 0.1); 
		}
	}
}

BOOST_FIXTURE_TEST_CASE( test_distance_per_slice3d_direct, Distance3DInfFixture ) 
{
	C3DDistance slice_based_distance; 
	
	C2DFImage slice(C2DBounds(4,4)); 
	
	for (int i = 0; i < 4; ++i) {
		distance_transform_prepare(&src_init[16 * i], &src_init[16 * (i+1)], slice.begin(), true);
		slice_based_distance.push_slice(i, slice); 
	}

	int k = 0; 
	for (int z = 0; z < 4; ++z) 
		for (int y = 0; y < 4; ++y) 
			for (int x = 0; x < 4; ++x, ++k) {
				C3DFVector p(x,y,z); 
				
				auto d = slice_based_distance.get_distance_at(p); 
				cvdebug() << "p=" << p << ":" << d << ", vs " << sqrt(test_val[k]) << "\n"; 
				BOOST_CHECK_CLOSE(d, sqrt(test_val[k]), 0.1); 
				
				
			}

}


BOOST_FIXTURE_TEST_CASE( test_distance_per_slice3d_direct_3_3_3, Distance3DInfFixture ) 
{
	C3DDistance slice_based_distance; 
	
	C2DFImage slice(C2DBounds(4,4)); 
	
	for (int i = 0; i < 4; ++i) {
		distance_transform_prepare(&src_init[16 * i], &src_init[16 * (i+1)], slice.begin(), true);
		slice_based_distance.push_slice(i, slice); 
	}

	C3DFVector p(3,3,3); 
	auto d = slice_based_distance.get_distance_at(p); 
	cvdebug() << "p=" << p << ":" << d << ", vs " << sqrt(test_val[3*16 + 3 *4 + 3]) << "\n"; 
	BOOST_CHECK_CLOSE(d, sqrt(test_val[3*16 + 3 *4 + 3]), 0.1); 

}


BOOST_FIXTURE_TEST_CASE( test_distance_full3d_func,  Distance3DFuncFixture ) 
{
	C3DFImage src_img(C3DBounds(4,4,4)); 
	
	distance_transform_prepare(&src_init[0], &src_init[64], src_img.begin(), false); 
	
	C3DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		cvdebug() << "k=" << k << ":" << *i << ", vs " << test_val[k] << "\n"; 
		BOOST_CHECK_CLOSE(*i, test_val[k], 0.1); 
	}
}

BOOST_FIXTURE_TEST_CASE( test_distance_per_slice3d_func, Distance3DFuncFixture ) 
{
	C3DDistance slice_based_distance; 
	
	C2DFImage slice(C2DBounds(4,4)); 
	
	for (int i = 0; i < 4; ++i) {
		distance_transform_prepare(&src_init[16 * i], &src_init[16 * (i+1)], slice.begin(), false);
		slice_based_distance.push_slice(i, slice); 
	}


	int k = 0; 
	for (int i = 0; i< 4; ++i) {
		auto out_slice = slice_based_distance.get_distance_slice(i);
		for(auto i = out_slice.begin(); i != out_slice.end(); ++i, ++k ) {
			cvdebug() << "k=" << k << ":" << *i << ", vs " << test_val[k] << "\n"; 
			BOOST_CHECK_CLOSE(*i, test_val[k], 0.1); 
		}
	}
}

const bool Distance3DInfFixture::src_init[] = {
		1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0,    
		0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 1, 0,  0, 0, 0, 0, 
		0, 0, 0, 0,   0, 1, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,   0, 1, 0, 0,  0, 0, 0, 0
	}; 


const float Distance3DInfFixture::test_val[] = {
		0, 1, 4, 6,   1, 2, 2, 3,   4, 2, 1, 2,   6, 3, 2, 3,   
		1, 2, 3, 5,   2, 1, 1, 2,   3, 1, 0, 1,   5, 2, 1, 2,  
		2, 1, 2, 5,   1, 0, 1, 3,   2, 1, 1, 2,   3, 2, 2, 3,   
		3, 2, 3, 6,   2, 1, 2, 5,   1, 0, 1, 4,   2, 1, 2, 5
}; 

const float Distance3DFuncFixture::src_init[] = {
	1, 2, 4, 5,   5, 4, 3, 2,   3, 2, 2, 3,  4, 5, 3, 1, 
	1, 4, 5, 2,   1, 2, 3, 3,   2, 3, 1, 4,  4, 4, 3, 4, 
	3, 2, 1, 1,   2, 1, 4, 3,   0, 3, 2, 1,  5, 4, 4, 2,
	7, 8, 1, 2,   6, 5, 4, 3,   4, 1, 4, 2,  5, 5, 4, 3
}; 
	
const float Distance3DFuncFixture::test_val[] = {
	1, 2, 5, 5,   2, 3, 3, 4,   3, 3, 2, 2,  5, 4, 2, 1, 
	1, 2, 2, 2,   1, 2, 2, 3,   1, 2, 1, 2,  2, 3, 2, 2, 
	2, 2, 1, 1,   1, 1, 2, 2,   0, 1, 2, 1,  1, 2, 3, 2,
	4, 2, 1, 2,   2, 2, 2, 3,   1, 1, 2, 2,  2, 2, 3, 3
}; 
