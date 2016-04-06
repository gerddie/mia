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
#include <mia/core/cmeansinit/kmeans.hh>

NS_MIA_USE

using std::vector; 

BOOST_AUTO_TEST_CASE (test_kmeans_simple)
{
	const vector<double> input_data = {0.1,  0.4,  0.8}; 

	CMeans::NormalizedHistogram in_h(3);

	for (auto i = 0; i < 3; ++i) {
		in_h[i].first = input_data[i];
		in_h[i].second = 1.0 / 3.0;
	}

	auto initializer = BOOST_TEST_create_from_plugin<CKMeansInitializerPlugin>("kmeans:nc=3");

	auto classes = initializer->run(in_h);

	BOOST_CHECK_EQUAL(classes.size(), 3u);
	
	BOOST_CHECK_CLOSE(classes[0], 0.1f, 0.1);
	BOOST_CHECK_CLOSE(classes[1], 0.4f, 0.1);
	BOOST_CHECK_CLOSE(classes[2], 0.8f, 0.1);
	
}


BOOST_AUTO_TEST_CASE (test_kmeans)
{
	const vector<double> input_bins = {1.0, 3.0, 2.0, 10.4, 2.4,
					   11.0, 11.1, 25.1, 25.2, 28.0,
					   29.7, 12.2, 29.9,
					   2.1, 27.1,  2.5,  3.1
	};

	const vector<double> input_count = {2, 1, 1, 1, 1,
					    1, 2, 1, 1, 2,
					    1, 1, 1, 1, 1, 1, 1}; 
	

	
	CMeans::NormalizedHistogram in_h(17);

	for (auto i = 0; i < 17; ++i) {
		in_h[i].first = input_bins[i];
		in_h[i].second = input_count[i] / 20.0;
	}
	
	auto initializer = BOOST_TEST_create_from_plugin<CKMeansInitializerPlugin>("kmeans:nc=3");
	
	auto classes = initializer->run(in_h);

	BOOST_CHECK_EQUAL(classes.size(), 3u);

	BOOST_CHECK_CLOSE(classes[0], (1.0 + 3.0 + 2.0 + 1.0 + 2.4 + 2.1 + 2.5 + 3.1) / 8.0, 0.1);
	BOOST_CHECK_CLOSE(classes[1], (10.4 + 11.0 + 11.1 + 11.1 + 12.2) / 5.0, 0.1);
	BOOST_CHECK_CLOSE(classes[2], (25.1 + 25.2 + 28.0 + 29.7 + 28.0 + 29.9 + 27.1) / 7.0, 0.1);


}




