/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/2d/filter/sepconv.hh>


NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
namespace bfs=::boost::filesystem;
using namespace SeparableConvolute_2dimage_filter;

struct  SepconvTextFixture {
	SepconvTextFixture(); 
	
	void run(const C2DFilter& f) const; 
	P2DImage image;
}; 

SepconvTextFixture::SepconvTextFixture()
{
	C2DFImage *src = new C2DFImage(C2DBounds(3,3));
	fill(src->begin(), src->end(), 0);
	(*src)(1,1) = 16.0f;
	image.reset(src); 
}

void SepconvTextFixture::run(const C2DFilter& f) const
{
	const float gauss_ref[9] = {
		1, 2, 1, /**/ 2, 4, 2, /**/ 1, 2, 1
	};
	P2DImage result = f.filter(*image);
	
	BOOST_CHECK_EQUAL(result->get_size(), image->get_size());
	C2DFImage *r = dynamic_cast<C2DFImage*>(&*result);

	BOOST_REQUIRE(r);

	int j = 0;
	for (C2DFImage::const_iterator i = r->begin(); i != r->end(); ++i, ++j) {
		cvdebug() << *i << " vs  " << gauss_ref[j] << "\n";
		BOOST_CHECK_CLOSE(*i, gauss_ref[j], 0.0001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_sepconv, SepconvTextFixture )
{
	auto filter = BOOST_TEST_create_from_plugin<C2DSeparableConvoluteFilterPlugin>("sepconv:kx=[gauss:w=1],ky=[gauss:w=1]"); 
	run(*filter); 
}

BOOST_FIXTURE_TEST_CASE( test_gauss, SepconvTextFixture )
{
	auto filter = BOOST_TEST_create_from_plugin<C2DGaussFilterPlugin>("gauss:w=1"); 
	run(*filter); 
}

BOOST_AUTO_TEST_CASE( test_sobel_x )
{
	auto sobel_x = BOOST_TEST_create_from_plugin<C2DSobelFilterPlugin>("sobel:dir=x"); 

	const float in_image[] = {
		1, 2, 3, 4,                 
		2, 3, 2, 5,
		6, 7, 8, 9,
		5, 4, 6, 3,
		6, 7, 8, 3
	};

	
	const float test_image[] = {
		0, 0.75,    1, 0, 
		0, 0.5,    1, 0, 
		0, 2.5/4,      0.625, 0,       
		0, 0.75, -0.5, 0,     
		0, 3.5/4, -6.5 / 4, 0        
	};

	C2DFImage src(C2DBounds(4,5), in_image);

	auto filtered = sobel_x->filter(src);

	const C2DFImage& f = dynamic_cast<const C2DFImage&>(*filtered);

	BOOST_CHECK_EQUAL(f.get_size(), src.get_size());

	const float *t = test_image; 
	for(auto i = f.begin(); i != f.end(); ++i, ++t) {
		cvdebug() << *i << " " << *t << "\n"; 
		BOOST_CHECK_CLOSE(*i, *t, 0.1);
	}
}

BOOST_AUTO_TEST_CASE( test_sobel_y )
{
	auto sobel_y = BOOST_TEST_create_from_plugin<C2DSobelFilterPlugin>("sobel:dir=y"); 

	const float in_image[] = {
		1, 2, 6, 5, 6,
		2, 3, 7, 4, 7,
		3, 2, 8, 6, 8,
		4, 5, 9, 3, 3
	};

	
	const float test_image[] = {
		0,  0,  0,  0,  0,
		0.75, 0.5, 2.5/4, 0.75, 3.5/4,
		1, 1, 0.625, -0.5, -6.5/4,
		0,  0,  0,  0,  0
	};

	C2DFImage src(C2DBounds(5,4), in_image);

	auto filtered = sobel_y->filter(src);

	const C2DFImage& f = dynamic_cast<const C2DFImage&>(*filtered);

	BOOST_CHECK_EQUAL(f.get_size(), src.get_size());

	const float *t = test_image; 
	for(auto i = f.begin(); i != f.end(); ++i, ++t) {
		cvdebug() << *i << " " << *t << "\n"; 
		BOOST_CHECK_CLOSE(*i, *t, 0.1);
	}
	
}


BOOST_AUTO_TEST_CASE( test_scharr_x )
{
	auto scharr_x = BOOST_TEST_create_from_plugin<C2DScharrFilterPlugin>("scharr:dir=x"); 

	const float in_image[] = {
		1, 2, 3, 4,                 
		2, 3, 2, 5,
		6, 7, 8, 9,
		5, 4, 6, 3,
		6, 7, 8, 3
	};

	
	const float test_image[] = {
		0, .625, 1, 0, 
		0, .375, 1, 0, 
		0, .71875, .71875, 0, 
		0, .6875, -0.5, 0, 
		0, .8125, -1.4375, 0
	};

	C2DFImage src(C2DBounds(4,5), in_image);

	auto filtered = scharr_x->filter(src);

	const C2DFImage& f = dynamic_cast<const C2DFImage&>(*filtered);

	BOOST_CHECK_EQUAL(f.get_size(), src.get_size());

	const float *t = test_image; 
	for(auto i = f.begin(); i != f.end(); ++i, ++t) {
		cvdebug() << *i << " " << *t << "\n"; 
		BOOST_CHECK_CLOSE(*i, *t, 0.1);
	}
}

BOOST_AUTO_TEST_CASE( test_scharr_y )
{
	auto scharr_y = BOOST_TEST_create_from_plugin<C2DScharrFilterPlugin>("scharr:dir=y"); 

	const float in_image[] = {
		1, 2, 6, 5, 6,
		2, 3, 7, 4, 7,
		3, 2, 8, 6, 8,
		4, 5, 9, 3, 3
	};

	
	const float test_image[] = {
		0,  0,  0,  0,  0,
		.625, 0.375, 0.71875, 0.6875, 0.8125,
		1, 1, 0.71875, -0.5, -1.4375,
		0,  0,  0,  0,  0
	};

	C2DFImage src(C2DBounds(5,4), in_image);

	auto filtered = scharr_y->filter(src);

	const C2DFImage& f = dynamic_cast<const C2DFImage&>(*filtered);

	BOOST_CHECK_EQUAL(f.get_size(), src.get_size());

	const float *t = test_image; 
	for(auto i = f.begin(); i != f.end(); ++i, ++t) {
		cvdebug() << *i << " " << *t << "\n"; 
		BOOST_CHECK_CLOSE(*i, *t, 0.1);
	}
	
}





