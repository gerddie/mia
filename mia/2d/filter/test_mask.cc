/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/template/filtertest.hh>
#include <mia/2d/filter/mask.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace mask_2dimage_filter;

BOOST_AUTO_TEST_CASE(test_mask_2d_additional_is_mask)
{


	const int src[15] =   {   0,    1,     2,     3,    4,     5,     6,    7,    8,    9,   10,    11,    12,   13,   14};
	const bool mask[15] = {true, true, false, false, true, false, false, true, true, true, true, false, false, true, false};
	const int ref[15] =   {   0,    1,     0,     0,    4,     0,     0,    7,    8,    9,   10,     0,     0,   13,    0};
//	const int iref[15] =  {   0,    0,     2,     3,    0,     5,     6,    0,    0,    0,    0,    11,    12,    0,    1};

	C2DBounds size(3,5);

	C2DSIImage *src_img = new C2DSIImage(size);
	P2DImage src_wrap(src_img);
	copy(&src[0], &src[15], src_img->begin());

	C2DBitImage *mask_img = new C2DBitImage(size);
	copy(&mask[0], &mask[15], mask_img->begin());
	P2DImage pmask(mask_img); 

	C2DMask mask_f(C2DImageDataKey("mask.@"), C2DMask::f_zero, false);
	save_image("mask.@", pmask); 

	P2DImage res_wrap = mask_f.filter(*src_wrap);
	const C2DSIImage *res_img = dynamic_cast<const C2DSIImage *>(res_wrap.get());
	BOOST_REQUIRE(res_img);


	BOOST_REQUIRE(res_img->get_size()== size);

	BOOST_CHECK(equal(res_img->begin(), res_img->end(), &ref[0]));
}

BOOST_AUTO_TEST_CASE(test_mask_2d_main_is_mask)
{


	const int src[15] =   {   0,    1,     2,     3,    4,     5,     6,    7,    8,    9,   10,    11,    12,   13,   14};
	const bool mask[15] = {true, true, false, false, true, false, false, true, true, true, true, false, false, true, false};
	const int ref[15] =   {   0,    1,     0,     0,    4,     0,     0,    7,    8,    9,   10,     0,     0,   13,    0};
//	const int iref[15] =  {   0,    0,     2,     3,    0,     5,     6,    0,    0,    0,    0,    11,    12,    0,    1};

	C2DBounds size(3,5);

	C2DSIImage *src_img = new C2DSIImage(size);
	P2DImage src_wrap(src_img);
	copy(&src[0], &src[15], src_img->begin());

	C2DBitImage *mask_img = new C2DBitImage(size);
	copy(&mask[0], &mask[15], mask_img->begin());
	P2DImage pmask(mask_img); 

	C2DMask mask_f(C2DImageDataKey("orig.@"), C2DMask::f_zero, false);
	save_image("orig.@", src_wrap); 

	P2DImage res_wrap = mask_f.filter(*pmask);
	const C2DSIImage *res_img = dynamic_cast<const C2DSIImage *>(res_wrap.get());
	BOOST_REQUIRE(res_img);


	BOOST_REQUIRE(res_img->get_size()== size);

	BOOST_CHECK(equal(res_img->begin(), res_img->end(), &ref[0]));
}



BOOST_FIXTURE_TEST_CASE(test_mask_2d_additional_is_mask_inverse, TFiltertestFixture<T2DImage>)
{
	const int src[15] =   {   0,    1,     2,     3,    4,     5,     6,    7,    8,    9,   10,    11,    12,   13,   14};
	const bool init_mask[15] = {true, true, false, false, true, false, false, true, true, true, true, false, false, true, false};
	const int ref[15] =  {   0,    0,     2,     3,    0,     5,     6,    0,    0,    0,    0,    11,    12,    0,    14};

	C2DBounds size(3,5);


	C2DBitImage mask(size, init_mask); 
	P2DImage pmask(&mask, void_destructor<C2DImage>()); 
	
	save_image("mask.@", pmask);  
	

	auto filter = BOOST_TEST_create_from_plugin<C2DMaskImageFilterFactory>("mask:input=mask.@,fill=zero,inverse=1"); 
	
	run(size, src, size, ref, *filter); 

}

BOOST_FIXTURE_TEST_CASE(test_mask_2d_main_is_mask_inverse, TFiltertestFixture<T2DImage>)
{
	const int init_src[15] =   {   0,    1,     2,     3,    4,     5,     6,    7,    8,    9,   10,    11,    12,   13,   14};
	const bool init_mask[15] = {true, true, false, false, true, false, false, true, true, true, true, false, false, true, false};
	const int ref[15] =  {   0,    0,     2,     3,    0,     5,     6,    0,    0,    0,    0,    11,    12,    0,    14};

	C2DBounds size(3,5);


	C2DSIImage data(size, init_src); 
	P2DImage pdata(&data, void_destructor<C2DImage>()); 
	
	save_image("data.@", pdata);  
	
	auto filter = BOOST_TEST_create_from_plugin<C2DMaskImageFilterFactory>("mask:input=data.@,fill=zero,inverse=1"); 
	
	run(size, init_mask, size, ref, *filter); 
}

BOOST_AUTO_TEST_CASE(test_mask_2d_no_binary)
{

	const int src[15] =   {   0,    1,     2,     3,    4,     5,     6,    7,    8,    9,   10,    11,    12,   13,   14};

	C2DBounds size(3,5);

	C2DSIImage *src_img = new C2DSIImage(size);
	P2DImage src_wrap(src_img);
	copy(&src[0], &src[15], src_img->begin());

	auto f = BOOST_TEST_create_from_plugin<C2DMaskImageFilterFactory>("mask:input=orig.@");
	save_image("orig.@", src_wrap); 

	BOOST_CHECK_THROW(f->filter(*src_wrap), invalid_argument); 
	
}
