/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/2d/fullcost/image.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;

struct ImagefullcostFixture {
	ImagefullcostFixture(); 

	P2DInterpolatorFactory ipf; 
	P2DTransformationFactory tff; 
}; 


CSplineKernelTestPath init_splinekernel_path; 

struct InitSplinekernelTestPath {
	InitSplinekernelTestPath() {
		CPathNameArray cost_plugpath;
		cost_plugpath.push_back(bfs::path("../cost"));
		C2DImageCostPluginHandler::set_search_path(cost_plugpath);
		
		CPathNameArray filter_plugpath;
		filter_plugpath.push_back(bfs::path("../filter"));
		C2DFilterPluginHandler::set_search_path(filter_plugpath);
		
		CPathNameArray io_plugpath;
		io_plugpath.push_back(bfs::path("../io"));
		C2DImageIOPluginHandler::set_search_path(io_plugpath);
		
	}
}; 

C2DTransformCreatorHandlerTestPath tfc_test_path; 

InitSplinekernelTestPath init_path; 

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

	auto t = tff->create(size); 
	auto params = t->get_parameters(); 
	std::fill(params.begin(), params.end(), 0.0); 
	t->set_parameters(params); 

	P2DImage src(new C2DFImage(size, src_data ));
	P2DImage ref(new C2DFImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 
	auto cost_kernel = C2DImageCostPluginHandler::instance().produce("ssd"); 

	C2DImageFullCost cost("src.@", "ref.@", cost_kernel, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	
	CDoubleVector gradient(t->degrees_of_freedom()); 
	double cost_value = cost.evaluate(*t, gradient);

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 55.0, 0.1);

	double value = cost.cost_value(*t);

	BOOST_CHECK_CLOSE(value, 0.5 * 55.0, 0.1);
	
	BOOST_CHECK_CLOSE(gradient[10], -0.5f, 0.1);
	BOOST_CHECK_CLOSE(gradient[11], -3.0f, 0.1);
	
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
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 

	auto cost_kernel = C2DImageCostPluginHandler::instance().produce("ssd"); 
	C2DImageFullCost cost("src.@", "ref.@", cost_kernel, 1.0, false); 
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

	auto t = tff->create(size); 
	auto params = t->get_parameters(); 
	std::fill(params.begin(), params.end(), 0.0); 
	t->set_parameters(params); 

	P2DImage src(new C2DUBImage(size, src_data ));
	P2DImage ref(new C2DUBImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 

	P2DInterpolatorFactory ipf(new C2DInterpolatorFactory(produce_spline_kernel("bspline:d=3"), 
							      *produce_spline_boundary_condition("mirror"), 
							      *produce_spline_boundary_condition("mirror")));  
	auto cost_kernel = C2DImageCostPluginHandler::instance().produce("ssd"); 
	C2DImageFullCost cost("src.@", "ref.@", cost_kernel, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	
	
	CDoubleVector gradient(t->degrees_of_freedom()); 
	double cost_value = cost.evaluate(*t, gradient);

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0 * 4.0, 0.1);

	double value = cost.cost_value(*t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0 * 4.0, 0.1);
	
	BOOST_CHECK_CLOSE(gradient[10], -255 * 255 * 0.5f, 0.1);
	BOOST_CHECK_CLOSE(gradient[11], -255 * 255 * 0.5f, 0.1);
	
}

#if NEEDS_REVIEW
BOOST_FIXTURE_TEST_CASE( test_imagefullcost_2_scaled,  ImagefullcostFixture)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0,   0,   0,   0,   0, 0, 0,
		0, 0,   0,   0,   0,   0, 0, 0,
		0, 0, 255, 255, 255, 255, 0, 0,
		0, 0, 255, 255, 255, 255, 0, 0,
		0, 0, 255, 255, 255, 255, 0, 0,
		0, 0, 255, 255, 255, 255, 0, 0,
		0, 0,   0,   0,   0,   0, 0, 0,
		0, 0,   0,   0,   0,   0, 0, 0
	};
	const unsigned char ref_data[64] = {
		0, 0,   0,   0,   0,   0,   0, 0,
		0, 0,   0,   0,   0,   0,   0, 0,
		0, 0, 100, 100, 100, 100,   0, 0,
		0, 0, 100, 100, 100, 100,   0, 0,
		0, 0, 100, 100, 100, 100,   0, 0,
		0, 0, 100, 100, 100, 100,   0, 0,
		0, 0,   0,   0,   0,   0,   0, 0,
		0, 0,   0,   0,   0,   0,   0, 0
	};
	C2DBounds size(8,8); 

	P2DImage src(new C2DUBImage(size, src_data ));
	P2DImage ref(new C2DUBImage(size, ref_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 

	C2DImageFullCost cost("src.@", "ref.@", "ssd", ipf, 1.0, false); 
	cost.reinit(); 
	cost.set_size(size);
	
	C2DTransformMock t(size); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 155 * 155.0 * 16.0, 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 155 * 155.0 * 16.0, 0.1);
	
	C2DBounds rsize(4,4);
	cost.set_size(rsize);
	C2DTransformMock t2(rsize); 
	CDoubleVector gradient2(t2.degrees_of_freedom()); 
	double cost_value2 = cost.evaluate(t2, gradient2);
	BOOST_CHECK_CLOSE(cost_value2, 0.5 * 155 * 155.0 * 4.0, 0.1);
	BOOST_CHECK_CLOSE(cost.cost_value(t), 0.5 * 155 * 155.0 * 4.0, 0.1);

	BOOST_CHECK_CLOSE(gradient2[10], 155 * 255 * 0.5f, 0.1);
	BOOST_CHECK_CLOSE(gradient2[11], 155 * 255 * 0.5f, 0.1);


}
#endif

ImagefullcostFixture::ImagefullcostFixture():
	ipf(new C2DInterpolatorFactory(produce_spline_kernel("bspline:d=3"), 
				       *produce_spline_boundary_condition("mirror"), 
				       *produce_spline_boundary_condition("mirror"))), 
	tff(C2DTransformCreatorHandler::instance().produce("vf"))
{
}


