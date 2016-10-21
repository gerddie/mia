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

#include <mia/core/parallelcxx11.hh>
#include <mia/internal/autotest.hh>
using namespace mia;
using namespace std; 

BOOST_AUTO_TEST_CASE (test_preduce)
{
	int identity_value = 0;
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
	
	int result = preduce(range, identity_value, p_func, r_func); 
	
	BOOST_CHECK_EQUAL(result, 100*199); 
	
}

BOOST_AUTO_TEST_CASE (test_preduce_vector_result)
{
	vector<int> identity_value(2, 0);
	C1DParallelRange range(0, 200, 10);

	vector<int> input(200);
	for (int i = 0; i < 200; ++i)
		input[i] = i;

	auto p_func = [&input](const C1DParallelRange& range, vector<int> in_value) -> vector<int>{
		for (auto i= range.begin(); i != range.end(); ++i) {
			in_value[0] += input[i];
			in_value[1] += 2 * input[i]; 
		}
		return in_value; 
	};

	auto r_func = [](const vector<int>& a, const vector<int>& b) -> vector<int>{
		vector<int> out(a);
		transform(out.begin(), out.end(), b.begin(), out.begin(), [](int ia, int ib){return ia + ib;}); 
		return out;
	}; 
	
	auto  result = preduce(range, identity_value, p_func, r_func);
	
	BOOST_CHECK_EQUAL(result.size(), 2u); 
	BOOST_CHECK_EQUAL(result[0], 100*199);
	BOOST_CHECK_EQUAL(result[1], 2 * 100*199); 
	
}


BOOST_AUTO_TEST_CASE (test_pfor)
{

	C1DParallelRange range(0, 200, 10);

	vector<int> input(200);
	for (int i = 0; i < 200; ++i)
		input[i] = i;

	auto p_func = [&input](const C1DParallelRange& range) {
		for (auto i= range.begin(); i != range.end(); ++i) {
			input[i] *= 2; 
		}
	};


	pfor(range, p_func); 
	
	for (int i = 0; i < 200; ++i) {
		BOOST_CHECK_EQUAL(input[i], 2*i); 
	}
}
