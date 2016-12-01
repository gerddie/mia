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

#include <mia/2d/transform.hh>
#include <mia/internal/autotest.hh>

#include <mia/2d/ica.hh>

using namespace mia;
using namespace std;
using namespace boost::unit_test;

const size_t slices = 5;
const size_t nx = 2;
const size_t ny = 5;
const C2DBounds size(nx, ny);

struct ICASeriesFixture {
	ICASeriesFixture();

protected:
	vector<C2DFImage> image_set;
	vector<float>     mean;
	PIndepCompAnalysisFactory ica_factory; 
};

BOOST_FIXTURE_TEST_CASE ( test_empty_initialization, ICASeriesFixture )
{
	vector<C2DFImage> series;
    BOOST_CHECK_THROW( C2DImageSeriesICA s(*ica_factory, series, false), invalid_argument);
}


BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory, image_set, false);

	ica.run(3, false, false);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_imcomplete_mix, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, false);
	C2DImageSeriesICA::IndexSet skip;
	skip.insert(0);
	skip.insert(1);
	skip.insert(2);

	ica.run(3, false, false);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_incomplete_mix(i, skip);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), mean[i], 0.001);
	}
}


BOOST_FIXTURE_TEST_CASE( test_ica_with_stripped_series_mean, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, true);

	ica.run(3, false, false);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, true);

	ica.run(4, false, false);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_stripped_and_normalized, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, true);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_normalized, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, false);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_normalized2, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, false);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_mix_normalized, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, false);

	ica.run(4,  true, false);

	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_none, ICASeriesFixture )
{
    C2DImageSeriesICA ica(*ica_factory,image_set, false);

	ica.run(4, false, false);
	for (size_t i = 0; i < slices; ++i) {
		C2DFImage mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());

		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y), image_set[i](x,y), 0.001);
	}
}



BOOST_FIXTURE_TEST_CASE( test_ica_mean_substract , ICASeriesFixture)
{
	C2DBounds size(2,3);
	float init_image1[6] = {1, 2, 3, 4, 5, 6};
	float init_image2[6] = {7, 6, 5, 4, 3, 2};


	vector<C2DFImage> images;
	images.push_back(C2DFImage(size, init_image1));
	images.push_back(C2DFImage(size, init_image2));

	C2DImageSeriesICA ica(*ica_factory,images, true);

	const C2DFImage& mean = ica.get_mean_image();

	BOOST_CHECK_EQUAL(mean.get_size(), size);

	for(size_t y = 0; y < size.y; ++y)
		for(size_t x = 0; x < size.x; ++x) {
			BOOST_CHECK_CLOSE(mean(x,y), 4.0f, 0.1);
		}


}



ICASeriesFixture::ICASeriesFixture():
	mean(slices),
	ica_factory( produce_ica_factory("internal"))
{
	float data_rows[slices][nx * ny] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 4, -4,  -5,  -3,  6, -4,  6, -3, -1, 4 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};


	mean[0]= 0.1f;
	mean[1]= 0.3f;
	mean[2]=-1.f;
	mean[3]= 0.f;
	mean[4]=-4.f;

	for (size_t i = 0; i < slices; ++i)
		image_set.push_back(C2DFImage(size, data_rows[i]));
}

