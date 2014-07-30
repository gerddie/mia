/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/2d/filter/noise.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace noise_2dimage_filter;


struct CTestNoiseGenerator: public  CNoiseGenerator {
	CTestNoiseGenerator () :CNoiseGenerator(1){}

	double get() const {
		return 2.0;
	}
};

struct NoiseFixture {

	NoiseFixture():
		size(2,2),
		src_image(size)
	{
		src_image(0,0) = 0;
		src_image(0,1) = 127;
		src_image(1,0) = 128;
		src_image(1,1) = 255;
	}

	void check (const C2DImage *result, unsigned char *test_data) const {
		const C2DUBImage *r = dynamic_cast<const C2DUBImage *>(result);
		BOOST_REQUIRE(r);
		BOOST_CHECK_EQUAL(r->get_size(), size);
		BOOST_REQUIRE(r->get_size() == size);
		for (C2DUBImage::const_iterator i = r->begin(); i != r->end(); ++i, ++test_data) {
			BOOST_CHECK_EQUAL(*i, *test_data);
		}
	}

	C2DBounds size;
	C2DUBImage src_image;

};



BOOST_FIXTURE_TEST_CASE( test_noise_add, NoiseFixture )
{
	unsigned char test_result[4] = { 2, 130, 129, 255 };

	PNoiseGenerator  generator(new CTestNoiseGenerator());

	C2DNoise f(generator, false);

	P2DImage result = f.filter(src_image);

	check(result.get(), test_result);


}

BOOST_FIXTURE_TEST_CASE( test_noise_modulate, NoiseFixture )
{
	unsigned char test_result[4] = { 0, 255, 254, 255 };

	PNoiseGenerator generator(new CTestNoiseGenerator());
	C2DNoise f(generator, true);

	P2DImage result = f.filter(src_image);

	check(result.get(), test_result);

}

BOOST_AUTO_TEST_CASE( test_noise_create_obj )
{
	string init("noise:g=[gauss:mu=1,sigma=10],mod=1"); 
	auto p = BOOST_TEST_create_from_plugin<C2DNoiseImageFilterFactory>(init.c_str()); 
	BOOST_CHECK_EQUAL(p->get_init_string(), init); 
}
