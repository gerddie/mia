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




