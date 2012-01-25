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

#include <iomanip>
#include <mia/internal/plugintester.hh> 
#include <mia/3d/filter/seededwatershed.hh>


using namespace mia; 

C3DFilterPluginHandlerTestPath filter_test_path; 
C3DShapePluginHandlerTestPath shape_test_path; 

const unsigned char  seed[3*72] = {
	1, 0, 0, 0, 0, 0, 0, 0, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 3, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,

	1, 0, 0, 0, 0, 0, 0, 0, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 3, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,

	1, 0, 0, 0, 0, 0, 0, 0, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 3, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,

}; 


BOOST_AUTO_TEST_CASE ( test_seeded_watershead ) 
{
	const C3DBounds size(9,8,3); 
	
	const int image[3*72] = {
		1,   1,   1,   3,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   4,   4,   4,   4,
		0,   0,   0,   0,   1,   2,   5,   2,   2,
		1,   0,   0,   0,   1,   2,   5,   2,   2,
		0,   0,   0,   0,   2,   3,   3,   3,   2,
		0,   0,   0,   0,   3,   3,   3,   3,   3,
		0,   0,   0,   0,   3,   3,   3,   3,   3,

		1,   1,   1,   3,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   4,   4,   4,   4,
		0,   0,   0,   0,   1,   2,   5,   2,   2,
		1,   0,   0,   0,   1,   2,   5,   2,   2,
		0,   0,   0,   0,   2,   3,   3,   3,   2,
		0,   0,   0,   0,   3,   3,   3,   3,   3,
		0,   0,   0,   0,   3,   3,   3,   3,   3,

		1,   1,   1,   3,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   5,   5,   5,   5,
		0,   0,   0,   0,   4,   4,   4,   4,   4,
		0,   0,   0,   0,   1,   2,   5,   2,   2,
		1,   0,   0,   0,   1,   2,   5,   2,   2,
		0,   0,   0,   0,   2,   3,   3,   3,   2,
		0,   0,   0,   0,   3,   3,   3,   3,   3,
		0,   0,   0,   0,   3,   3,   3,   3,   3,

	}; 

	const unsigned char  test[3*72] = {
		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

	}; 

	P3DImage seed_image(new C3DUBImage(size, seed)); 
	P3DImage input_image(new C3DSIImage(size, image));
	
	
	auto f = BOOST_TEST_create_from_plugin<C3DSeededWSFilterPlugin>("sws:seed=seed.@,n=6n,mark=1"); 
	save_image("seed.@", seed_image); 
	
	auto result = f->filter(*input_image); 

	BOOST_CHECK_EQUAL(result->get_size(), size); 
	const C3DUBImage& r = dynamic_cast<const C3DUBImage&>(*result); 
	
	auto ir = r.begin(); 
	auto t = test; 
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++t, ++ir) {
			BOOST_CHECK_EQUAL(*ir, *t); 
		}
	
	cvdebug() << "result \n"; 
	ir = r.begin(); 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++ir) {
				cverb <<setw(3) << (int)*ir << ", "; 
			}
			cverb << "\n"; 
		}
	}
	
	cvdebug() << "test \n"; 
	t = test; 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++t) {
				cverb <<setw(8) << (int)*t << ", "; 
			}
			cverb << "\n"; 
		}
	}

	P3DImage grad = run_filter(*input_image, "gradnorm");
	const C3DFImage& fgrad = dynamic_cast<const C3DFImage&>(*grad); 
	
	cvdebug() << "grad \n"; 
	auto g = fgrad.begin(); 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++g) {
				cverb <<setw(8) << *g << ", "; 
			}
			cverb << "\n"; 
		}
	}


}
 

