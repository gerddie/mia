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
#include <mia/2d/filter/distance.hh>
#include <mia/2d/imagetest.hh>


NS_MIA_USE
using namespace std;
using namespace distance_2d_filter;


BOOST_AUTO_TEST_CASE( test_distance_double )
{
       C2DBitImage input(C2DBounds(3, 5));
       fill(input.begin(), input.end(), 0);
       input(1, 2) = 1;
       const C2DImage& filter_input = input;
       C2DDImage ref(C2DBounds(3, 5));
       ref(0, 0) = ref(2, 0) = ref(0, 4) = ref(2, 4) = sqrt(5);
       ref(0, 1) = ref(2, 1) = ref(0, 3) = ref(2, 3) = sqrt(2);
       ref(1, 1) = ref(1, 3) = ref(0, 2) = ref(2, 2) = 1.0;
       ref(1, 0) = ref(1, 4) = 2;
       ref(1, 2) = 0;
       C2DDistance filter;
       P2DImage rwrap = mia::filter( filter, filter_input );
       test_image_equal(*rwrap, ref);
}


