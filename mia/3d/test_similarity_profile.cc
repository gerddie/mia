/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/3d/similarity_profile.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/filter.hh>


namespace bfs=::boost::filesystem;
NS_MIA_USE; 

CSplineKernelTestPath splinekernel_init_path; 
struct SimityProfileFixture {
	SimityProfileFixture(); 

	P3DFullCost cost; 
	C3DImageSeries series; 

}; 


BOOST_FIXTURE_TEST_CASE (test_C3DSimilarityProfile_ref10, SimityProfileFixture) 
{
	
	C3DSimilarityProfile sp(cost, series, 10); 
	// test value obtained by using octave 
	BOOST_CHECK_CLOSE(sp.get_peak_frequency(), 108.98704, 0.1);
	
}



SimityProfileFixture::SimityProfileFixture()
{
	CPathNameArray cost_plugpath;
	cost_plugpath.push_back(bfs::path("cost"));
	C3DImageCostPluginHandler::set_search_path(cost_plugpath);

	CPathNameArray filter_plugpath;
	filter_plugpath.push_back(bfs::path("filter"));
	C3DFilterPluginHandler::set_search_path(filter_plugpath);

	CPathNameArray io_plugpath;
	io_plugpath.push_back(bfs::path("io"));
	C3DImageIOPluginHandler::set_search_path(io_plugpath);

	CPathNameArray fullcost_plugpath;
	fullcost_plugpath.push_back(bfs::path("fullcost"));
	C3DFullCostPluginHandler::set_search_path(fullcost_plugpath);


	cost = C3DFullCostPluginHandler::instance().produce("image");

	C3DBounds size(1,1,1); 
	float values[40] = {
		-2, -3, -4, -1, 1, 4, 2, -1, -3, -2, 
		-2, -3, -4, -1, 1, 4, 2, -1, -3, -2, 
		-2, -3, -4, -1, 1, 4, 2, -1, -3, -2, 
		-2, -3, -4, -1, 1, 4, 2, -1, -3, -2 }; 
 
	for(size_t i = 0; i < 40; ++i) {
		C3DFImage *img = new C3DFImage(size); 
		(*img)(0,0,0) = values[i]; 
		series.push_back(P3DImage(img)); 
	}
}

