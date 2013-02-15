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

#include <mia/internal/plugintester.hh>
#include <mia/3d/filter/scale.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace scale_3dimage_filter;

CSplineKernelTestPath splinekernel_init_path; 

BOOST_AUTO_TEST_CASE( test_downscale )
{

	const short init[8*64] = {
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 


		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		0, 0, 0, 0, 1, 1, 1, 1,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		2, 2, 2, 2, 3, 3, 3, 3,/**/ 
		
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 

		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		4, 4, 4, 4, 5, 5, 5, 5,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7,/**/ 
		6, 6, 6, 6, 7, 7, 7, 7/**/ 

	};

	const short test[64] = {
		0, 0, 1, 1, 0, 0, 1, 1, 
		2, 2, 3, 3, 2, 2, 3, 3, 
		0, 0, 1, 1, 0, 0, 1, 1, 
		2, 2, 3, 3, 2, 2, 3, 3, 

		4, 4, 5, 5, 4, 4, 5, 5, 
		6, 6, 7, 7, 6, 6, 7, 7, 
		4, 4, 5, 5, 4, 4, 5, 5, 
		6, 6, 7, 7, 6, 6, 7, 7
	};

	C3DSSImage fimage(C3DBounds(8, 8, 8), init );
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));


	auto scaler = BOOST_TEST_create_from_plugin<C3DScaleFilterPlugin>("scale:sx=4,sy=4,sz=4,interp=[bspline:d=3]"); 

	P3DImage scaled = scaler->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(4, 4, 4));

	const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.0f, 8.0f));

	for (size_t i = 0; i < 64; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], test[i]); 
	}
		

}

extern const short init_short[]; 
extern const float test_float[]; 

BOOST_AUTO_TEST_CASE( test_downscale_float )
{


	C3DFImage fimage(C3DBounds(8, 8, 8));
	copy(init_short, init_short + 8*8*8, fimage.begin()); 
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));

	auto scaler = BOOST_TEST_create_from_plugin<C3DScaleFilterPlugin>("scale:sx=4,sy=4,sz=4,interp=[bspline:d=3]"); 
	P3DImage scaled = scaler->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(4, 4, 4));

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.0f, 8.0f));

	for (size_t i = 0; i < 64; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test_float[i] << '\n'; 
		BOOST_CHECK_CLOSE(fscaled[i], test_float[i], 0.1); 
	}
		

}

BOOST_AUTO_TEST_CASE( test_noscale_float )
{

	C3DFImage fimage(C3DBounds(8, 8, 8) );
	copy(init_short, init_short + 8*8*8, fimage.begin()); 
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));


	auto scaler = BOOST_TEST_create_from_plugin<C3DScaleFilterPlugin>("scale:sx=0,sy=0,sz=0,interp=[bspline:d=3]"); 
	P3DImage scaled = scaler->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(8, 8, 8));

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(2.0f, 3.0f, 4.0f));

	for (size_t i = 0; i < 512; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << init_short[i] << '\n'; 
		BOOST_CHECK_CLOSE(fscaled[i], init_short[i], 0.1); 
	}
}


const short init_short[8*64] = {
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	
	
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	0, 0, 0, 0, 1, 1, 1, 1,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	2, 2, 2, 2, 3, 3, 3, 3,/**/ 
	
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	4, 4, 4, 4, 5, 5, 5, 5,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7,/**/ 
	6, 6, 6, 6, 7, 7, 7, 7/**/ 
	
};

// this data is the actual outcome of the downscaling 
// so it's no real test 
const float test_float[64] = {
	0.38376388,   0.284638166, 1.37324274, 1.27411711,
	0.185512438,  0.0863867104,1.17499137, 1.07586563,
	2.36272168,   2.26359606,  3.35220051, 3.25307488,
	2.1644702,    2.06534457,  3.15394902, 3.0548234,
	-0.0127390167,-0.111864746, 0.976739883,0.877614141,
	-0.210990474, -0.310116202, 0.778488457,0.679362714,
	1.96621883,   1.86709309,  2.95569777, 2.85657191,
	1.76796734,   1.6688416,   2.75744629, 2.65832043,
	4.34167957,   4.24255371,  5.33115816, 5.23203278,
	4.14342785,   4.04430246,  5.13290691, 5.03378105,
	6.32063723,   6.22151136,  7.31011629, 7.21099043,
	6.12238598,   6.02326012,  7.11186457, 7.01273918,
	3.9451766,    3.84605098,  4.93465567, 4.8355298,
	3.74692512,   3.64779949,  4.73640394, 4.63727808,
	5.92413425,   5.82500887,  6.91361332, 6.81448746,
	5.72588301,   5.62675714,  6.71536207, 6.61623621
};


static vector<double> fill_sin(int len)
{
	vector<double> result(len); 
	for (int i = 0; i < len; ++i) 
		result[i] = sin(2 * M_PI * i / (len -1)); 
	return result; 
}

BOOST_AUTO_TEST_CASE( test_isoscale_float )
{
	C3DBounds in_size(129, 177, 257); 
	C3DFImage fimage( in_size);

	{
		vector<double> fz = fill_sin(in_size.z); 
		vector<double> fy = fill_sin(in_size.y); 
		vector<double> fx = fill_sin(in_size.x); 
		
		auto i = fimage.begin(); 
		for (unsigned z = 0; z < in_size.z; ++z)
			for (unsigned y = 0; y < in_size.y; ++y)
				for (unsigned x = 0; x < in_size.x; ++x, ++i)
					*i = 200 * fx[x] * fy[y] *fz[z]; 
	}

	fimage.set_voxel_size(C3DFVector(.5, .25, .125));

	auto isofy = BOOST_TEST_create_from_plugin<CIsoVoxelFilterPlugin>("isovoxel:size=1"); 

	P3DImage scaled = isofy->filter(fimage);
	
	C3DBounds test_size(65, 45, 33);

	BOOST_CHECK_EQUAL(scaled->get_size(), test_size);

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.f, 1.f, 1.f));

	{
		vector<double> fz = fill_sin(test_size.z); 
		vector<double> fy = fill_sin(test_size.y); 
		vector<double> fx = fill_sin(test_size.x); 

		auto k = fscaled.begin(); 
		for (unsigned z = 0; z < test_size.z; ++z)
			for (unsigned y = 0; y < test_size.y; ++y)
				for (unsigned x = 0; x < test_size.x; ++x, ++k) {
					const double v = 200 * fx[x] * fy[y] *fz[z]; 
					BOOST_CHECK_CLOSE(v, *k, 0.1); 
				}
	}
}
