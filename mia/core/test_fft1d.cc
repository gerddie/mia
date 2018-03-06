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

#include <stdexcept>
#include <cmath>

#include <mia/core/fft1d_r2c.hh>

const size_t n = 32;
using namespace std;
using namespace mia;

BOOST_AUTO_TEST_CASE ( test_fftw_2dfloat )
{
       CFFT1D_R2C fft(n);
       const float in_data[n] = {
              0, 10, 20, 25, 20, 8, -1, -9, -19, -21, -23, -20, -11, 1, 11, 19,
              26, 20, 8, -1, -9, -19, -21, -23, -20, -11, 1, 11, 19, 26, 18, 8
       };
       const float test_data[n / 2 + 1][2]  = {
              {  43.0000, +0.0000},
              {  108.8248, +0.1413},
              {   275.3777, +1.1785},
              {  -198.7287, -1.8731},
              {   -58.8701, -0.4142},
              {   -64.7561, -15.0292},
              {   -25.7085, -0.9779},
              {   -26.4171, +4.9283},
              {    -7.0000, -4.0000},
              {   -10.9279, +0.7783},
              {    12.8730, -8.3922},
              {    -3.9231, +0.6853},
              {  -5.1299, -2.4142},
              {    -3.4693, +7.0129},
              {    -2.5422, +5.7642},
              {    -8.6026, -2.8371},
              {    -5.0000, +0.0000}
       };
       vector<CFFT1D_R2C::Real> in_data_vector(in_data, in_data + 32);
       vector<CFFT1D_R2C::Complex> complex_result = fft.forward(in_data_vector);
       BOOST_CHECK_EQUAL(fft.out_size(), n / 2 + 1);
       BOOST_CHECK_EQUAL(complex_result.size(), n / 2 + 1);

       for ( size_t i = 0; i < n / 2 + 1; ++i) {
              BOOST_CHECK_CLOSE(test_data[i][0],  complex_result[i].real(), 0.1);
              BOOST_CHECK_CLOSE(test_data[i][1],  complex_result[i].imag(), 0.1);
       }

       vector<CFFT1D_R2C::Real>    real_result =  fft.backward(complex_result);
       BOOST_CHECK_EQUAL(real_result.size(), n);

       for ( size_t i = 0; i < n; ++i)
              BOOST_CHECK_CLOSE(real_result[i], n * in_data[i], 0.1);
}
