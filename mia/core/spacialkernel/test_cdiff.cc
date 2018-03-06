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
#include <mia/core/spacialkernel/cdiff.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace cdiff_1d_folding_kernel;

BOOST_AUTO_TEST_CASE( test_cdiff )
{
       auto kernel = BOOST_TEST_create_from_plugin<C1DSpacialCDiffKernelPlugin>("cdiff");
       BOOST_CHECK_EQUAL(kernel->size(), 3u);
       BOOST_CHECK_EQUAL((*kernel)[0], -1);
       BOOST_CHECK_EQUAL((*kernel)[1],  0);
       BOOST_CHECK_EQUAL((*kernel)[2],  1);
       vector<double> input = {0, 1, 2, -1, 0 };
       kernel->apply_inplace(input);
       BOOST_CHECK_SMALL(input[0], 1e-10);
       BOOST_CHECK_CLOSE(input[1], 1, 0.1);
       BOOST_CHECK_CLOSE(input[2], -1, 0.1);
       BOOST_CHECK_CLOSE(input[3], -1, 0.1);
       BOOST_CHECK_SMALL(input[4], 1e-10);
}

BOOST_AUTO_TEST_CASE( test_scharr )
{
       auto kernel = BOOST_TEST_create_from_plugin<C1DScharrKernelPlugin>("scharr");
       BOOST_CHECK_EQUAL(kernel->size(), 3u);
       BOOST_CHECK_EQUAL((*kernel)[0], 3.0 / 16.0);
       BOOST_CHECK_EQUAL((*kernel)[1], 10.0 / 16.0);
       BOOST_CHECK_EQUAL((*kernel)[2], 3.0 / 16.0);
       vector<double> input1 = {1, 1, 1, 1, 1};
       kernel->apply_inplace(input1);
       BOOST_CHECK_CLOSE(input1[0], 1.0, 0.001);
       BOOST_CHECK_CLOSE(input1[1], 1.0, 0.001);
       BOOST_CHECK_CLOSE(input1[2], 1.0, 0.001);
       BOOST_CHECK_CLOSE(input1[3], 1.0, 0.001);
       BOOST_CHECK_CLOSE(input1[4], 1.0, 0.001);
       vector<double> input = {0, 1, 2, -1, 0 };
       kernel->apply_inplace(input);
       BOOST_CHECK_CLOSE(input[0], 3.0 / 8.0, 0.001);
       BOOST_CHECK_CLOSE(input[1], 1.0, 0.001);
       BOOST_CHECK_CLOSE(input[2], 20.0 / 16.0, 0.001);
       BOOST_CHECK_CLOSE(input[3], -1.0 / 4.0, 0.001);
       BOOST_CHECK_CLOSE(input[4], -3.0 / 8.0, 0.001);
}


