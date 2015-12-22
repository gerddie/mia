/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/parallelcxx11.hh>
#include <mia/internal/autotest.hh>
using namespace mia;
using namespace std; 

BOOST_AUTO_TEST_CASE (test_preduce)
{
	int init_value = 1;
	C1DParallelRange range(0, 200, 10);

	vector<int> input(200);
	for (int i = 0; i < 200; ++i)
		input[i] = i;

	auto p_func = [&input](const C1DParallelRange& range, int in_value) {
		for (auto i= range.begin(); i != range.end(); ++i) {
			in_value += input[i]; 
		}
		return in_value; 
	};

	auto r_func = [](int a, int b){return a+b;}; 
	
	int result = preduce(range, init_value, p_func, r_func); 
	
	BOOST_CHECK_EQUAL(result, 100*199 + 1); 
	
}
