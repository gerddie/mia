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
#include <mia/3d/fifof/byslice.hh>
#include <mia/core/spacial_kernel.hh>

NS_USE(byslice_2dstack_filter);
NS_MIA_USE;
namespace bfs=::boost::filesystem;


void init_path() 
{
	static bool run = false; 
	if (run) 
		return; 
	
	CPathNameArray kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("..")/
				   bfs::path("core")/bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);
	
	CPathNameArray filter2dpath;
	filter2dpath.push_back(bfs::path("../../2d/filter"));
	C2DFilterPluginHandler::set_search_path(filter2dpath);
	run = true; 
}

BOOST_FIXTURE_TEST_CASE( test_fifof_byslice_median , fifof_Fixture )
{
	init_path(); 

	const size_t n_slices = 3;
	const C2DBounds size(3,3);

	float input_data[n_slices * 9] = {
		1, 2, 3, 
		4, 5, 6, 
		7, 8, 9,
		
		11, 12, 13, 
		14, 15, 16, 
		17, 18, 19,
		
		21, 22, 23, 
		24, 25, 26, 
		27, 28, 29
	};

	float test_data[n_slices * 9] = {
		   3, 3.5,    4, 
		 4.5,   5,  5.5, 
		   6,  6.5,   7,

		  13, 13.5,    14, 
		14.5,   15,  15.5, 
		  16, 16.5,    17,
		   
		  23, 23.5,    24, 
		24.5,   25,  25.5, 
		  26, 26.5,    27,
		   
	};

	prepare(input_data, test_data, size, n_slices);
	C2DBysliceFifoFilter f("median:w=1");

	call_test(f);
}
