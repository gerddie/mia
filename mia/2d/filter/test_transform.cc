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

#include <mia/core/datapool.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/inittesthandlers.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/filter/transform.hh>


using namespace transform_2dimage_filter; 
using namespace mia; 

PrepareTestPluginPath plugin_path_init; 

struct TransformFixture {

	TransformFixture();
	P2DImage src; 
	C2DBounds size;

	static const float src_image_init[]; 
	static const float test_image_init[]; 

}; 



BOOST_FIXTURE_TEST_CASE( test_2dfilter_transform, TransformFixture )
{
	auto t = BOOST_TEST_create_from_plugin<C2DTransformFilterPluginFactory>("transform:file=transform.@");
	auto transformed = t->filter(src); 

	const C2DFImage& r = dynamic_cast<const C2DFImage&>(*transformed); 
	
	BOOST_REQUIRE(r.get_size() == size); 
	const float *itest = test_image_init; 
	auto ir = r.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++ir, ++itest)
			BOOST_CHECK_EQUAL(*ir, *itest); 

}

TransformFixture::TransformFixture():
	size(10,9)
{
		
	src.reset((new C2DFImage(size, src_image_init))); 
        auto transform_factory = produce_2dtransform_factory("translate:imgkernel=[bspline:d=0]"); 
	auto transform = transform_factory->create(size); 
	auto params = transform->get_parameters(); 
	params[0] =  1; 
	params[1] =  2; 
	transform->set_parameters(params); 
	save_transform("transform.@", *transform); 
}
	

const float TransformFixture::src_image_init[10 * 9] = {
	
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,10,30,30, 0, 0, 0,
	0, 0, 0, 0,50,50,50, 0, 0, 0,
	0, 0, 0, 0,50,50,50, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const float TransformFixture::test_image_init[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,10,30,30, 0, 0,
	0, 0, 0, 0, 0,50,50,50, 0, 0,
	0, 0, 0, 0, 0,50,50,50, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
