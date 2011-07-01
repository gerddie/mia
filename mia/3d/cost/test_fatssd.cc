/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

BOOST_AUTO_TEST_CASE ( test_fatssd ) 
{
	C3DBounds size(8,16,7);
	vector<float> init_test(8 * 16 * 7, 1.0);
	vector<float> init_ref (8 * 16 * 7, 2.0);

	P3DImage test_image(new C3DFImage(size, &init_test[0]));
	P3DImage ref_image(new C3DFImage(size, &init_ref[0]));

	P3DInterpolatorFactory ipf(create_3dinterpolation_factory(ip_bspline3));
	CFatSSD3DImageCost cost(test_image, ref_image, ipf, 1.0);
	double scale = 1.0; 

	BOOST_CHECK_CLOSE(scale * cost.value(),8 * 16 * 7 * 0.5, 0.1);  
}
