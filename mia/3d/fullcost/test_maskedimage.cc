/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/3d/fullcost/maskedimage.hh>
#include <mia/3d/transformmock.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/filter.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem;
using std::invalid_argument;
using std::runtime_error; 

BOOST_AUTO_TEST_CASE( test_imagefullcost_src_mask)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,255,255, 0,  0,255,255, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,255,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0

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
		0, 0, 0, 0, 
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};

	C3DBounds size(4,4,4); 

	P3DImage src(new C3DUBImage(size, src_data ));
	P3DImage ref(new C3DUBImage(size, ref_data ));

	P3DImage src_mask(new C3DBitImage(size, src_mask_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 
	BOOST_REQUIRE(save_image("src-mask.@", src_mask)); 

        assert("at least one mask must be provided"); 
	C3DMaskedImageFullCost cost("src.@", "ref.@","src-mask.@", "" , nullptr, nullptr, 
                                    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

	cvdebug() << "prepare cost\n"; 
	cost.reinit(); 
	cvdebug() << "set size cost\n"; 
	cost.set_size(size);

	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0/16.0 , 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[111], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[112], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[113], 255 *255/128.0 , 0.1);
	
}

BOOST_AUTO_TEST_CASE( test_imagefullcost_ref_mask)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,255,255, 0,  0,255,255, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,255,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0

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
		0, 0, 0, 0, 
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};

	C3DBounds size(4,4,4); 

	P3DImage src(new C3DUBImage(size, src_data ));
	P3DImage ref(new C3DUBImage(size, ref_data ));

	P3DImage src_mask(new C3DBitImage(size, src_mask_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref)); 
	BOOST_REQUIRE(save_image("ref-mask.@", src_mask)); 

        assert("at least one mask must be provided"); 
	C3DMaskedImageFullCost cost("src.@", "ref.@","", "ref-mask.@", nullptr, nullptr, 
                                    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

	cvdebug() << "prepare cost\n"; 
	cost.reinit(); 
	cvdebug() << "set size cost\n"; 
	cost.set_size(size);
	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0/16.0 , 0.1);

	double value = cost.cost_value(t);

	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);
	
	BOOST_CHECK_CLOSE(gradient[111], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[112], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[113], 255 *255/128.0 , 0.1);
	
}


BOOST_AUTO_TEST_CASE( test_imagefullcost_src_ref_mask)
{

	// create two images 
	const unsigned char src_data[64] = {
		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,  0,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0,
 		0, 0, 0, 0,   0,255,255, 0,  0,255,255, 0,   0, 0, 0, 0,
		0, 0, 0, 0,   0,255,  0, 0,  0,  0,  0, 0,   0, 0, 0, 0

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
		0, 0, 0, 0, 
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};

	const bool ref_mask_data[64] = {
		0, 0, 0, 0, 
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,

		0, 0, 0, 0,
		1, 1, 1, 1,
		1, 1, 1, 1,
		0, 0, 0, 0,

		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0

	};

	
	C3DBounds size(4,4,4); 

	P3DImage src(new C3DUBImage(size, src_data ));
	P3DImage ref(new C3DUBImage(size, ref_data ));

	P3DImage src_mask(new C3DBitImage(size, src_mask_data ));
	P3DImage ref_mask(new C3DBitImage(size, ref_mask_data ));
	
	BOOST_REQUIRE(save_image("src.@", src)); 
	BOOST_REQUIRE(save_image("ref.@", ref));
	BOOST_REQUIRE(save_image("src-mask.@", ref_mask)); 
	BOOST_REQUIRE(save_image("ref-mask.@", src_mask)); 

        assert("at least one mask must be provided"); 
	C3DMaskedImageFullCost cost("src.@", "ref.@", "src-mask.@", "ref-mask.@", nullptr, nullptr, 
                                    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

	cvdebug() << "prepare cost\n"; 
	cost.reinit(); 
	cvdebug() << "set size cost\n"; 
	cost.set_size(size);
	
	C3DTransformMock t(size, C3DInterpolatorFactory("bspline:d=3", "mirror")); 
	
	CDoubleVector gradient(t.degrees_of_freedom()); 
	double cost_value = cost.evaluate(t, gradient);
	BOOST_CHECK_EQUAL(gradient.size(), 3u * 64u); 

	BOOST_CHECK_CLOSE(cost_value, 0.5 * 255 * 255.0/16.0 , 0.1);

	double value = cost.cost_value(t);
	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);

	value = cost.cost_value();
	BOOST_CHECK_CLOSE(value, 0.5 * 255 * 255.0/16.0  , 0.1);


	C3DBounds read_size = C3DBounds::_0;

	BOOST_CHECK(cost.get_full_size(read_size));
	BOOST_CHECK_EQUAL(read_size, size);

	C3DBounds read_wrong_size(2,3,4); 
	BOOST_CHECK(!cost.get_full_size(read_wrong_size));
	
	BOOST_CHECK_CLOSE(gradient[111], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[112], 255 *255/128.0 , 0.1);
	BOOST_CHECK_CLOSE(gradient[113], 255 *255/128.0 , 0.1);

	BOOST_CHECK(cost.has(property_gradient));

	BOOST_CHECK(!cost.has("nonexistent-property"));

	cost.set_size(C3DBounds(2,2,2));
}

