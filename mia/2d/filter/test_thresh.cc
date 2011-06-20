/* -*- mia-c++  -*-
 *
 * Copyright (c) 2005-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <mia/internal/autotest.hh>
#include <mia/2d/filter/thresh.hh>
#include <mia/2d/shape.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace thresh_2dimage_filter;


struct ThreshFixture {

	ThreshFixture(); 
	void run_test(P2DShape neighborhood, const int *test_data); 

	static const int src[]; 
	static const int ref_4[]; 
	static const int thresh; 

	static const C2DBounds size;	
	P2DImage src_img; 

}; 

struct CTest4Shape: public C2DShape {
	CTest4Shape() {
		insert(C2DShape::Flat::value_type( 0, 0));
		insert(C2DShape::Flat::value_type( 1, 0));
		insert(C2DShape::Flat::value_type( 0, 1));
		insert(C2DShape::Flat::value_type(-1, 0));
		insert(C2DShape::Flat::value_type( 0,-1));
	}
};


const C2DBounds ThreshFixture::size = C2DBounds(7, 5); 
const int    ThreshFixture::thresh = 5;


const int ThreshFixture::src[] = 
	{ 0, 1, 2, 3, 2, 3, 5, 
	  2, 5, 2, 3, 5, 3, 2,
	  1, 2, 7, 6, 4, 2, 1,
	  3, 4, 4, 3, 4, 3, 2,
	  1, 3, 2, 4, 5, 6, 2}; 


const int ThreshFixture::ref_4[] =
	{ 0, 1, 0, 0, 2, 3, 5, 
	  2, 5, 2, 3, 5, 3, 2,
	  0, 2, 7, 6, 4, 0, 0,
	  0, 0, 4, 3, 4, 3, 0,
	  0, 0, 0, 4, 5, 6, 2}; 


ThreshFixture::ThreshFixture():
	src_img(new C2DSIImage(size, src))
{
}

void ThreshFixture::run_test(P2DShape neighborhood, const int *test_data)
{
	C2DThreshNImageFilter thresh_4(neighborhood, thresh); 
	P2DImage result = mia::filter(thresh_4, *src_img); 
	BOOST_REQUIRE(result); 
	BOOST_REQUIRE(result->get_size() == src_img->get_size()); 
	
	const C2DSIImage& rimg = dynamic_cast<const C2DSIImage&>(*result);
	
	for(auto ir = rimg.begin(); ir != rimg.end(); ++ir, ++test_data) 
		BOOST_CHECK_EQUAL(*ir, *test_data); 

}
	
BOOST_FIXTURE_TEST_CASE( test_thresh_n4, ThreshFixture) 
{
	run_test(P2DShape(new CTest4Shape), ref_4);  


	
}

