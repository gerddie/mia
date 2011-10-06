/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <iomanip>
#include <mia/2d/multicost.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/transformmock.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core/mitestimages.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem; 
using namespace std; 

C2DFullCostPluginHandlerTestPath fullcost_path_init; 
C2DTransformCreatorHandlerTestPath transform_path_init;


BOOST_AUTO_TEST_CASE (test_image_spline_gradinet ) 
{
	C2DBounds size(mi_test_size.width, mi_test_size.height); 
	P2DImage mov(new C2DFImage(size, moving_init_data)); 
	P2DImage ref(new C2DFImage(size, reference_init_data)); 

	save_image("src.@", mov); 
	save_image("ref.@", ref); 

	auto tff = produce_2dtransform_factory("spline:rate=2"); 
	auto transform = tff->create(size); 
	auto cost = produce_2dfullcost("image:cost=[mi:rbins=16,mbins=16],src=src.@,ref=ref.@");
	cost->reinit(); 
	cost->set_size(size); 


	auto params = transform->get_parameters(); 

	CDoubleVector grad(params.size()); 

	cost->evaluate(*transform, grad);
	const double h = 0.1; 

	for (size_t i = 0; i < params.size(); ++i) {
		params[i] += h; 
		transform->set_parameters(params); 
		double pcost = cost->cost_value(*transform);
		params[i] -= 2 * h; 
		transform->set_parameters(params); 
		double mcost = cost->cost_value(*transform);
		params[i] += h; 
		
		cvmsg() << "mcost=" <<  setprecision(12) <<mcost << " pcost = " <<  pcost 
			<< " q= " << 2.0 * h * grad[i] / (pcost - mcost)
			<< "\n"; 

		BOOST_CHECK_CLOSE((pcost - mcost) / (2.0 * h), grad[i], 0.1); 

	}

}

