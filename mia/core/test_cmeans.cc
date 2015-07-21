/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <cmath>
#include <mia/core/cmeans.hh>


using namespace mia;

using std::vector; 

double gauss(double x , double c, double sigma)
{
	const double f = sqrt(2 * 3.1415926); 
	double d = x - c;
	return 100000 * exp(- d * d / (2 * sigma * sigma)) / (sigma * f); 
}

class CTestFixedInitializer: public CMeans::Initializer
{
public:
	CTestFixedInitializer(const CMeans::DVector& init);
	CMeans::DVector run(const CMeans::NormalizedHistogram& nh) const; 
private:
	CMeans::DVector m_init; 
};

CTestFixedInitializer::CTestFixedInitializer(const CMeans::DVector& init):
	m_init(init)
{
}

CMeans::DVector CTestFixedInitializer::run(const CMeans::NormalizedHistogram& nh) const
{
	return m_init; 
}

	
BOOST_AUTO_TEST_CASE( test_even_initialized )
{

        CMeans::SparseHistogram sh(250);

	vector<double> x = {
		2,    4,  5,  7,  8, 10, 12, 14, 18, 20,
		22,  24, 25, 30, 31, 34, 37, 38, 39, 40,
		41, 42, 43, 45, 46, 48, 49, 50, 53, 55,
		57, 58, 60, 61, 63, 64, 67, 70, 72, 75,
   	        78, 79, 80, 82, 85, 86, 88, 90, 92, 93,
		
		95, 99,100,102,103,104,106,107,109,110,
		112,114,116,118,121,122,123,124,128,130,
		132,134,136,138,141,142,143,144,148,149,

		150,152,156,158,159,160,163,164,165,167,
		168,169,170,171,172,173,175,177,178,179,

		180,182,186,188,189,190,193,194,195,197,
		198,199,200,201,202,203,205,207,208,209,
		212,214,216,218,221,222,223,224,228,230,
		232,234,236,238,241,242,243,244,248,249,
		250,252,256,258,259,260,263,264,265,267,
		
		268,269,270,271,272,273,275,277,278,279,
		280,282,286,288,289,290,293,294,295,297,
		312,314,316,318,321,322,323,324,328,330,
		332,334,336,338,341,342,343,344,348,349,
		350,352,356,358,359,360,363,364,365,367,

		368,369,370,371,372,373,375,377,378,379,
		380,382,386,388,389,390,393,394,395,397,
		400,401,402,403,405,407,408,409,412,414,
		416,418,421,422,423,424,428,430,432,434,
		436,438,441,442,443,444,448,449,450,452
	}; 

	vector<double> centers{30, 120, 220, 300, 390};
	vector<double> weights{2.0, 0.9, 1.3, 1.1, 0.9};

	vector<double> expect{34.34, 105.31, 214.627, 278.295, 383.15};

	double k = 20;
	for (int i = 0; i < 250; ++i) {
		sh[i].first = x[i];
		for (int j = 0; j < 5; ++j) {
			double v = gauss(x[i], centers[j], k) * weights[j];
			sh[i].second += v; 
		}
		cvdebug() << "h[" << sh[i].first << "]=" << sh[i].second << "\n"; 
	}

	CMeans::PInitializer cci(new CTestFixedInitializer({0, 0.25, 0.5, 0.75, 1}));
	CMeans cm(0.01, 0.0001, cci);


	CMeans::DVector result_cci(5); 
	
	auto sp = cm.run(sh,  result_cci);

	
	for (int i = 0; i < 5; ++i) {
		BOOST_CHECK_CLOSE(result_cci[i], expect[i], 0.1); 
	}
        
}

