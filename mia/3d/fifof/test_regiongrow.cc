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

#include <mia/internal/autotest.hh>
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/fifof/regiongrow.hh>

namespace bfs=boost::filesystem;
NS_USE(regiongrow_fifof);
NS_MIA_USE;

struct RegiongrowFixture: public fifof_Fixture {
	RegiongrowFixture() {
		CPathNameArray searchpath;
		searchpath.push_back(bfs::path("..")/bfs::path("shapes"));
		C3DShapePluginHandler::set_search_path(searchpath);
	}
};



BOOST_FIXTURE_TEST_CASE( test_fifof_regiongrow , RegiongrowFixture )
{
	const size_t n_slices = 5;
	const C2DBounds size(3,3);

	CProbabilityVector test_map(1,10);

	float k = 0.0f;
	for (CProbabilityVector::value_type::iterator i = test_map[0].begin();
	     i != test_map[0].end(); ++i, k += 0.1f)
		*i = k;

	unsigned char input_data[n_slices*9] = {
		  0, 5, 0,   0, 7, 0,   0, 0, 0 ,
		  0, 1, 0,   0, 6, 0,   0, 0, 0 ,
		  0, 2, 0,   8, 9, 7,   0, 6, 6 ,
		  0, 3, 0,   0, 7, 0,   0, 0, 6 ,
		  6, 4, 0,   0, 0, 0,   0, 0, 6
	};

	bool test_data[n_slices*9] = {
		  0, 0, 0,   0, 1, 0,   0, 0, 0,
		  0, 0, 0,   0, 1, 0,   0, 0, 0,
		  0, 0, 0,   1, 1, 1,   0, 1, 1,
		  0, 0, 0,   0, 1, 0,   0, 0, 1,
		  0, 0, 0,   0, 0, 0,   0, 0, 1
	};

	prepare(input_data, test_data, size, n_slices);

	C2DRegiongrowFifoFilter filter(test_map, 0.59f, 0.9f, 0, 2);
	call_test(filter);

}

