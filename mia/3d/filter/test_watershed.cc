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

#include <iomanip>
#include <mia/internal/plugintester.hh> 
#include <mia/3d/filter/watershed.hh>


using namespace mia; 
using namespace std; 

BOOST_AUTO_TEST_CASE ( test_seeded_watershead ) 
{
	const C3DBounds size(11, 10, 3); 
	

	const int image[330] = {
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		1,   1,   1,   2,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   0,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,

		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		1,   1,   1,   2,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   0,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,

		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		1,   1,   1,   2,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   0,   0,  0,  0,
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,


	}; 

	const unsigned char  test[330] = {
		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3, 
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4,   4, 255,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3,
              255, 255, 255, 255, 255, 255, 255,   2, 255,  3,  3,
		5,   5,   5,   5,   5,   5,   5, 255,   3,  3,  3,
		5,   5,   5,   5,   5,   5, 255,   6, 255,  3,  3,

		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3, 
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4,   4, 255,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3,
              255, 255, 255, 255, 255, 255, 255,   2, 255,  3,  3,
		5,   5,   5,   5,   5,   5,   5, 255,   3,  3,  3,
		5,   5,   5,   5,   5,   5, 255,   6, 255,  3,  3,

		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3, 
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	        4,   4,   4,   4, 255,   1, 255,   2, 255,  3,  3,
	        4,   4,   4, 255,   1,   1, 255,   2, 255,  3,  3,
	      255, 255, 255,   1,   1,   1, 255,   2, 255,  3,  3,
		1,   1,   1,   1,   1,   1, 255,   2, 255,  3,  3,
              255, 255, 255, 255, 255, 255, 255,   2, 255,  3,  3,
		5,   5,   5,   5,   5,   5,   5, 255,   3,  3,  3,
		5,   5,   5,   5,   5,   5, 255,   6, 255,  3,  3,

	}; 

	P3DImage input_image(new C3DSIImage(size, image));
	
	
	auto f = BOOST_TEST_create_from_plugin<C3DWatershedFilterPlugin>("ws:n=6n,mark=1,evalgrad=1");
	
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
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "line " << y << ":";  
		for (size_t x = 0; x < size.x; ++x, ++ir) {
			cverb <<setw(3) << (int)*ir << ", "; 
		}
		cverb << "\n"; 
	}
	
	cvdebug() << "test \n"; 
	t = test; 
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "line " << y << ":";  
		for (size_t x = 0; x < size.x; ++x, ++t) {
			cverb <<setw(8) << (int)*t << ", "; 
		}
		cverb << "\n"; 
	}

	P3DImage gradient = run_filter(*input_image, "gradnorm");
	const C3DFImage& fgrad = dynamic_cast<const C3DFImage&>(*gradient); 
	
	cvdebug() << "grad \n"; 
	auto g = fgrad.begin(); 
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "line " << y << ":";  
		for (size_t x = 0; x < size.x; ++x, ++g) {
			cverb <<setw(8) << *g << ", "; 
		}
		cverb << "\n"; 
	}


}
 
