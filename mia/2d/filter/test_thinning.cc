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


#include <mia/internal/plugintester.hh>
#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
#include <mia/2d/filter/thinning.hh>
#include <mia/2d/imagetest.hh>

namespace bmpl=boost::mpl;

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace thinning_2dimage_filter;


BOOST_AUTO_TEST_CASE( test_circular_thinning )
{
	const C2DBounds size(14, 16); 
	
	vector<bool> input_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, // 4
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 9
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, // 9 
		0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 5
		0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 6
		0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 4
		0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 6
		0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, // 9
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 9 
		0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 6
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1 = 68
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	vector<bool> test_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, // 4 
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, // 7 
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 2
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2 
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 2 
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, // 2
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 6 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1 = 28 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	C2DBitImage source(size, input_image); 
	C2DBitImage expect(size, test_image); 
	
	C2DThinningImageFilter filter(0);

	P2DImage target = filter.filter(source);
	test_image_equal(*target, expect);


	auto e = expect.begin(); 
	const C2DBitImage& targ = dynamic_cast<const C2DBitImage&>(*target); 
	auto t = targ.begin(); 
	
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "cmp = "; 
		for (size_t x = 0; x < size.x; ++x, ++e, ++t) {
				if (*t == *e) 
					cverb << "(" << *t << "=" << *e << ") "; 
				else 
					cverb << "<" << *t << " " << *e << "> "; 
			}
		cverb << "\n"; 
	}
}



BOOST_AUTO_TEST_CASE( test_pruning_keep_all )
{
	const C2DBounds size(14, 16); 
	
	vector<bool> input_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, // 4 
		0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, // 7 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2 
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 2 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, // 2
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 6 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1 = 28 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	vector<bool> test_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, // 4 
		0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, // 7 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2 
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 2 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, // 2
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 6 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1 = 28 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	C2DBitImage source(size, input_image); 
	C2DBitImage expect(size, test_image); 
	
	C2DPruningImageFilter filter(1);

	P2DImage target = filter.filter(source);
	test_image_equal(*target, expect);


	auto e = expect.begin(); 
	const C2DBitImage& targ = dynamic_cast<const C2DBitImage&>(*target); 
	auto t = targ.begin(); 
	
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "cmp = "; 
		for (size_t x = 0; x < size.x; ++x, ++e, ++t) {
				if (*t == *e) 
					cverb << "(" << *t << "=" << *e << ") "; 
				else 
					cverb << "<" << *t << " " << *e << "> "; 
			}
		cverb << "\n"; 
	}
}



BOOST_AUTO_TEST_CASE( test_pruning_until_convergence )
{
	const C2DBounds size(14, 16); 
	
	vector<bool> input_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, // 4 
		0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, // 7 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, // 2 
		0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 2 
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, // 2
		0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 6 
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, // 1 = 28 
		0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	vector<bool> test_image = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, // 4 
		0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, // 7 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2 
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // 2 
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, // 2
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // 6 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1 = 28 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}; 

	C2DBitImage source(size, input_image); 
	C2DBitImage expect(size, test_image); 
	
	C2DPruningImageFilter filter(0);

	P2DImage target = filter.filter(source);
	test_image_equal(*target, expect);


	auto e = expect.begin(); 
	const C2DBitImage& targ = dynamic_cast<const C2DBitImage&>(*target); 
	auto t = targ.begin(); 
	
	for (size_t y = 0; y < size.y; ++y) {
		cvdebug() << "cmp = "; 
		for (size_t x = 0; x < size.x; ++x, ++e, ++t) {
				if (*t == *e) 
					cverb << "(" << *t << "=" << *e << ") "; 
				else 
					cverb << "<" << *t << " " << *e << "> "; 
			}
		cverb << "\n"; 
	}
}


