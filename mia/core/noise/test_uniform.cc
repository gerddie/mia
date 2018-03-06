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
#include <mia/core/noise/uniform.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace uniform_noise_generator;

BOOST_AUTO_TEST_CASE( test_uniform )
{
       const double a = 1.0;
       const double b = 2.0;
       CUniformNoiseGenerator ng(1, a, b);
       double sum1 = 0.0;
       double sum2 = 0.0;
       const size_t n = 10000000;
       size_t k = n;

       while (k--) {
              double val = ng();
              sum1 += val;
              sum2 += val * val;
       }

       cvdebug() << sum1 << " (" << sum2 << ")\n";
       sum1 /= n;
       sum2 = sqrt(( sum2 - n * sum1 * sum1) / (n - 1));
       cvdebug() << sum1 << " (" << sum2 << ")\n";
       double mu = (b + a) * 0.5;
       double sigma = sqrt((b - a) * (b - a) / 12.0);
       BOOST_CHECK_CLOSE(mu, sum1, 1.0);
       BOOST_CHECK_CLOSE(sigma, sum2, 1.0);
}

BOOST_AUTO_TEST_CASE( test_uniform_2 )
{
       const double a = 0.0;
       const double b = 10.0;
       CUniformNoiseGenerator ng(1, a, b);
       double sum1 = 0.0;
       double sum2 = 0.0;
       const size_t n = 10000000;
       size_t k = n;

       while (k--) {
              double val = ng();
              sum1 += val;
              sum2 += val * val;
       }

       cvdebug() << sum1 << " (" << sum2 << ")\n";
       sum1 /= n;
       sum2 = sqrt(( sum2 - n * sum1 * sum1) / (n - 1));
       cvdebug() << sum1 << " (" << sum2 << ")\n";
       double mu = (b + a) * 0.5;
       double sigma = sqrt((b - a) * (b - a) / 12.0);
       BOOST_CHECK_CLOSE(sum1, mu, 1.0);
       BOOST_CHECK_CLOSE(sum2, sigma,  1.0);
}
