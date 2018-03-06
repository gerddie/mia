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

#include <mia/internal/autotest.hh>
#include <mia/2d/similarity_profile.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/filter.hh>


namespace bfs =::boost::filesystem;
NS_MIA_USE;

struct SimityProfileFixture {
       SimityProfileFixture();

       P2DFullCost cost;
       C2DImageSeries series;

};


BOOST_FIXTURE_TEST_CASE (test_C2DSimilarityProfile_ref10, SimityProfileFixture)
{
       C2DSimilarityProfile sp(cost, series, 10, 0);
       // test value obtained by using octave
       BOOST_CHECK_CLOSE(sp.get_peak_frequency(), 108.98704, 0.1);
}

BOOST_FIXTURE_TEST_CASE (test_C2DSimilarityProfile_ref10_profile, SimityProfileFixture)
{
       C2DSimilarityProfile sp(cost, series, 10, 0);
       auto subset = sp.get_periodic_subset();
       size_t test_subset[] = {0, 3, 10, 13, 20, 23, 30, 33, 39};
       BOOST_CHECK_EQUAL(subset.size(), 9);
       cvdebug() << "got subset: " << subset << "\n";

       for (int i = 0; i < 9; ++i) {
              BOOST_CHECK_EQUAL(subset[i], test_subset[i]);
       }
}




SimityProfileFixture::SimityProfileFixture()
{
       cost = C2DFullCostPluginHandler::instance().produce("image:cost=ssd");
       C2DBounds size(1, 1);
       float values[40] = {
              -2, -3, -4, -1, 1, 4, 2, -1, -3, -2.1,
              -2, -3, -4, -1, 1, 4, 2, -1, -3, -2.1,
              -2, -3, -4, -1, 1, 4, 2, -1, -3, -2.1,
              -2, -3, -4, -1, 1, 4, 2, -1, -3, -2.1
       };

       for (size_t i = 0; i < 40; ++i) {
              C2DFImage *img = new C2DFImage(size);
              (*img)(0, 0) = values[i];
              series.push_back(P2DImage(img));
       }
}

