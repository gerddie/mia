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

#include <mia/3d/fullcost/taggedssd.hh>
#include <mia/3d/transformmock.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dfilter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE; 
NS_USE(taggedssd_3d); 

namespace bfs=::boost::filesystem;


CSplineKernelTestPath splinekernel_init_path; 

BOOST_AUTO_TEST_CASE( test_taggedssd)
{

	// create two images 
	const unsigned char src_data_x[64] = {
		0, 0, 0, 0,   0, 0, 0,  0,   0,  0,  0,  0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 0, 0,  0,   0,  0,  0,  0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,255,255,0,   0,255,255,  0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,255, 0, 0,   0,  0,  0,  0,   0, 0, 0, 0

	};
	const unsigned char ref_data_x[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,      0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,      0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,      0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,      0, 0, 0, 0

	};
	C3DBounds size(4,4,4); 

	P3DImage src(new C3DUBImage(size, src_data_x ));
	P3DImage ref(new C3DUBImage(size, ref_data_x ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 

	C3DTaggedSSDCost cost("src.@", "ref.@",  "src.@",  "ref.@",  "src.@",  "ref.@",  1.0); 

	cost.reinit(); 
	cost.set_size(size);
	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 
	
	const double v = 255.0 * 0.0202146; 

	double test_cost = (0.5 * v * v  * 5.0)/64.0; 

	BOOST_CHECK_CLOSE(cost_value, test_cost , 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, test_cost , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[111], v * v * 0.5f / 64 , 0.1);
	BOOST_CHECK_CLOSE(gradient[112], v * v * 0.5f / 64 , 0.1);
	BOOST_CHECK_CLOSE(gradient[113], v * v * 0.5f / 64 , 0.1);
	
}


BOOST_AUTO_TEST_CASE( test_taggedssd_separate )
{

	// create two images 
	const unsigned char src_data_x[64] = {
		1, 3, 1, 1,   1, 1, 5, 1,   1, 1, 1, 5,   7, 1, 1, 1,
 		1, 5, 1, 1,   1, 1, 7, 1,   1, 1, 1, 3,   9, 1, 1, 1,
 		1, 7, 1, 1,   1, 1, 9, 1,   1, 1, 1, 9,   3, 1, 1, 1,
		1, 9, 1, 1,   1, 1, 3, 1,   1, 1, 1, 7,   5, 1, 1, 1

	};
	const unsigned char ref_data_x[64] = {
		0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1,   1, 0, 0, 0,
		0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1,   1, 0, 0, 0,
		0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1,   1, 0, 0, 0,
		0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1,   1, 0, 0, 0

	};

	const float grad_x[64] = {
		0, 0,-1, 0,   0, 2, 0, 0,   0, 0, 2, 0,  0, -3, 0, 0,
 		0, 0,-2, 0,   0, 3, 0, 0,   0, 0, 1, 0,  0, -4, 0, 0,
 		0, 0,-3, 0,   0, 4, 0, 0,   0, 0, 4, 0,  0, -1, 0, 0,
		0, 0,-4, 0,   0, 1, 0, 0,   0, 0, 3, 0,  0, -2, 0, 0

	};

	const unsigned char src_data_z[64] = {
		2, 4, 6, 8,   0, 0, 0, 0,    0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   6, 8, 4, 2,    0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 0, 0, 0,    4, 2, 6, 8,   0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,   8, 4, 2, 6

	};
	const unsigned char ref_data_z[64] = {
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1

	};
	const float grad_z[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,   0, 0, 0, 0,
 		1, 2, 3, 4,   0, 0, 0, 0,   -2,-1,-3,-4,   0, 0, 0, 0,
 		0, 0, 0, 0,   3, 4, 2, 1,    0, 0, 0, 0,  -4,-2,-1,-3,
		0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,   0, 0, 0, 0
	};
	
	const unsigned char src_data_y[64] = {
		0, 2, 0, 0,   0, 0, 2, 0,   0,  0,  0,  4,   8, 0, 0, 0,
 		0, 4, 0, 0,   0, 0, 4, 0,   0,  0,  0,  6,   6, 0, 0, 0,
 		0, 6, 0, 0,   0, 0, 6, 0,   0,  0,  0,  8,   4, 0, 0, 0,
		0, 8, 0, 0,   0, 0, 8, 0,   0,  0,  0,  2,   2, 0, 0, 0

	};
	const unsigned char ref_data_y[64] = {
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1,
		1, 1, 1, 1,   1, 1, 1, 1,    1, 1, 1, 1,   1, 1, 1, 1
	};
	const float grad_y[64] = {
		0, 0, 0, 0,   0, 1, 0,-2,  -4,  0,  1,  0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 2, 0,-3,  -3,  0,  2,  0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 3, 0,-4,  -2,  0,  3,  0,   0, 0, 0, 0,
		0, 0, 0, 0,   0, 4, 0,-1,  -1,  0,  4,  0,   0, 0, 0, 0
	};

	C3DBounds size(4,4,4); 

	P3DImage srcx(new C3DUBImage(size, src_data_x ));
	P3DImage refx(new C3DUBImage(size, ref_data_x ));
	P3DImage srcy(new C3DUBImage(size, src_data_y ));
	P3DImage refy(new C3DUBImage(size, ref_data_y ));
	P3DImage srcz(new C3DUBImage(size, src_data_z ));
	P3DImage refz(new C3DUBImage(size, ref_data_z ));

	BOOST_REQUIRE(save_image("srcx.@", srcx)); 
	BOOST_REQUIRE(save_image("refx.@", refx)); 
	BOOST_REQUIRE(save_image("srcy.@", srcy)); 
	BOOST_REQUIRE(save_image("refy.@", refy)); 
	BOOST_REQUIRE(save_image("refz.@", refz)); 
	BOOST_REQUIRE(save_image("srcz.@", srcz)); 


	C3DTaggedSSDCost cost("srcx.@", "refx.@",  "srcy.@",  "refy.@",  "srcz.@",  "refz.@",  1.0); 

	cost.reinit(); 
	cost.set_size(size);
	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 

	const double test_cost = (3 * 48 + 8 * ( 1 + 9 + 25 + 49 ) + 
				  4 * ( 4 + 16 + 36 + 64)) / 6.0 / 64.0 * 0.545873 * 0.545873;   

	BOOST_CHECK_CLOSE(cost_value, test_cost , 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, test_cost , 0.1);
	double scale = 0.545873 * 0.545873 / 64.0; 
		
	for (int i = 0; i < 64; ++i) {
		BOOST_CHECK_CLOSE(gradient[3*i  ], grad_x[i] * scale, 0.1);
		BOOST_CHECK_CLOSE(gradient[3*i+1], grad_y[i] * scale, 0.1);
		BOOST_CHECK_CLOSE(gradient[3*i+2], grad_z[i] * scale, 0.1);
	}
}
