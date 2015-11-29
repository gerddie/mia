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

#include <mia/internal/autotest.hh>
#include <mia/core/seriesstats.hh>

NS_MIA_USE; 

using std::vector; 

BOOST_AUTO_TEST_CASE( test_seriesstats )
{
	FIntensityStatsAccumulator acc; 

	vector<double> a = {1,2,3,4,5}; 
	vector<double> b = {5,6,4}; 
	
	acc(a); 
	acc(b); 
	acc(a); 

	auto result = acc.get_result(); 

	BOOST_CHECK_EQUAL(result.n, 2 * a.size() + b.size()); 
	BOOST_CHECK_EQUAL(result.sum, 45); 
	BOOST_CHECK_EQUAL(result.sumsq, 110 + 25+16+36); 
	BOOST_CHECK_EQUAL(result.min, 1); 
	BOOST_CHECK_EQUAL(result.max, 6); 
	
	BOOST_CHECK_CLOSE(result.mean, 45.0/13.0, 0.001); 
	BOOST_CHECK_CLOSE(result.variation, sqrt(2.6025641026), 0.001); 
	
	
	
	
	


}