BOOST_AUTO_TEST_CASE( test_some_expected_failures)
{
	P3DImage src_234(new C3DUBImage(C3DBounds(2,3,4)));
	P3DImage src_345(new C3DUBImage(C3DBounds(3,4,5)));

	P3DImage mask_234(new C3DBitImage(C3DBounds(2,3,4)));
	P3DImage mask_345(new C3DBitImage(C3DBounds(3,4,5)));

	BOOST_REQUIRE(save_image("234.@", src_234)); 
	BOOST_REQUIRE(save_image("345.@", src_345));
	BOOST_REQUIRE(save_image("mask_234.@", mask_234)); 
	BOOST_REQUIRE(save_image("mask_345.@", mask_345)); 

	{
		// at least one mask should be given
		BOOST_CHECK_THROW(C3DMaskedImageFullCost cost("234.@", "234.@", "", "", nullptr, nullptr, 
			      C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0), invalid_argument);
	}

	{
		C3DMaskedImageFullCost cost("234.@", "234.@", "234.@", "", nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

		// mask should be binary 
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument);  
	}

	{
		C3DMaskedImageFullCost cost("234.@", "234.@", "", "234.@",  nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

		// mask should be binary 
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument);  
	}

	{
		C3DMaskedImageFullCost cost("234.@", "345.@", "", "mask_234.@",  nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 

		// images must be of same size 
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument);  
	}

	{
		C3DMaskedImageFullCost cost("234.@", "234.@", "", "mask_345.@",  nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 
		
		// mask and image must be of same size 
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument);  
	}
	{
		C3DMaskedImageFullCost cost("234.@", "234.@", "mask_345.@", "", nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0); 
		
		// mask and image must be of same size 
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument);  
	}
	
	{
		// reference must be given 
		BOOST_CHECK_THROW(C3DMaskedImageFullCost cost("234.@", "", "", "mask_234.@",  nullptr, nullptr, 
				   C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0), runtime_error);
	}

	{
		// source must be given 
		BOOST_CHECK_THROW(C3DMaskedImageFullCost cost("", "234.@", "", "mask_234.@",  nullptr, nullptr, 
				      C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0), runtime_error); 
	}

	{
		// reference must exists at reinit()
		C3DMaskedImageFullCost cost("234.@", "nonex.@", "", "mask_234.@",  nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0);
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument); 
	}

	{
		// source must exist at reinit()
		C3DMaskedImageFullCost cost("nonex.@", "234.@", "", "mask_234.@",  nullptr, nullptr, 
					    C3DMaskedImageCostPluginHandler::instance().produce("ssd"), 1.0);
		BOOST_CHECK_THROW(cost.reinit(), invalid_argument); 
	}

	
}
