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


#include <mia/internal/autotest.hh>
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/fifof/morphological.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace morphological_fifof;
namespace bfs=boost::filesystem;

struct MorpologicalFilter_Fixture: public fifof_Fixture  {

	MorpologicalFilter_Fixture();
	P3DShape m_shape;
};

MorpologicalFilter_Fixture::MorpologicalFilter_Fixture()
{
	list< bfs::path> searchpath;
	searchpath.push_back(bfs::path("..")/bfs::path("shapes"));
	C3DShapePluginHandler::set_search_path(searchpath);
	m_shape = C3DShapePluginHandler::instance().produce("6n");
}

BOOST_FIXTURE_TEST_CASE ( morphological_dilate, MorpologicalFilter_Fixture )
{
	const size_t n_slices = 5;
	const C2DBounds size(3,3);
	const size_t npixels = 9;

	const float input_data[n_slices * npixels] = {
		  0, 0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 1, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 1, 0, 0,
		  0, 1, 0, 0, 0, 0, 0, 0, 0
	};

	const float test_data[n_slices * npixels] = {
		  0, 0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 1, 0, 0, 0, 0, 0,
		  1, 0, 0, 1, 1, 0, 1, 0, 0,
		  0, 1, 0, 1, 0, 0, 1, 1, 0,
		  1, 1, 1, 0, 1, 0, 1, 0, 0
	};

	prepare(input_data, test_data, size, n_slices);
	C2DMorphFifoFilter<DilateCompare> dilate(m_shape);
	call_test(dilate);
}

BOOST_FIXTURE_TEST_CASE ( morphological_erode, MorpologicalFilter_Fixture )
{
	const size_t n_slices = 5;
	const C2DBounds size(3,3);

	float input_data[n_slices*9] = {
		  2, 2, 2, 2, 2, 2, 2, 2, 2,
		  2, 2, 2, 2, 2, 2, 2, 2, 2,
		  2, 2, 2, 1, 2, 2, 2, 2, 2,
		  2, 2, 2, 2, 2, 2, 1, 2, 2,
		  2, 1, 2, 2, 2, 2, 2, 2, 2
	};

	float test_data[n_slices*9] = {
		  2, 2, 2, 2, 2, 2, 2, 2, 2,
		  2, 2, 2, 1, 2, 2, 2, 2, 2,
		  1, 2, 2, 1, 1, 2, 1, 2, 2,
		  2, 1, 2, 1, 2, 2, 1, 1, 2,
		  1, 1, 1, 2, 1, 2, 1, 2, 2
	};

	prepare(input_data, test_data, size, n_slices);

	C2DMorphFifoFilter<ErodeCompare> erode(m_shape);
	call_test(erode);
}

