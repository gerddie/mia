/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <stdexcept>
#include <cmath>

#include <mia/core/slopestatistics.hh>


using namespace std;
NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_check_simple_series )
{
	vector<float> curve = {
		4, 5, 6, 5, 4, 3, 2, 2, 1, 4
	};
	int index = 3; 

	CSlopeStatistics stats(curve, index);

	BOOST_CHECK_CLOSE(stats.get_curve_length(),
			  7.0f * sqrt(2.0f)+ 1.0f + sqrt(10.0f), 0.1);

	BOOST_CHECK_CLOSE(stats.get_range(), 5.0f, 0.1);

	BOOST_CHECK_EQUAL(stats.get_index(), index);

	BOOST_CHECK_EQUAL(stats.get_first_peak().first, 2u);
	BOOST_CHECK_EQUAL(stats.get_second_peak().first, 8u);

	BOOST_CHECK_EQUAL(stats.get_first_peak().second, 6);
	BOOST_CHECK_EQUAL(stats.get_second_peak().second, 1);
}

BOOST_AUTO_TEST_CASE( test_perfusion_series )
{
	vector<float> curve= {
		-15, -12, -12, -5, -4,
		0, 12, 10, 8, 9,
		7, 6, 5, 6, 4,
		2, 1, 1, 1, 1
	};

	CSlopeStatistics stats(curve, 0);

	BOOST_CHECK_CLOSE(stats.get_range(), 27.0f, 0.1);

	BOOST_CHECK_EQUAL(stats.get_perfusion_high_peak().first, 6u);
	BOOST_CHECK_EQUAL(stats.get_perfusion_high_peak().second, 12);
}


BOOST_AUTO_TEST_CASE( test_perfusion_mean_freq_and_energy )
{
	vector<float> curve = {
		15, 12, 12, 5, 4,0, -12, -10, -8, -9,-7, -6, -5, -6, -4, -2, -1, -1, -1, -1
	};
	CSlopeStatistics stats(curve, 0);

	BOOST_CHECK_CLOSE(stats.get_mean_frequency(),3.51f, 0.1);

	BOOST_CHECK_CLOSE(stats.get_energy(),238.83f, 0.1);

}