BOOST_AUTO_TEST_CASE ( test_seeded_watershead_from_grad ) 
{
	const C3DBounds size(11, 10, 4); 

	const int image[440] = {
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   2,   0,  0,  0,
		1,   1,   1,   2,   1,   1,   1,   2,   0,  0,  0,
		2,   2,   2,   2,   1,   1,   1,   2,   0,  0,  0,
		0,   0,   0,   2,   1,   1,   1,   2,   0,  0,  0,
		0,   0,   0,   2,   2,   2,   2,   2,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   0,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   2,   0,  0,  0,

		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   2,   0,  0,  0,
		1,   1,   1,   2,   1,   1,   1,   2,   0,  0,  0,
		2,   2,   2,   2,   1,   1,   1,   2,   0,  0,  0,
		0,   0,   0,   2,   1,   1,   1,   2,   0,  0,  0,
		0,   0,   0,   2,   2,   2,   2,   2,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   2,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   0,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   2,   0,  0,  0,

		0,   0,   0,   0,   0,   0,   0,   4,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   4,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   4,   0,  0,  0,
		1,   1,   1,   2,   3,   3,   3,   4,   2,  2,  2,
		2,   2,   2,   2,   3,   3,   3,   4,   2,  2,  2,
		0,   0,   0,   2,   3,   3,   3,   4,   1,  1,  1,
		0,   0,   0,   2,   2,   2,   2,   4,   1,  1,  1,
		0,   0,   0,   2,   0,   0,   0,   4,   2,  2,  2,
		2,   2,   2,   2,   2,   2,   2,   4,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   4,   0,  0,  0,


		0,   0,   0,   0,   0,   0,   0,   4,   0,  0,  0, 
		0,   0,   0,   0,   0,   0,   0,   4,   0,  0,  0,
		2,   2,   2,   2,   2,   2,   2,   4,   0,  0,  0,
		1,   1,   1,   2,   1,   1,   1,   4,   1,  1,  1,
		2,   2,   2,   2,   1,   0,   1,   4,   0,  0,  2,
		0,   0,   0,   2,   1,   1,   1,   4,   0,  0,  2,
		0,   0,   0,   2,   2,   2,   2,   4,   0,  0,  2,
		0,   0,   0,   2,   0,   0,   0,   4,   1,  1,  1,
		2,   2,   2,   2,   2,   2,   2,   4,   0,  0,  0,
		0,   0,   0,   2,   0,   0,   0,   4,   0,  0,  0,


	}; 


	const int test[440] = {
                1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2, 
		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
              255, 255, 255,   1, 255, 255, 255,   2,   2,  2,  2,
		9,   9,   9, 255,  10,  10,  10, 255,   2,  2,  2,
  	      255, 255, 255,  10,  10,  10,  10, 255,   2,  2,  2,
		3,   3,   3, 255,  10,  10,  10, 255,   2,  2,  2,
		3,   3,   3,   3, 255, 255, 255,   2,   2,  2,  2,
		3,   3,   3, 255,   4,   4,   4, 255,   2,  2,  2,
	      255, 255, 255,  11, 255, 255, 255,   2,   2,  2,  2,
		5,   5,   5, 255,   6,   6,   6, 255,   2,  2,  2,

		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2, 
		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
	      255, 255, 255,   1, 255, 255, 255,   2,   2,  2,  2,
		9,   9,   9, 255,  10,  10,  10, 255,   2,  2,  2,
              255, 255, 255,  10,  10,  10,  10, 255,   2,  2,  2,
		3,   3,   3, 255,  10,  10,  10, 255,   2,  2,  2,
		3,   3,   3,   3, 255, 255, 255,   2,   2,  2,  2,
		3,   3,   3, 255,   4,   4,   4, 255,   2,  2,  2,
	      255, 255, 255,  11, 255, 255, 255,   2,   2,  2,  2,
		5,   5,   5, 255,   6,   6,   6, 255,   2,  2,  2,

		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
	      255, 255, 255,   1,   1,   1,   1, 255,   2,  2,  2,
		9,   9,   9, 255, 255, 255, 255,   2,   2,  2,  2,
              255, 255, 255,  10, 255, 255, 255,   2, 255,255,  2,
		3,   3,   3, 255, 255, 255, 255,   2, 255,255,  2,
		3,   3,   3,   3, 255,   4,   4, 255, 255,255,  2,
		3,   3,   3, 255,   4,   4,   4, 255,   2,  2,  2,
	      255, 255, 255,  11, 255, 255, 255,   2,   2,  2,  2,
		5,   5,   5, 255,   6,   6,   6, 255,   2,  2,  2,


		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
		1,   1,   1,   1,   1,   1,   1, 255,   2,  2,  2,
	      255, 255, 255,   1, 255, 255, 255,   2,   2,  2,  2,
		9,   9,   9, 255,   7,   7,   7, 255, 255,255,  2,
              255, 255, 255, 255,   7,   7,   7, 255,   8,  8,255,
		3,   3,   3, 255,   7,   7,   7, 255,   8,  8,255,
		3,   3,   3,   3, 255, 255, 255,   8,   8,  8,255,
		3,   3,   3, 255,   4,   4,   4, 255, 255,255,  2,
	      255, 255, 255,  11, 255, 255, 255,   2,   2,  2,  2,
		5,   5,   5, 255,   6,   6,   6, 255,   2,  2,  2,


	}; 
	
	P3DImage input_image(new C3DSIImage(size, image));
	
	auto f = BOOST_TEST_create_from_plugin<C3DWatershedFilterPlugin>("ws:n=6n,mark=1");
	auto result = f->filter(*input_image); 
	
	BOOST_CHECK_EQUAL(result->get_size(), size); 
	const C3DUBImage& r = dynamic_cast<const C3DUBImage&>(*result); 

	auto ir = r.begin(); 
	auto t = test; 
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++t, ++ir) {
				BOOST_CHECK_EQUAL(*ir, *t); 
			}
	
	ir = r.begin(); 
	t = test; 
	auto i = image; 
	for (size_t z = 0; z < size.z; ++z) {
		cvdebug() << "result \n"; 
		for (size_t y = 0; y < size.y; ++y) {
			cvdebug() << "line " << y << ":";  
			for (size_t x = 0; x < size.x; ++x, ++ir, ++t, ++i) {
				if (*ir == *t) 
					cverb << *i <<" |" << setw(3) << (int)*ir << " " << setw(3) << (int)*t << "| ,"; 
				else 
					cverb << *i <<"=>" << setw(3) << (int)*ir << " " << setw(3) << (int)*t << "<=,"; 
			}
			cverb << "\n"; 
		}

	}

}
	

