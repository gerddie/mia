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

#include <iomanip>
#include <mia/internal/plugintester.hh> 
#include <mia/2d/filter/regiongrow.hh>

using namespace mia;
using namespace regiongrow_2d_filter;

C2DShapePluginHandlerTestPath shape_plugin_path; 

BOOST_AUTO_TEST_CASE ( test_regiongrow ) 
{
	const C2DBounds size(11, 10); 
	

	const int grad_image[110] = {
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   3,   2,   0,   0,   0,   0,   0,  0,  0, 
		0,   9,   7,   6,   5,   7,   6,   0,   0,  0,  0, 
		0,   8,   4,   2,   0,   8,   3,   0,   0,  0,  0, 
		0,   7,   6,   2,   3,   4,   0,   0,   0,  0,  0, 
		0,   4,   3,   4,   3,   3,   8,   0,   0,  0,  0, 
		0,   7,   4,   4,   5,   3,   6,   0,   0,  0,  0, 
		0,   3,   7,   7,   4,   5,   8,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
	}; 

	const bool test_image_n4[110] = {
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   0,   0,   1,   1,   1,   1,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   1,   0,   0,  0,  0, 
		0,   0,   1,   1,   0,   1,   1,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
	}; 

	const bool seed[110] = {
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   1,   1,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   1,   1,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   1,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
	}; 


	const bool test_image_n8[110] = {
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   0,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   1,   0,   0,  0,  0, 
		0,   1,   1,   1,   1,   1,   1,   0,   0,  0,  0, 
		0,   0,   1,   1,   1,   1,   1,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 
	}; 

	P2DImage input_image(new C2DSIImage(size, grad_image));
	P2DImage seed_image(new C2DBitImage(size, seed));
	save_image("seed.@", seed_image); 

	{
		auto f = BOOST_TEST_create_from_plugin<C2DRegiongrowFilterPlugin>("regiongrow:seed=seed.@,n=8n");
		
		auto result = f->filter(*input_image); 
		
		BOOST_CHECK_EQUAL(result->get_size(), size); 
		const C2DBitImage& r = dynamic_cast<const C2DBitImage&>(*result); 
		
		auto ir = r.begin(); 
		auto t = test_image_n8; 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++t, ++ir) {
				BOOST_CHECK_EQUAL(*ir, *t); 
			}
		
		cvdebug() << "result n8\n"; 
		ir = r.begin(); 
		t = test_image_n8; 
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++ir, ++t) {
				if (*ir == *t)
					cverb << " " << *ir << "=" << *t << " "; 
				else 
					cverb << "<" << *ir << "!" << *t << ">"; 
			}
			cverb << "\n"; 
		}
	}

	{
		auto f = BOOST_TEST_create_from_plugin<C2DRegiongrowFilterPlugin>("regiongrow:seed=seed.@,n=4n");
		
		auto result = f->filter(*input_image); 
		
		BOOST_CHECK_EQUAL(result->get_size(), size); 
		const C2DBitImage& r = dynamic_cast<const C2DBitImage&>(*result); 
		
		auto ir = r.begin(); 
		auto t = test_image_n4; 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++t, ++ir) {
				BOOST_CHECK_EQUAL(*ir, *t); 
			}
		
		cvdebug() << "result n4\n"; 
		ir = r.begin(); 
		t = test_image_n4; 
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++ir, ++t) {
				if (*ir == *t)
					cverb << " " << *ir << "=" << *t << " "; 
				else 
					cverb << "<" << *ir << "!" << *t << ">"; 
			}
			cverb << "\n"; 
		}
	}
	
}
 


