/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
	sum2 = sqrt(( sum2 - n * sum1 * sum1) / (n-1));

	cvdebug() << sum1 << " (" << sum2 << ")\n";
	double mu = (b+a) * 0.5;
	double sigma = sqrt((b-a) * (b-a) / 12.0);

	BOOST_CHECK_CLOSE(mu, sum1, 1.0);
	BOOST_CHECK_CLOSE(sigma, sum2, 1.0);

}
