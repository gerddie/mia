/* -*- mia-c++ -*- 
 *
 * Copyright (c) 2004-2011 
 *
 * Max-Planck-Institute of Evolutionary Anthropology, Leipzg 
 * ETSI Telecomunicacion, UPM, Madrid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



#include <mia/internal/autotest.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/stackdisttrans.hh>

NS_MIA_USE;  


BOOST_AUTO_TEST_CASE( test_isotropic_data )
{
	C2DBounds Size2D(3,3); 

	bool in_3d_1[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0};
	bool in_3d_2[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0}; 

	C2DBitImage mask1(Size2D, in_3d_1); 
	C2DBitImage mask2(Size2D, in_3d_2); 
	
	bool ref_3d_1[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1};
	bool ref_3d_2[9] = { 0, 1, 1, 1, 1, 1, 0, 1, 1}; 

	C2DBitImage ref1(Size2D, ref_3d_1); 
	C2DBitImage ref2(Size2D, ref_3d_2); 
	
	vector<C2DStackDistanceTransform::DistanceFromPoint> ref_result; 
	
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,0,0), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,0,0), 1)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,0,0), 2)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,1,0), 1)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,1,0), sqrt(2.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,1,0), sqrt(5.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,2,0), 2)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,2,0), sqrt(5.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,2,0), sqrt(8.0f))); 
	
	//ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,0,1), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,0,1), sqrt(2.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,0,1), sqrt(5.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,1,1), sqrt(2.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,1,1), sqrt(3.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,1,1), sqrt(6.0f))); 
	//ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,2,1), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,2,1), sqrt(6.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,2,1), 3)); 
	
	
	C2DStackDistanceTransform dt(mask1); 

	dt.read(mask2, 1); 
	dt.read(mask1, 2); 
	dt.read(mask2, 3); 


	auto result0 = dt.get_slice(0, ref1); 
	auto result1 = dt.get_slice(1, ref2); 

	
	result0.insert(result0.end(),result1.begin(), result1.end()); 
	
	
	BOOST_REQUIRE(result0.size() == ref_result.size()); 
	for( auto test = result0.begin(), ref = ref_result.begin(); test != result0.end(); 
	     ++test, ++ref) {
		BOOST_CHECK_EQUAL(*test, *ref); 
	}


}



BOOST_AUTO_TEST_CASE( test_anisotropic_data )
{
	C2DBounds Size2D(3,3); 
	C2DFVector pixel_size(2,3); 

	bool in_3d_1[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0};
	bool in_3d_2[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0}; 

	C2DBitImage mask1(Size2D, in_3d_1); 
	C2DBitImage mask2(Size2D, in_3d_2); 
	mask1.set_pixel_size(pixel_size); 
	mask2.set_pixel_size(pixel_size); 

	
	bool ref_3d_1[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1};
	bool ref_3d_2[9] = { 0, 1, 1, 1, 1, 1, 0, 1, 1}; 



	C2DBitImage ref1(Size2D, ref_3d_1); 
	C2DBitImage ref2(Size2D, ref_3d_2); 
	ref1.set_pixel_size(pixel_size); 
	ref2.set_pixel_size(pixel_size); 
	
	vector<C2DStackDistanceTransform::DistanceFromPoint> ref_result; 
	C3DFVector voxel_size(2.0, 3.0, 4.0); 
	
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,0,0), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,0,0), 2)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,0,0), 4)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,1,0), 3)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,1,0), sqrt(13.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,1,0), 5.0f)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,2,0), 6)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,2,0), sqrt(40.0))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,2,0), sqrt(52.0f))); 
	
	//ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,0,1), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,0,1), sqrt(20.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,0,1), sqrt(32.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,1,1), 5.0f)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,1,1), sqrt(29.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,1,1), sqrt(41.0f))); 
	//ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(0,2,1), 0)); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(1,2,1), sqrt(56.0f))); 
	ref_result.push_back(C2DStackDistanceTransform::DistanceFromPoint(C3DBounds(2,2,1), sqrt(68.0f))); 
			     
	
	C2DStackDistanceTransform dt(mask1, voxel_size); 

	dt.read(mask2, 1); 
	dt.read(mask1, 2); 
	dt.read(mask2, 3); 


	auto result0 = dt.get_slice(0, ref1); 
	auto result1 = dt.get_slice(1, ref2); 

	
	result0.insert(result0.end(),result1.begin(), result1.end()); 
	
	
	BOOST_REQUIRE(result0.size() == ref_result.size()); 
	for( auto test = result0.begin(), ref = ref_result.begin(); test != result0.end(); 
	     ++test, ++ref) {
		BOOST_CHECK_EQUAL(*test, *ref); 
	}
}



