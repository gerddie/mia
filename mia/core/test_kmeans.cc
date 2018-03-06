/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <mia/core/kmeans.hh>

NS_MIA_USE;
using namespace std;

BOOST_AUTO_TEST_CASE( test_closes_class_center )
{
       std::vector<double> classes{-10, 10, 20, 30};
       BOOST_CHECK_EQUAL(kmeans_get_closest_clustercenter(classes, 3, -2), 0);
       BOOST_CHECK_EQUAL(kmeans_get_closest_clustercenter(classes, 1, 30), 1);
       BOOST_CHECK_EQUAL(kmeans_get_closest_clustercenter(classes, 3, 24), 2);
       BOOST_CHECK_EQUAL(kmeans_get_closest_clustercenter(classes, 3, 28), 3);
       BOOST_CHECK_EQUAL(kmeans_get_closest_clustercenter(classes, 3, 18), 2);
}

BOOST_AUTO_TEST_CASE( test_kmeans0 )
{
       const size_t nelements = 3;
       const double input_data[nelements] = {
              1.0,  11.0,  20.1
       };
       const int cluster_data[nelements] = {
              0, 1, 2
       };
       vector<int> clustering(nelements);
       vector<double> classes(3);
       kmeans(input_data, input_data + nelements, clustering.begin(), classes);
       BOOST_REQUIRE(classes.size() == 3);
       BOOST_REQUIRE(clustering.size() == nelements);
       BOOST_CHECK_CLOSE(classes[0], 1.0, 0.1);
       BOOST_CHECK_CLOSE(classes[1], 11.0, 0.1);
       BOOST_CHECK_CLOSE(classes[2], 20.1, 0.1);

       for (size_t i = 0; i < nelements; ++i)
              BOOST_CHECK_EQUAL(clustering[i], cluster_data[i]);
}

BOOST_AUTO_TEST_CASE( test_kmeans )
{
       const size_t nelements = 20;
       const double input_data[nelements] = {
              1.0, 3.0,  2.0, 10.4,  2.4, 11.0, 11.1, 25.1, 25.2, 28.0,
              29.7, 11.1, 12.2, 28.0, 29.9,  1.0,  2.1, 27.1,  2.5,  3.1
       };
       const int cluster_data[nelements] = {
              0, 0, 0, 1, 0, 1, 1, 2, 2, 2,
              2, 1, 1, 2, 2, 0, 0, 2, 0, 0
       };
       vector<int> clustering(nelements);
       vector<double> classes(3);
       kmeans(input_data, input_data + nelements, clustering.begin(), classes);
       BOOST_REQUIRE(classes.size() == 3);
       BOOST_REQUIRE(clustering.size() == nelements);
       BOOST_CHECK_CLOSE(classes[0], (1.0 + 3.0 + 2.0 + 1.0 + 2.4 + 2.1 + 2.5 + 3.1) / 8.0, 0.1);
       BOOST_CHECK_CLOSE(classes[1], (10.4 + 11.0 + 11.1 + 11.1 + 12.2) / 5.0, 0.1);
       BOOST_CHECK_CLOSE(classes[2], (25.1 + 25.2 + 28.0 + 29.7 + 28.0 + 29.9 + 27.1) / 7.0, 0.1);

       for (size_t i = 0; i < nelements; ++i) {
              cvdebug() << input_data[i] << " -> " << clustering[i] << ": "
                        << classes[clustering[i]] << "\n";
              BOOST_CHECK_EQUAL(clustering[i], cluster_data[i]);
       }
}

