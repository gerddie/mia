/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/2d/filter/watershed.hh>


using namespace mia; 
using namespace std; 


BOOST_AUTO_TEST_CASE ( test_seeded_watershead ) 
{
	const C2DBounds size(11, 10); 
	

	const int image[110] = {
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

	const unsigned char  test[110] = {
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

	P2DImage input_image(new C2DSIImage(size, image));
	
	
	auto f = BOOST_TEST_create_from_plugin<C2DWatershedFilterPlugin>("ws:n=4n,mark=1,evalgrad=1"); 
	
	auto result = f->filter(*input_image); 

	BOOST_CHECK_EQUAL(result->get_size(), size); 
	const C2DUBImage& r = dynamic_cast<const C2DUBImage&>(*result); 
	
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

	P2DImage gradient = run_filter(*input_image, "gradnorm");
	const C2DFImage& fgrad = dynamic_cast<const C2DFImage&>(*gradient); 
	
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
 
