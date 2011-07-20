/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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


#include <mia/3d/fullcost/image.hh>
#include <mia/3d/transformmock.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dfilter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;

struct ImagefullcostFixture {
	ImagefullcostFixture(); 
	
}; 

CSplineKernelTestPath splinekernel_init_path; 

BOOST_FIXTURE_TEST_CASE( test_imagefullcost_2,  ImagefullcostFixture)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,  0,255,255,0,  0,255,255,0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,255, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0

	};
	const unsigned char ref_data[64] = {
		0, 0, 0, 0,  0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};
	C3DBounds size(4,4,4); 

	P3DImage src(new C3DUBImage(size, src_data ));
	P3DImage ref(new C3DUBImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 

	C3DImageFullCost cost("src.@", "ref.@", "ssd", 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0 * 5.0 , 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0 * 5.0 , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[111], 255 * 255 * 0.5f , 0.1);
	BOOST_CHECK_CLOSE(gradient[112], 255 * 255 * 0.5f , 0.1);
	BOOST_CHECK_CLOSE(gradient[113], 255 * 255 * 0.5f , 0.1);
	
}

ImagefullcostFixture::ImagefullcostFixture()
{
	list< bfs::path> cost_plugpath;
	cost_plugpath.push_back(bfs::path("../cost"));
	C3DImageCostPluginHandler::set_search_path(cost_plugpath);

	list< bfs::path> filter_plugpath;
	filter_plugpath.push_back(bfs::path("../filter"));
	C3DFilterPluginHandler::set_search_path(filter_plugpath);

	list< bfs::path> io_plugpath;
	io_plugpath.push_back(bfs::path("../io"));
	C3DImageIOPluginHandler::set_search_path(io_plugpath);
}
