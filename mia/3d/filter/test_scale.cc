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


	CScale scaler(C3DBounds(4,4,4), "bspline:d=3");

	P3DImage scaled = scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(4, 4, 4));

	const C3DSSImage& fscaled = dynamic_cast<const C3DSSImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.0f, 1.5f, 2.0f));

	for (size_t i = 0; i < 64; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], test[i]); 
	}
		

}

extern const float init_float[]; 
extern const float test_float[]; 

BOOST_AUTO_TEST_CASE( test_downscale_float )
{


	C3DFImage fimage(C3DBounds(8, 8, 8), init_float );
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));


	CScale scaler(C3DBounds(4, 4, 4), "bspline:d=3");

	P3DImage scaled = scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(4, 4, 4));

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.0f, 1.5f, 2.0f));

	for (size_t i = 0; i < 64; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test_float[i] << '\n'; 
		BOOST_CHECK_CLOSE(fscaled[i], test_float[i], 0.1); 
	}
		

}

BOOST_AUTO_TEST_CASE( test_noscale_float )
{

	C3DFImage fimage(C3DBounds(8, 8, 8), init_float );
	fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));


	CScale scaler(C3DBounds(0, 0, 0), "bspline:d=3");

	P3DImage scaled = scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C3DBounds(8, 8, 8));

	const auto fscaled = dynamic_cast<const C3DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(2.0f, 3.0f, 4.0f));

	for (size_t i = 0; i < 512; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << init_float[i] << '\n'; 
		BOOST_CHECK_CLOSE(fscaled[i], init_float[i], 0.1); 
	}
}


const float init_float[8*64] = {
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
