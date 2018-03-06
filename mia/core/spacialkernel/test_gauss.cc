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
#include <mia/core/spacialkernel/gauss.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace gauss_1d_folding_kernel;

BOOST_AUTO_TEST_CASE( test_gauss )
{
       C1DGaussFilterKernel kernel(1);
       bool success = kernel.size() == 3;
       success &= (kernel[0] == kernel[2]);
       success &= (kernel[1] == .5);
       success &= (kernel[2] == 0.25);
       const double input[5] = {0, 0, 1, 0, 0 };
       const double input2[5] = {1, 1, 1, 1, 1 };
       const double input3[5] = {2, 1, 1, 1, 3 };
       const double test_out_1[5] = {0, 0.25, 0.5, 0.25, 0 };
       const double test_out_2[5] = {0.0625, 0.25, 0.375, 0.25, 0.0625 };
       const double test_out_3[5] = {1, 1, 1, 1, 1};
       const double test_out_4[5] = {0.109375, 0.234375, 0.3125, 0.234375, 0.109375};
       // this is  mirror on boundary
       const double test_out_31[5] = {1.75, 1.25, 1, 1.5, 2.5};
       vector<double> in1(5);
       copy(input, input + 5, in1.begin());
       kernel.apply_inplace(in1);
       success &= (equal(in1.begin(), in1.end(), test_out_1));
       C1DGaussFilterKernel kernel2(2);
       success &= (kernel2.size() == 5);
       success &= (kernel2[0] == kernel2[4]);
       success &= (kernel2[1] == kernel2[3]);
       success &= (kernel2[0] == .0625);
       success &= (kernel2[1] == 0.25);
       success &= (kernel2[2] == 0.375);
       copy(input, input + 5, in1.begin());
       kernel2.apply_inplace(in1);
       success &= (equal(in1.begin(), in1.end(), test_out_2, equal_double()));

       if (!success)  {
              for (size_t i = 0; i < 5; ++i)
                     cvfail() << in1[i] << " vs " << test_out_2[i] << "\n";
       }

       copy(input2, input2 + 5, in1.begin());
       kernel2.apply_inplace(in1);
       success &= (equal(in1.begin(), in1.end(), test_out_3, equal_double()));

       if (!success)  {
              for (size_t i = 0; i < 5; ++i)
                     cvfail() << in1[i] << " vs " << test_out_3[i] << "\n";
       }

       // add a test where the kernel is bigger then the input data
       C1DGaussFilterKernel kernel3(3);
       success &= (kernel3.size() == 7);
       success &= (kernel3[0] == kernel3[6]);
       success &= (kernel3[1] == kernel3[5]);
       success &= (kernel3[2] == kernel3[4]);
       success &= (kernel3[0] == 0.015625);
       success &= (kernel3[1] == 0.09375);
       success &= (kernel3[2] == 0.234375);
       success &= (kernel3[3] == 0.3125);
       copy(input, input + 5, in1.begin());
       kernel3.apply_inplace(in1);
       success &= (equal(in1.begin(), in1.end(), test_out_4, equal_double()));

       if (!success)  {
              for (size_t i = 0; i < 5; ++i)
                     cvfail() << in1[i] << " vs " << test_out_4[i] << "\n";
       }

       copy(input3, input3 + 5, in1.begin());
       kernel.apply_inplace(in1);
       success &= (equal(in1.begin(), in1.end(), test_out_31, equal_double()));

       if (!success)  {
              for (size_t i = 0; i < 5; ++i)
                     cvfail() << in1[i] << " vs " << test_out_31[i] << "\n";
       }
}

