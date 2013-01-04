/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "TEST_HISTOGRAM"
#define BOOST_TEST_DYN_LINK
#include <stdexcept>
#include <vector>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/core/noisegen.hh>

#include <mia/core/histogram.hh>
#include <mia/core/msgstream.hh>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bfs=::boost::filesystem;


BOOST_AUTO_TEST_CASE( test_histogram)
{
	unsigned char init_data[10] = { 1, 2, 3, 4, 5, 5, 5, 5, 4, 3 };
	size_t test_result[256];
	fill( test_result, test_result+256, 0);

	test_result[1] = 1;
	test_result[2] = 1;
	test_result[3] = 2;
	test_result[4] = 2;
	test_result[5] = 4;


	THistogram<CUBHistogramFeeder> h = THistogram<CUBHistogramFeeder>(CUBHistogramFeeder(0,0,0));
	BOOST_CHECK(h.size() == 256);

	for (int i = 0; i < 10; ++i)
		h.push(init_data[i]);

	BOOST_CHECK(equal(h.begin(), h.end(), test_result));

	BOOST_CHECK_EQUAL(h.median(), 4);

	h.push(6, 2);
	BOOST_CHECK_EQUAL(h[6], 2u);


	BOOST_CHECK_EQUAL(h.median(), 4);

	BOOST_CHECK_EQUAL(h.MAD(), 1);

}

BOOST_AUTO_TEST_CASE( test_float_histogram)
{
	float init_data[10] = { 1, 2, 3, 4, 5, 5, 5, 5, 4, 3 };


	THistogram<THistogramFeeder<float> > h(THistogramFeeder<float>(1,5,5));
	h.push_range(init_data, init_data + 10);

	THistogram<THistogramFeeder<float> >::value_type value = h.at(0);

	BOOST_CHECK_EQUAL(value.first, 1);
	BOOST_CHECK_EQUAL(value.second, 1u);

	value = h.at(4);
	BOOST_CHECK_EQUAL(value.first, 5);
	BOOST_CHECK_EQUAL(value.second, 4u);

	BOOST_CHECK_CLOSE(h.average(), 3.7, 0.1);
	BOOST_CHECK_CLOSE(h.deviation(), sqrt((155.0 - 37.0 * 3.7) / 9.0), 0.1);

	THistogram<THistogramFeeder<float> > h2(h,0.0);
	BOOST_CHECK_CLOSE(h2.average(), 3.7, 0.1);
	BOOST_CHECK_CLOSE(h2.deviation(), sqrt((155.0 - 37.0 * 3.7) / 9.0), 0.1);

}

BOOST_AUTO_TEST_CASE( test_histogram2)
{
	const size_t nsamples = 31;
	const float init_data[nsamples] = {
		1, 1, 2, 3, 1, 6, 8, 1,
		1, 4, 6, 8, 5, 1, 1, 2,
		2, 2, 7, 3, 1,
		3, 3, 7, 7, 2,
		2, 4, 1, 8, 2
	};

	size_t test_result[256];
	fill( test_result, test_result+256, 0);
	test_result[0] = 9;
	test_result[1] = 7;
	test_result[2] = 4;
	test_result[3] = 2;
	test_result[4] = 1;
	test_result[5] = 2;
	test_result[6] = 3;
	test_result[7] = 3;


	THistogram<THistogramFeeder<float> > h(THistogramFeeder<float>(1,256,256));
	BOOST_CHECK(h.size() == 256);

	for (size_t i = 0; i < nsamples; ++i)
		h.push(init_data[i]);

	for (size_t i = 0; i < 256; ++i) {
		cvdebug() << "i (" << h.at(i).first << ") = " << h.at(i).second << " vs. " << test_result[i] << "\n";
		BOOST_CHECK_EQUAL(h.at(i).second, test_result[i]);
	}

	BOOST_CHECK_EQUAL(h.median(), 2);
	BOOST_CHECK_EQUAL(h.MAD(), 1);

}

BOOST_AUTO_TEST_CASE( test_histogram3)
{
	const size_t nsamples = 31;
	const float init_data[nsamples] = {
		1, 1, 2, 3, 1, 6, 9, 1,
		1, 4, 6, 9, 5, 1, 1, 2,
		2, 2, 7, 3, 1,
		3, 3, 7, 7, 2,
		2, 4, 1, 9, 2
	};

	THistogram<THistogramFeeder<double> > h(THistogramFeeder<double>(1,9,4096));
	BOOST_CHECK(h.size() == 4096);

	for (size_t i = 0; i < nsamples; ++i)
		h.push(init_data[i]);

	BOOST_CHECK_CLOSE(h.median(), 2.0, 0.1);
	BOOST_CHECK_CLOSE(h.MAD(), 1.0, 0.1);

}

BOOST_AUTO_TEST_CASE( test_histogram_cut_5)
{
	const size_t nsamples = 100;

	THistogram<THistogramFeeder<double> > h(THistogramFeeder<double>(1,100,100));

	for (size_t i = 0; i < nsamples; ++i)
		h.push(i+1);

	auto range = h.get_reduced_range(5); 
	BOOST_CHECK_EQUAL(range.first, 5); 
	BOOST_CHECK_EQUAL(range.second, 95);
}


struct FNormalDistribution {
	FNormalDistribution(float mean, float sigma) :
		m_mean(mean),
		m_sigma(sigma),
		m_w1(1.0 / (2.0 * m_sigma * m_sigma)),
		m_w2(sqrt( m_w1 / M_PI))

	{

	}

	float operator()(float x) const {
		const float h = x - m_mean;
		return m_w2 * exp( - m_w1 * h * h);
	}
	float m_mean;
	float m_sigma;
	float m_w1;
	float m_w2;
};

BOOST_AUTO_TEST_CASE ( test_histogram_gauss_noise )
{
	const size_t size = 32000;

	vector<double> data;

	CNoiseGeneratorPluginHandler::set_search_path({bfs::path("noise")});

	const CNoiseGeneratorPluginHandler::Instance&  ngp = CNoiseGeneratorPluginHandler::instance();

	auto ng = ngp.produce("gauss:mu=127,sigma=16,seed=1");

	FNormalDistribution n1(127, 16);

	typedef THistogram<THistogramFeeder<double> > CDoubleHistogram;
	CDoubleHistogram histo(THistogramFeeder<double>(0,256,64));

	for (size_t k = 0; k < 1; ++k) {
		const CNoiseGenerator& g = *ng;
		size_t i = 0;
		while ( i < size ) {
			double h = g();
			if ( h >= 0.0 && h < 256.0) {
				histo.push(h);
				++i;
			}
		}
	}

	BOOST_CHECK_CLOSE(histo.average(), 127.0 ,1);
	BOOST_CHECK_CLOSE(histo.deviation(), 16.0 ,1);

}
