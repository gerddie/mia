/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#define VSTREAM_DOMAIN "test-maskedimage" 
#include <mia/2d/fullcost/maskedimage.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_imagefullcost_src_mask)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,  
		0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 0, 0, 0, 
		0, 0, 0, 0,   0, 0, 0, 0,
		
 		0, 0, 0,  0,   0,255,255, 0,  
		0,255,255,0,   0, 128, 0, 0, 
		0, 255, 0, 0,  0, 128,  0, 0,  
		0, 0,  0, 0,   0, 0, 0, 0

	};
	const unsigned char ref_data[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	const bool src_mask_data[64] = {
		0, 0, 0, 0,   0,  0,  0, 0,  0, 0, 0, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,  0,  0, 0,  0, 0, 0, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,  1,  1, 0,  0, 1, 1, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,  0,  0, 0,  0, 0, 0, 0,   0, 0, 0, 0

	};

	C2DBounds size(8,8); 

	P2DImage src(new C2DUBImage(size, src_data ));
	P2DImage ref(new C2DUBImage(size, ref_data ));

	P2DImage src_mask(new C2DBitImage(size, src_mask_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 
	BOOST_REQUIRE(save_image("src-mask.@", src_mask)); 

	C2DMaskedImageFullCost cost("src.@", "ref.@","src-mask.@", "" ,
                                    C2DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

	cvdebug() << "prepare cost\n"; 
	cost.reinit(); 
	cvdebug() << "set size cost\n"; 
	cost.set_size(size);
	
	auto tfactory = produce_2dtransform_factory("vf"); 
	auto t = tfactory->create(size); 
	auto tp = t->get_parameters(); 
	std::fill(tp.begin(), tp.end(),0.0); 
	t->set_parameters(tp); 
	
	CDoubleVector gradient(t->degrees_of_freedom()); 
	double cost_value = cost.evaluate(*t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 2u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0/16.0 , 0.1);

	double value = cost.cost_value(*t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[74], -255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[75], -255.0 , 0.1);
	cvdebug() << gradient << "\n"; 	
}

BOOST_AUTO_TEST_CASE( test_imagefullcost_ref_mask)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,  
		0, 0, 0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0, 0, 0, 0, 
		0, 0, 0, 0,   0, 0, 0, 0,
		
 		0, 0, 0,  0,   0,255,255, 0,  
		0,255,255,0,   0, 128, 0, 0, 
		0, 255, 0, 0,  0,128,  0, 0,  
		0, 0,  0, 0,   0, 0, 0, 0

	};
	const unsigned char ref_data[64] = {
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
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};

	const bool src_mask_data[64] = {
		0, 0, 0, 0,   0, 0, 0, 0,   
		0, 0, 0, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,   
		0, 0, 0, 0,   0, 0, 0, 0,
		
		0, 0, 0, 0,   0, 1, 1, 0,   
		0, 1, 1, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0, 0, 0, 0,   
		0, 0, 0, 0,   0, 0, 0, 0

	};

	C2DBounds size(8,8); 

	P2DImage src(new C2DUBImage(size, src_data ));
	P2DImage ref(new C2DUBImage(size, ref_data ));

	P2DImage src_mask(new C2DBitImage(size, src_mask_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 
	BOOST_REQUIRE(save_image("ref-mask.@", src_mask)); 

        assert("at least one mask must be provided"); 
	C2DMaskedImageFullCost cost("src.@", "ref.@","", "ref-mask.@",
                                    C2DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

	cvdebug() << "prepare cost\n"; 
	cost.reinit(); 
	cvdebug() << "set size cost\n"; 
	cost.set_size(size);
	
	auto tfactory = produce_2dtransform_factory("vf"); 
	auto t = tfactory->create(size); 
	auto tp = t->get_parameters(); 
	std::fill(tp.begin(), tp.end(),0.0); 
	t->set_parameters(tp); 

	
	CDoubleVector gradient(t->degrees_of_freedom()); 
	double cost_value = cost.evaluate(*t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 2u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0/16.0 , 0.1);

	double value = cost.cost_value(*t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[74], -255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[75], -255.0 , 0.1);

	cvdebug() << gradient << "\n"; 
}
