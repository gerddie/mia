/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
		0, 0, 1, 1, 
		0, 0, 1, 1, 
		2, 2, 3, 3, 
		2, 2, 3, 3, 
		
		0, 0, 1, 1, 
		0, 0, 1, 1, 
		2, 2, 3, 3, 
		2, 2, 3, 3, 

		4, 4, 5, 5, 
		4, 4, 5, 5, 
		6, 6, 7, 7, 
		6, 6, 7, 7, 

		4, 4, 5, 5, 
		4, 4, 5, 5, 
		6, 6, 7, 7, 
		6, 6, 7, 7
	};

	C3DSSImage fimage(C3DBounds(8, 8, 8), init );
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));


	auto scaler = BOOST_TEST_create_from_plugin<C3DScaleFilterPlugin>("scale:sx=4,sy=4,sz=4,interp=[bspline:d=0]"); 

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


/**
   This test only checks that the scaling didn't change.  
   The values are not really tested as they should be, they are 
   taken from a output that is believed to be working correct 
   because the 1D scaler works correctly.
*/
BOOST_AUTO_TEST_CASE( test_downscale_float_persists )
{
	
	const float test_float[64] = {
		1.21431e-17, 0.123207, 0.876793, 1, 
		0.246414,    0.369621, 1.12321,  1.24641, 
		1.75359,     1.87679,  2.63038,  2.75359, 
		2,           2.12321,  2.87679,  3, 

		0.492827,    0.616034, 1.36962,  1.49283, 
		0.739241,    0.862448, 1.61603,  1.73924, 
		2.24641,     2.36962,  3.12321,  3.24641, 
		2.49283,     2.61603,  3.36962,  3.49283, 

		3.50717,     3.63038,  4.38397,  4.50717, 
		3.75359,     3.87679,  4.63038,  4.75359, 
		5.26076,     5.38397,  6.13755,  6.26076, 
		5.50717,     5.63038,  6.38397,  6.50717, 

		4,           4.12321,  4.87679,  5, 
		4.24641,     4.36962,  5.12321,  5.24641, 
		5.75359,     5.87679,  6.63038,  6.75359, 
		6,           6.12321,  6.87679,  7
	};
	

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



static vector<double> fill_sin(double start, double end, int len)
{
	double delta = (end - start) / (len - 1); 
	vector<double> result(len); 
	for (int i = 0; i < len; ++i) 
		result[i] = sin(delta * i + start); 
	return result; 
}

BOOST_AUTO_TEST_CASE( test_isoscale_float )
{
	const double start = M_PI/10.0; 
	const double end = 9.0 * M_PI/10.0; 

	C3DBounds in_size(129, 177, 257); 
	C3DFImage fimage( in_size);

	{
		vector<double> fz = fill_sin(start, end, in_size.z); 
		vector<double> fy = fill_sin(start, end, in_size.y); 
		vector<double> fx = fill_sin(start, end, in_size.x); 
		
		auto i = fimage.begin(); 
		for (unsigned z = 0; z < in_size.z; ++z)
			for (unsigned y = 0; y < in_size.y; ++y)
				for (unsigned x = 0; x < in_size.x; ++x, ++i)
					*i = 200 * fx[x] * fy[y] *fz[z]; 
	}

	fimage.set_voxel_size(C3DFVector(.5, .25, .125));

	auto isofy = BOOST_TEST_create_from_plugin<CIsoVoxelFilterPlugin>("isovoxel:size=1,interp=[bspline:d=3]"); 

	P3DImage scaled = isofy->filter(fimage);
	
	C3DBounds test_size(65, 45, 33);

	BOOST_CHECK_EQUAL(scaled->get_size(), test_size);

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.f, 1.f, 1.f));

	{
		vector<double> fz = fill_sin(start, end, test_size.z); 
		vector<double> fy = fill_sin(start, end, test_size.y); 
		vector<double> fx = fill_sin(start, end, test_size.x); 

		// the outer boundra is not tested because its interpolation is 
		// dependend on the boundary conditions and as a general rule 
		// the approximation does  not correspond to what is expected analytically 
		// 
		for (unsigned z = 1; z < test_size.z - 1; ++z)
			for (unsigned y = 1; y < test_size.y- 1; ++y) {
				auto k = fscaled.begin_at(1,y,z); 
				for (unsigned x = 1; x < test_size.x- 1; ++x, ++k) {
					const double v = 200 * fx[x] * fy[y] *fz[z]; 
					BOOST_CHECK_CLOSE(*k, v, 0.1); 
				}
			}
	}
}
