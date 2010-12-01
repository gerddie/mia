/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/internal/autotest.hh>

#include <stdexcept>
#include <cmath>

#include <mia/core/slopestatistics.hh>


using namespace std;
NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_check_simple_series )
{
	const size_t curve_length = 10;
	const float init_curve[curve_length] = {
		4, 5, 6, 5, 4, 3, 2, 2, 1, 4
	};

	vector<float> curve(curve_length);
	copy(init_curve, init_curve + curve_length, curve.begin());

	CSlopeStatistics stats(curve);

	BOOST_CHECK_CLOSE(stats.get_curve_length(),
			  7.0f * sqrt(2.0f)+ 1.0f + sqrt(10.0f), 0.1);

	BOOST_CHECK_CLOSE(stats.get_range(), 5.0f, 0.1);

	BOOST_CHECK_EQUAL(stats.get_first_peak().first, 2u);
	BOOST_CHECK_EQUAL(stats.get_second_peak().first, 8u);

	BOOST_CHECK_EQUAL(stats.get_first_peak().second, 6);
	BOOST_CHECK_EQUAL(stats.get_second_peak().second, 1);
}

BOOST_AUTO_TEST_CASE( test_perfusion_series )
{
	const size_t curve_length = 20;
	const float init_curve[curve_length] = {
		15, 12, 12, 5, 4,
		0, -12, -10, -8, -9,
		-7, -6, -5, -6, -4,
		-2, -1, -1, -1, -1
	};
	vector<float> curve(curve_length);
	copy(init_curve, init_curve + curve_length, curve.begin());

	CSlopeStatistics stats(curve);

	BOOST_CHECK_CLOSE(stats.get_range(), 27.0f, 0.1);

	BOOST_CHECK_EQUAL(stats.get_perfusion_high_peak().first, 6u);
	BOOST_CHECK_EQUAL(stats.get_perfusion_high_peak().second, -12);
}


BOOST_AUTO_TEST_CASE( test_perfusion_mean_freq )
{
	const size_t curve_length = 20;
	const float init_curve[curve_length] = {
		15, 12, 12, 5, 4,0, -12, -10, -8, -9,-7, -6, -5, -6, -4, -2, -1, -1, -1, -1
	};
	vector<float> curve(init_curve, init_curve + curve_length);
	CSlopeStatistics stats(curve);

	BOOST_CHECK_CLOSE(stats.get_mean_frequency(),3.51f, 0.1);

}

BOOST_AUTO_TEST_CASE( test_perfusion_mean_freq2 )
{
	const size_t curve_length = 30;
	const float init_curve[curve_length] = {
		3.5100e-03,
		1.7162e+03,
		3.6592e+03,
		1.7118e+03,
		9.6635e+02,
		7.2798e+02,
		4.2911e+02,
		1.2834e+02,
		1.7935e+02,
		2.3231e+02,
		2.4314e+02,
		3.0963e+02,
		2.3646e+02,
		2.7485e+02,
		2.3438e+02,
		1.3227e+02,
		5.2508e+01,
		7.1757e+01,
		1.3509e+02,
		1.0618e+02,
		1.6261e+02,
		1.3636e+02,
		6.1491e+01,
		5.7952e+01,
		7.4816e+01,
		6.7554e+01,
		1.0019e+02,
		1.3639e+02,
		1.5046e+02,
		4.0949e+01

	};
	vector<float> curve(init_curve, init_curve + curve_length);
	CSlopeStatistics stats(curve);

	BOOST_CHECK_CLOSE(stats.get_mean_frequency(), 5.23036051f, 0.1);

}
