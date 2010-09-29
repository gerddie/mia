/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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


#include <mia/2d/fullcost/image.hh>
#include <mia/2d/transformmock.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;

struct ImagefullcostFixture {
	ImagefullcostFixture(); 
	
}; 

BOOST_FIXTURE_TEST_CASE( test_imagefullcost,  ImagefullcostFixture ) 
{

	// create two images 
	const float src_data[16] = {
		0, 0, 0, 0,
		0, 3, 1, 0,
		0, 6, 7, 0,
		0, 0, 0, 0
	};
	const float ref_data[16] = {
		0, 0, 0, 0,
		0, 2, 3, 0,
		0, 1, 2, 0,
		0, 0, 0, 0
	};
	C2DBounds size(4,4); 

	P2DImage src(new C2DFImage(size, src_data ));
	P2DImage ref(new C2DFImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image2d("src.@", src)); 
	BOOST_REQUIRE(save_image2d("ref.@", ref)); 

	C2DImageFullCost cost("src.@", "ref.@", "ssd", ip_bspline3, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	C2DTransformMock t(size); 
	
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 55.0, 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 55.0, 0.1);
	
	BOOST_CHECK_CLOSE(gradient[10], 0.5f, 0.1);
	BOOST_CHECK_CLOSE(gradient[11], 3.0f, 0.1);
	
}

BOOST_FIXTURE_TEST_CASE( test_imagefullcost_no_translate,  ImagefullcostFixture ) 
{


	// create two images 
	const float src_data[16] = {
		0, 0, 0, 0,
		0, 3, 1, 0,
		0, 6, 7, 0,
		0, 0, 0, 0
	};
	const float ref_data[16] = {
		0, 0, 0, 0,
		0, 2, 3, 0,
		0, 1, 2, 0,
		0, 0, 0, 0
	};
	C2DBounds size(4,4); 

	P2DImage src(new C2DFImage(size, src_data ));
	P2DImage ref(new C2DFImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image2d("src.@", src)); 
	BOOST_REQUIRE(save_image2d("ref.@", ref)); 

	C2DImageFullCost cost("src.@", "ref.@", "ssd", ip_bspline3, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	double value = cost.cost_value();

	BOOST_CHECK_CLOSE(value, 0.5 * 55.0, 0.1);

}


BOOST_FIXTURE_TEST_CASE( test_imagefullcost_2,  ImagefullcostFixture)
{

	// create two images 
	const unsigned char src_data[16] = {
		0, 0, 0, 0,
		0, 255, 255, 0,
		0, 255, 255, 0,
		0, 0, 0, 0
	};
	const unsigned char ref_data[16] = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	C2DBounds size(4,4); 

	P2DImage src(new C2DUBImage(size, src_data ));
	P2DImage ref(new C2DUBImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image2d("src.@", src)); 
	BOOST_REQUIRE(save_image2d("ref.@", ref)); 

	C2DImageFullCost cost("src.@", "ref.@", "ssd", ip_bspline3, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	
	C2DTransformMock t(size); 
	
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0 * 4.0, 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0 * 4.0, 0.1);
	
	BOOST_CHECK_CLOSE(gradient[10], 255 * 255 * 0.5f, 0.1);
	BOOST_CHECK_CLOSE(gradient[11], 255 * 255 * 0.5f, 0.1);
	
}

ImagefullcostFixture::ImagefullcostFixture()
{
	list< bfs::path> cost_plugpath;
	cost_plugpath.push_back(bfs::path("../cost"));
	C2DImageCostPluginHandler::set_search_path(cost_plugpath);

	list< bfs::path> filter_plugpath;
	filter_plugpath.push_back(bfs::path("../filter"));
	C2DFilterPluginHandler::set_search_path(filter_plugpath);

	list< bfs::path> io_plugpath;
	io_plugpath.push_back(bfs::path("../io"));
	C2DImageIOPluginHandler::set_search_path(io_plugpath);
}
