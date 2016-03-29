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

#include <mia/internal/autotest.hh>


#include <mia/core/ica.hh>
#include <mia/3d/ica.hh>

using namespace mia;
using namespace std;
using namespace boost::unit_test;

const size_t slices = 5;
const size_t nx = 2;
const size_t ny = 5;
const size_t nz = 1;
const C3DBounds size(nx, ny, nz);

struct ICA3DSeriesFixture {
	ICA3DSeriesFixture();


	void check(const C3DFImage& mixed, const C3DFImage& expect);
	void check(const C3DFImage& mixed, float expect);
protected:
	vector<C3DFImage> image_set;
	vector<float>     mean;
};

BOOST_AUTO_TEST_CASE ( test_empty_initialization )
{
	vector<C3DFImage> series;
    BOOST_CHECK_THROW( C3DImageSeriesICA s(CICAAnalysisITPPFactory(),  series, false), invalid_argument);
}


BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);

	ica.run(3,false,false);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_imcomplete_mix, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);
	C3DImageSeriesICA::IndexSet skip;
	skip.insert(0);
	skip.insert(1);
	skip.insert(2);

	ica.run(3,false,false);

	for (size_t i = 0; i < slices; ++i) {
		C3DFImage mixed = ica.get_incomplete_mix(i, skip);
		BOOST_CHECK_EQUAL(mixed.get_size(), image_set[i].get_size());
		BOOST_REQUIRE(mixed.get_size() ==image_set[i].get_size());
		check(mixed, mean[i]);
	}
}


BOOST_FIXTURE_TEST_CASE( test_ica_with_stripped_series_mean, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, true);

	ica.run(3,false,false);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, true);

	ica.run(4, false, false);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_stripped_and_normalized, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, true);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_normalized, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_normalized2, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);

	ica.run(4, true, true);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_mix_normalized, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);

	ica.run(4, true, false);

	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}

BOOST_FIXTURE_TEST_CASE( test_ica_with_some_mean_4comp_none, ICA3DSeriesFixture )
{
    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), image_set, false);

	ica.run(4, false, false);
	for (size_t i = 0; i < slices; ++i)
		check(ica.get_mix(i), image_set[i]);
}



BOOST_AUTO_TEST_CASE( test_ica_mean_substract )
{
	C3DBounds size(2,3,1);
	float init_image1[6] = {1, 2, 3, 4, 5, 6};
	float init_image2[6] = {7, 6, 5, 4, 3, 2};


	vector<C3DFImage> images;
	images.push_back(C3DFImage(size, init_image1));
	images.push_back(C3DFImage(size, init_image2));

    C3DImageSeriesICA ica(CICAAnalysisITPPFactory(), images, true);

	const C3DFImage& mean = ica.get_mean_image();

	BOOST_CHECK_EQUAL(mean.get_size(), size);

	for(C3DFImage::const_iterator m = mean.begin(); m != mean.end(); ++m)
		BOOST_CHECK_CLOSE(*m, 4.0, 0.001);
}



ICA3DSeriesFixture::ICA3DSeriesFixture():
	mean(slices)
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
		image_set.push_back(C3DFImage(size, data_rows[i]));
}

void ICA3DSeriesFixture::check(const C3DFImage& mixed, const C3DFImage& expect)
{
	BOOST_CHECK_EQUAL(mixed.get_size(), expect.get_size());
	BOOST_REQUIRE(mixed.get_size() == expect.get_size());

	for (size_t z = 0; z < nz; ++z)
		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y,z), expect(x,y,z), 0.001);
}

void ICA3DSeriesFixture::check(const C3DFImage& mixed, float expect)
{
	for (size_t z = 0; z < nz; ++z)
		for (size_t y = 0; y < ny; ++y)
			for (size_t x = 0; x < nx; ++x)
				BOOST_CHECK_CLOSE(mixed(x,y,z), expect, 0.001);
}