BOOST_AUTO_TEST_CASE ( test_seeded_watershead_from_grad ) 
{
	const C3DBounds size(9,8,3); 
	const float image[72*3] = {
		0.0,        0,      0.4,      0.6,      0.4,      0.2,        0,        0,        0, 
		0.2,      0.2,      0.2,        1,        1, 0.282843,      0.2,      0.2,      0.2, 
		0.0,        0,        0,      0.8,        1,      0.6,        0,      0.6,      0.6, 
		0.2,        0,        0,      0.2,  0.72111, 0.894427,      0.2,  0.72111,      0.4, 
		0.,       0.2,        0,      0.2, 0.447214, 0.824621,      0.4, 0.632456,        0, 
		0.2,        0,        0,      0.4,  0.72111, 0.282843,      0.4, 0.282843,      0.2, 
		0.0,        0,        0,      0.6, 0.632456,        0,        0,        0,      0.2, 
		0.0,        0,        0,      0.6,      0.6,        0,        0,        0,        0, 

		0.0,        0,      0.4,      0.6,      0.4,      0.2,        0,        0,        0, 
		0.2,      0.2,      0.2,        1,        1, 0.282843,      0.2,      0.2,      0.2, 
		0.0,        0,        0,      0.8,        1,      0.6,        0,      0.6,      0.6, 
		0.2,        0,        0,      0.2,  0.72111, 0.894427,      0.2,  0.72111,      0.4, 
		0.,       0.2,        0,      0.2, 0.447214, 0.824621,      0.4, 0.632456,        0, 
		0.2,        0,        0,      0.4,  0.72111, 0.282843,      0.4, 0.282843,      0.2, 
		0.0,        0,        0,      0.6, 0.632456,        0,        0,        0,      0.2, 
		0.0,        0,        0,      0.6,      0.6,        0,        0,        0,        0, 
		
		0.0,        0,      0.4,      0.6,      0.4,      0.2,        0,        0,        0, 
		0.2,      0.2,      0.2,        1,        1, 0.282843,      0.2,      0.2,      0.2, 
		0.0,        0,        0,      0.8,        1,      0.6,        0,      0.6,      0.6, 
		0.2,        0,        0,      0.2,  0.72111, 0.894427,      0.2,  0.72111,      0.4, 
		0.,       0.2,        0,      0.2, 0.447214, 0.824621,      0.4, 0.632456,        0, 
		0.2,        0,        0,      0.4,  0.72111, 0.282843,      0.4, 0.282843,      0.2, 
		0.0,        0,        0,      0.6, 0.632456,        0,        0,        0,      0.2, 
		0.0,        0,        0,      0.6,      0.6,        0,        0,        0,        0


	}; 

	const unsigned char  test[3*72] = {
		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

		1,   1,   1, 255,   4,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4,   4,
		1,   1,   1,   1, 255,   4,   4,   4, 255,
		1,   1,   1,   1, 255,   4,   4, 255,   3,
		1,   1,   1, 255,   2, 255,   4, 255,   3,
		1,   1,   1,   1, 255,   3, 255,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,
		1,   1,   1,   1, 255,   3,   3,   3,   3,

	}; 

	P3DImage seed_image(new C3DUBImage(size, seed)); 
	P3DImage input_image(new C3DFImage(size, image));
	
	
	auto f = BOOST_TEST_create_from_plugin<C3DSeededWSFilterPlugin>("sws:seed=seed.@,n=6n,mark=1,grad=1"); 
	save_image("seed.@", seed_image); 
	
	auto result = f->filter(*input_image); 

	BOOST_CHECK_EQUAL(result->get_size(), size); 
	const C3DUBImage& r = dynamic_cast<const C3DUBImage&>(*result); 
	
	auto ir = r.begin(); 
	auto t = test; 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++t, ++ir) {
				BOOST_CHECK_EQUAL(*ir, *t); 
			}
	}

	cvdebug() << "result \n"; 
	ir = r.begin(); 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++ir) {
				cverb <<setw(3) << (int)*ir << ", "; 
			}
			cverb << "\n"; 
		}
	}
	
	cvdebug() << "test \n"; 
	t = test; 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++t) {
				cverb <<setw(8) << (int)*t << ", "; 
			}
			cverb << "\n"; 
		}
	}

	P3DImage grad = run_filter(*input_image, "gradnorm");
	const C3DFImage& fgrad = dynamic_cast<const C3DFImage&>(*grad); 
	
	cvdebug() << "grad \n"; 
	auto g = fgrad.begin(); 
	for (size_t z = 0; z < size.z; ++z) {
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++g) {
				cverb <<setw(8) << *g << ", "; 
			}
			cverb << "\n"; 
		}
	}
}
 
