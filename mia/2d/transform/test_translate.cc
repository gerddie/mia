/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/2d/transform/translate.hh>

NS_MIA_USE



struct TranslateTransformFixture {

	TranslateTransformFixture();

	C2DBounds size;
	C2DFVector value;
	C2DTranslateTransformation transf;
};

TranslateTransformFixture::TranslateTransformFixture():
	size(5,6),
	value(-2, 3),
	transf(size, value)
{
}

BOOST_FIXTURE_TEST_CASE(test_transform, TranslateTransformFixture)
{
	auto r = transf.transform(C2DFVector(1,2));
	BOOST_CHECK_CLOSE(r.x, -1, 0.1);
	BOOST_CHECK_CLOSE(r.y,  5, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_upscale, TranslateTransformFixture)
{
	C2DBounds new_size(10,12);
	auto r = transf.upscale(new_size);
	auto a = dynamic_cast<const C2DTranslateTransformation&>(*r);
	BOOST_CHECK_EQUAL(a.get_size(), new_size);

	auto params = a.get_parameters();
	BOOST_CHECK_EQUAL(params.size(), 2);
	params[0] = -4;
	params[1] =  6;
}

BOOST_FIXTURE_TEST_CASE(test_gradtranslate, TranslateTransformFixture)
{
	C2DFVectorfield field(size);

	C2DFVector f(0,0);
	for(size_t y = 0; y < size.y;++y)
		for(size_t x = 0; x < size.x;++x) {
			field(x,y) = C2DFVector(x, y);
			f.x += x;
			f.y += y;
		}

	gsl::DoubleVector a(2);
	transf.translate(field, a);

	BOOST_CHECK_CLOSE(a[0], f.x / (size.x * size.y), 0.1);
	BOOST_CHECK_CLOSE(a[1], f.y / (size.x * size.y), 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_get_params, TranslateTransformFixture)
{
	auto a = transf.get_parameters();
	BOOST_CHECK_EQUAL(a.size(), 2);
	BOOST_CHECK_EQUAL(a[0], -2);
	BOOST_CHECK_EQUAL(a[1],  3);
}

BOOST_FIXTURE_TEST_CASE(test_set_params, TranslateTransformFixture)
{
	auto a = transf.get_parameters();
	a[0] = 2;
	a[1] = -1;
	transf.set_parameters(a);
	auto b = transf.get_parameters();
	BOOST_CHECK_EQUAL(b.size(), 2);
	BOOST_CHECK_EQUAL(b[0],a[0]);
	BOOST_CHECK_EQUAL(b[1],a[1]);
}

BOOST_FIXTURE_TEST_CASE(test_invert_params, TranslateTransformFixture)
{
	auto a = transf.get_parameters();
	a[0] = 2;
	a[1] = -1;
	transf.set_parameters(a);
	unique_ptr<C2DTransformation> inverse( transf.invert()); 
	
	auto b = inverse->get_parameters();
	BOOST_CHECK_EQUAL(b.size(), 2);
	BOOST_CHECK_EQUAL(b[0],-a[0]);
	BOOST_CHECK_EQUAL(b[1],-a[1]);
}

