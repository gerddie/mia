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
#include <mia/3d/cost/fatssd.hh>

using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;

using ssd_3dimage_fatcost::CFatSSD3DImageCost; 

CSplineKernelTestPath init_path; 
C3DImageCostPluginHandlerTestPath init_cost_path; 

BOOST_AUTO_TEST_CASE ( test_fatssd ) 
{
	C3DBounds size(8,16,7);

	auto ptest = new C3DFImage(size); 
	auto pref = new C3DFImage(size); 
	
	fill(ptest->begin(), ptest->end(), 1.0f); 
	fill(pref->begin(), pref->end(), 1.0f); 
		
	
	P3DImage test_image(ptest);
	P3DImage ref_image(pref);


	P3DInterpolatorFactory ipf(create_3dinterpolation_factory(ip_bspline3, bc_mirror_on_bounds));


	CFatSSD3DImageCost cost(test_image, ref_image,  1.0);
	/*	
	double scale = 1.0; 

	BOOST_CHECK_CLOSE(scale * cost.value(),8 * 16 * 7 * 0.5, 0.1);  
	*/
}
