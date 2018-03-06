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


#include <mia/internal/plugintester.hh>
#include <mia/core/nccsum.hh>

using namespace mia;

BOOST_AUTO_TEST_CASE( test_nccsum_has_samples )
{
       NCCSums sums;
       BOOST_CHECK(!sums.has_samples());
       sums.add(1, 2);
       BOOST_CHECK(sums.has_samples());
}

BOOST_AUTO_TEST_CASE( test_nccsum_equal )
{
       const float src_data[4] = { 1, 1, 1, 3 };
       NCCSums sums_equal;

       for (int i = 0; i < 4; ++i)
              sums_equal.add(src_data[i], src_data[i]);

       BOOST_CHECK_SMALL(sums_equal.value(), 1e-5);
       std::pair<double, NCCGradHelper> vgh = sums_equal.get_grad_helper();
       BOOST_CHECK_SMALL(vgh.first, 1e-5);

       for (int i = 0; i < 4; ++i)
              BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[i], src_data[i]), 1e-5f);
}

BOOST_AUTO_TEST_CASE( test_nccsum_no_corr )
{
       const float src_data[4] = { 1, 1, 1, 1 };
       const float ref_data[4] = { 0, 2, 2, 0 };
       NCCSums sums_equal;

       for (int i = 0; i < 4; ++i)
              sums_equal.add(src_data[i], ref_data[i]);

       BOOST_CHECK_CLOSE(sums_equal.value(), 1.0, 1e-5);
       std::pair<double, NCCGradHelper> vgh = sums_equal.get_grad_helper();
       BOOST_CHECK_CLOSE(vgh.first, 1.0, 1e-5);

       // the correlation is at its minimum (and the cost function at the max)
       // therefore the gradient is zero.
       for (int i = 0; i < 4; ++i)
              BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[i], ref_data[i]), 1e-5f);
}

BOOST_AUTO_TEST_CASE( test_nccsum_summed )
{
       const float src_data1[4] = { 1, 1, 1, 4 };
       const float ref_data1[4] = { 2, 2, 2, 8 };
       const float src_data2[4] = { 2, 3, 1, 2 };
       const float ref_data2[4] = { 1, 2, 4, 3 };
       NCCSums sums1;
       NCCSums sums2;
       NCCSums sum;

       for (int i = 0; i < 4; ++i) {
              sums1.add(src_data1[i], ref_data1[i]);
              sums2.add(src_data2[i], ref_data2[i]);
              sum.add(src_data1[i], ref_data1[i]);
              sum.add(src_data2[i], ref_data2[i]);
       }

       NCCSums sump = sums1 + sums2;
       BOOST_CHECK_CLOSE(sump.value(), sum.value(), 0.01);
}

BOOST_AUTO_TEST_CASE( test_nccsum_double )
{
       const float src_data[4] = { 1, 1, 1, 4 };
       const float ref_data[4] = { 2, 2, 2, 8 };
       NCCSums sums_equal;

       for (int i = 0; i < 4; ++i)
              sums_equal.add(src_data[i], ref_data[i]);

       BOOST_CHECK_SMALL(sums_equal.value(), 1e-5);
       std::pair<double, NCCGradHelper> vgh = sums_equal.get_grad_helper();
       BOOST_CHECK_SMALL(vgh.first, 1e-5);

       // the correlation is at its minimum (and the cost function at the max)
       // therefore the gradient is zero.
       for (int i = 0; i < 4; ++i)
              BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[i], ref_data[i]), 1e-5f);
}


BOOST_AUTO_TEST_CASE( test_nccsum_zero_with_corr )
{
       const float src_data[4] = { 1, 1, 1, 1 };
       NCCSums sums_equal;

       for (int i = 0; i < 4; ++i)
              sums_equal.add(src_data[i], src_data[i]);

       BOOST_CHECK_SMALL(sums_equal.value(), 1e-5);
       std::pair<double, NCCGradHelper> vgh = sums_equal.get_grad_helper();
       BOOST_CHECK_SMALL(vgh.first, 1e-5);

       // the correlation is at its minimum (and the cost function at the max)
       // therefore the gradient is zero.
       for (int i = 0; i < 4; ++i)
              BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[i], src_data[i]), 1e-5f);
}


BOOST_AUTO_TEST_CASE( test_nccsum_some_corr )
{
       const float src_data[4] = { 1, 1, 2, 1 };
       const float ref_data[4] = { 0, 2, 2, 1 };
       NCCSums sums_equal;

       for (int i = 0; i < 4; ++i)
              sums_equal.add(src_data[i], ref_data[i]);

       BOOST_CHECK_CLOSE(sums_equal.value(), 1.0 - 0.27272727, 1e-5);
       std::pair<double, NCCGradHelper> vgh = sums_equal.get_grad_helper();
       BOOST_CHECK_CLOSE(vgh.first, 1.0 - 0.27272727, 1e-5);
       // the correlation is at its minimum (and the cost function at the max)
       // therefore the gradient is zero.
       float v0 = 0.7272727272;
       float v1 = -0.7272727272;
       BOOST_CHECK_CLOSE(vgh.second.get_gradient_scale(src_data[0], ref_data[0]), v0, 0.1);
       BOOST_CHECK_CLOSE(vgh.second.get_gradient_scale(src_data[1], ref_data[1]), v1, 0.1);
       BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[2], ref_data[2]), 1e-5f);
       BOOST_CHECK_SMALL(vgh.second.get_gradient_scale(src_data[3], ref_data[3]), 1e-5f);
}