BOOST_AUTO_TEST_CASE( test_perfusion_mean_freq2 )
{
	vector<float> curve =  {
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
	CSlopeStatistics stats(curve, 0);

	BOOST_CHECK_CLOSE(stats.get_mean_frequency(), 5.23036051f, 0.1);

}


struct WaveletFixture {
	struct Expect {
		float mean_level; 
		float peak_coeff; 
		int peak_level; 
		int peak_index; 
		vector<float> energy_levels; 
		vector<CSlopeStatistics::EEnergyCenterpos> energy_timepos; 
		CSlopeStatistics::EEnergyCenterpos mean_energy_timepos; 
	}; 
	void check(const vector<float>& x, const Expect& e) const; 
}; 

void WaveletFixture::check(const vector<float>& x, const Expect& e) const
{
	CSlopeStatistics stats(x, 0);

	BOOST_CHECK_CLOSE(stats.get_mean_frequency_level(), e.mean_level, 0.1);

	BOOST_CHECK_CLOSE(stats.get_peak_wavelet_coefficient(), e.peak_coeff, 0.1);
	auto idx = stats.get_peak_level_and_time_index(); 
	
	BOOST_CHECK_EQUAL(idx.first, e.peak_level); 
	BOOST_CHECK_EQUAL(idx.second, e.peak_index);

	auto le = stats.get_level_coefficient_sums(); 
	BOOST_REQUIRE(le.size() == e.energy_levels.size()); 
	for (size_t i = 0; i < le.size(); ++i) 
		BOOST_CHECK_CLOSE(le[i], e.energy_levels[i], 0.1); 


	BOOST_CHECK_CLOSE(stats.get_wavelet_energy(), 
			  accumulate(e.energy_levels.begin(), e.energy_levels.end(), 0.0), 0.1); 

	auto etp = stats.get_level_mean_energy_position(); 
	BOOST_REQUIRE(etp.size()== e.energy_timepos.size()); 
	for (size_t i = 0; i < etp.size(); ++i) {
		cvdebug() <<  i << ": got " << etp[i] << " expect " << e.energy_timepos[i] << "\n"; 
		BOOST_CHECK_EQUAL(etp[i], e.energy_timepos[i]);
	}
	
	BOOST_CHECK_EQUAL(stats.get_mean_energy_position(), e.mean_energy_timepos); 
}

BOOST_FIXTURE_TEST_CASE( test_wavelet_statistics_baseline, WaveletFixture )
{
	vector<float> curve = {
		-0.337928,
		-0.503076,
		-0.51522,
		-0.250471,
		-0.375565,
		-0.471721,
		-0.0341674,
		-0.521398,
		-0.374229,
		-0.129993,
		-0.075652,
		0.0399395,
		-0.219216,
		0.0896999,
		0.0679431,
		-0.0062162,
		0.146572,
		0.119675,
		0.211168,
		0.483478,
		0.0523503,
		0.317688,
		0.359736,
		0.404177,
		0.160111,
		0.33735,
		0.288014,
		0.398939,
		-0.15057,
		0.0587856,
		0.183049,
		0.326591,
		0.20704,
		0.23325,
		-0.102197,
		0.0775201,
		0.105982,
		-0.105662,
		-0.250138,
		0.194694,
		0.0859516,
		-0.0637214,
		-0.213735,
		0.152096,
		0.0638535,
		-0.248255,
		-0.0886441,
		0.123961,
		0.0355604,
		-0.0504062,
		-0.215511,
		0.143322,
		0.175857,
		0.210724,
		-0.256261,
		-0.130833,
		-0.0145948,
		-0.149502

	};
	Expect e = {
		1.9639f, 
		0.910862f, 
		1, 
		1, 
		{0.57569, 1.26270, 1.61808, 1.08937, 2.30959, 4.21920 }, 
		{CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_end, CSlopeStatistics::ecp_center, 
		 CSlopeStatistics::ecp_begin, CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_center }, 
		CSlopeStatistics::ecp_center
	}; 
	check(curve, e); 
}

BOOST_FIXTURE_TEST_CASE( test_wavelet_statistics_RV, WaveletFixture )
{
	vector<float> curve = {
		-4.3535,
		-4.02309,
		-3.98719,
		-3.97582,
		-3.98267,
		-4.04352,
		-4.35104,
		-2.14759,
		5.04918,
		7.97761,
		8.6514,
		8.28726,
		7.38352,
		5.6293,
		4.52537,
		3.17359,
		1.02402,
		0.704314,
		-0.344782,
		-1.03504,
		-1.77977,
		-1.43475,
		-1.33758,
		-1.60613,
		-1.43021,
		-0.777487,
		-0.635124,
		-0.430965,
		-0.212497,
		0.174651,
		0.440661,
		0.396136,
		0.447888,
		0.239261,
		-0.192818,
		-0.364814,
		-0.383997,
		-0.646301,
		-0.408086,
		-0.482292,
		-0.402189,
		-0.814235,
		-0.663673,
		-0.767067,
		-0.636486,
		-0.610515,
		-0.556308,
		-0.630348,
		-0.59308,
		-0.530053,
		-0.337022,
		-0.449969,
		-0.50345,
		-0.391277,
		-0.423272,
		-0.464304,
		-0.454401,
		-0.509432
	};
	Expect e = {
		2.0091,
		12.3495f, 
		3, 
		1, 
		{6.9179, 12.1628, 18.4302, 30.5213, 9.1098, 7.6350}, 
		{CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_end, CSlopeStatistics::ecp_begin, 
		 CSlopeStatistics::ecp_begin, CSlopeStatistics::ecp_begin, CSlopeStatistics::ecp_center }, 
		CSlopeStatistics::ecp_begin

	}; 
	check(curve, e); 
}

BOOST_FIXTURE_TEST_CASE( test_wavelet_statistics_perf, WaveletFixture )
{
	vector<float> curve = {
		-2.8777,
		-2.739,
		-2.79981,
		-2.90742,
		-2.97203,
		-2.8741,
		-3.25516,
		-2.84955,
		-3.13471,
		-3.1547,
		-3.14835,
		-3.2423,
		-2.73647,
		-3.87446,
		-4.02871,
		-3.87094,
		-2.74141,
		-2.88794,
		-1.29941,
		-0.679071,
		0.0474983,
		0.777731,
		1.73883,
		2.02613,
		2.64497,
		2.48394,
		2.42826,
		2.26612,
		2.19441,
		2.25864,
		1.90372,
		1.5902,
		1.54211,
		1.37297,
		1.23322,
		1.35887,
		1.26686,
		1.15585,
		1.47335,
		1.1583,
		1.30924,
		1.31398,
		1.57202,
		1.36046,
		1.42322,
		1.56397,
		1.5021,
		1.35904,
		1.40978,
		1.36603,
		1.61815,
		1.34705,
		1.32077,
		1.31044,
		1.32386,
		1.41111,
		1.33357,
		1.30646
	};
	Expect e = {
		1.2745,
		13.8267f, 
		1, 
		1, 
		{5.9352,  18.0967,  8.2709,    5.0151,    4.6476,    6.4534 }, 
		{CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_end, CSlopeStatistics::ecp_center, 
		 CSlopeStatistics::ecp_begin, CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_center }, 
		CSlopeStatistics::ecp_center
		
	}; 
	check(curve, e); 
}

BOOST_FIXTURE_TEST_CASE( test_wavelet_statistics_mov, WaveletFixture )
{
	vector<float> curve = {
		-5.81013,
		-0.822572,
		1.84109,
		2.02425,
		2.06275,
		-1.04509,
		-6.13801,
		-1.60957,
		1.20525,
		1.39125,
		1.84946,
		2.02153,
		-3.6345,
		0.978247,
		2.15678,
		-1.04762,
		-6.5762,
		0.979293,
		2.31742,
		2.32557,
		-5.89342,
		0.913827,
		2.7586,
		2.11121,
		1.59757,
		2.38534,
		2.82347,
		2.56165,
		-2.77743,
		0.39981,
		2.62729,
		2.40708,
		2.69796,
		1.85263,
		-5.51786,
		1.13038,
		2.40133,
		-5.04566,
		-0.661364,
		1.94749,
		2.18185,
		-5.29327,
		-0.049137,
		1.97671,
		2.22867,
		-3.55545,
		1.20774,
		2.24357,
		2.19708,
		-0.292082,
		-4.88995,
		1.82765,
		2.58816,
		2.31172,
		-4.82235,
		-4.56921,
		2.02028,
		-2.50106
	};
	Expect e = {
		3.0250,
		6.11027f, 
		5, 
		17, 
		{ 2.3575, 4.6265,  6.2766,   10.5313,   41.1598,   76.7153  }, 
		{CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_end, CSlopeStatistics::ecp_center, 
		 CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_center }, 
		CSlopeStatistics::ecp_center

	}; 
	check(curve, e); 
}

BOOST_FIXTURE_TEST_CASE( test_wavelet_statistics_LV, WaveletFixture )
{
	vector<float> curve = {
		-2.973,
		-3.26204,
		-3.20777,
		-3.23826,
		-3.23519,
		-3.20101,
		-2.92641,
		-3.34932,
		-3.46378,
		-3.46901,
		-3.34062,
		-3.23943,
		-2.61829,
		-0.573539,
		1.69087,
		3.83384,
		4.55864,
		7.22177,
		7.52901,
		6.60499,
		4.84741,
		4.9098,
		3.40621,
		2.53109,
		1.04984,
		0.189705,
		-0.0477628,
		-0.375392,
		-0.567252,
		-0.675114,
		-0.547272,
		-0.58087,
		-0.336021,
		-0.297511,
		-0.211432,
		0.106161,
		0.41867,
		0.059048,
		0.447294,
		0.387808,
		0.384867,
		0.054274,
		0.172588,
		0.0103516,
		0.0588318,
		-0.339614,
		-0.0998046,
		-0.231888,
		-0.18366,
		-0.327193,
		-0.431454,
		-0.358579,
		-0.397978,
		-0.440532,
		-0.624508,
		-0.573838,
		-0.277873,
		-0.449868 
	};
	Expect e = {
		1.7652, 
		12.9188f, 
		2,
		1, 
		{6.6206, 7.5725, 24.8336, 16.9259,  6.4489, 6.6958}, 
		{CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_end, CSlopeStatistics::ecp_center, 
		 CSlopeStatistics::ecp_begin, CSlopeStatistics::ecp_center, CSlopeStatistics::ecp_center }, 
		CSlopeStatistics::ecp_center
	}; 
	check(curve, e); 
}
