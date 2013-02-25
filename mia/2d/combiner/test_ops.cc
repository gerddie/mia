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

#include <mia/internal/autotest.hh>
#include <mia/2d/combiner/ops.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace Combiner2d; 


const unsigned int size_x = 2; 
const unsigned int size_y = 3; 
const unsigned int length = size_x * size_y; 
const float src1_init[length]   = { 1,  2,  3,  4,  5,  6 }; 
const float src2_init[length]   = { 1,  4,  9, 16, 25, 36 };

const float test_add[length]    = { 2,  6, 12, 20, 30, 42 }; 
const float test_sub12[length]  = { 0, -2, -6,-12,-20,-30 }; 
const float test_sub21[length]  = { 0,  2,  6, 12, 20, 30 }; 
const float test_mult[length]   = { 1,  8, 27, 64,125,216 }; 
const float test_div21[length]  = { 1,  2,  3,  4,  5,  6 }; 

const float test_absdiff[length]  = { 0,  2,  6, 12, 20, 30 }; 


class CombinerOpsFixture {
protected: 
	CombinerOpsFixture(); 

	template <typename T> 
	void test_result(const C2DImage& image, const T *data) const;  

	C2DBounds size; 
	
	P2DImage src1; 
	P2DImage src2; 
}; 

template <typename T> 
void CombinerOpsFixture::test_result(const C2DImage& image, const T *data) const
{
	const T2DImage<T>& img = dynamic_cast<const T2DImage<T>&>(image); 
	BOOST_CHECK_EQUAL(img.get_size(), size); 
	
	for (auto i = img.begin(); i != img.end(); ++i, ++data) 
		BOOST_CHECK_CLOSE(*i, *data, 0.01); 
}

CombinerOpsFixture::CombinerOpsFixture():
	size(size_x,size_y)
{
	
	C2DFImage *fsrc1 = new C2DFImage(size); 
	src1.reset(fsrc1); 
	copy(src1_init, src1_init + length, fsrc1->begin()); 
	
	C2DFImage *fsrc2 = new C2DFImage(size); 
	src2.reset(fsrc2); 
	copy(src2_init, src2_init + length, fsrc2->begin()); 
}

BOOST_FIXTURE_TEST_CASE( test_add_combiner, CombinerOpsFixture )
{
	test_result<float>(*C2DAddImageCombiner().combine(*src1, *src2), test_add); 
}

BOOST_FIXTURE_TEST_CASE( test_sub_combiner, CombinerOpsFixture )
{
	test_result<float>(*C2DSubImageCombiner().combine(*src1, *src2), test_sub12); 
	test_result<float>(*C2DSubImageCombiner().combine(*src2, *src1), test_sub21); 
}

BOOST_FIXTURE_TEST_CASE( test_times_combiner, CombinerOpsFixture )
{
	test_result<float>(*C2DTimesImageCombiner().combine(*src1, *src2), test_mult); 
}

BOOST_FIXTURE_TEST_CASE( test_div_combiner, CombinerOpsFixture )
{
	test_result<float>(*C2DDivImageCombiner().combine(*src2, *src1), test_div21); 
}

BOOST_FIXTURE_TEST_CASE( test_absdiff_combiner, CombinerOpsFixture )
{
	test_result<float>(*C2DAbsDiffImageCombiner().combine(*src2, *src1), test_absdiff); 
	test_result<float>(*C2DAbsDiffImageCombiner().combine(*src1, *src2), test_absdiff); 
}


	
