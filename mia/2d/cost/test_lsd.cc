/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/2d/cost/lsd.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace lsd_2dimage_cost;

class LSDFixture {
protected: 
	LSDFixture(); 
	
	
	C2DBounds size; 
	P2DImage src; 
	P2DImage ref; 
}; 


BOOST_FIXTURE_TEST_CASE( test_LSD_2D_self, LSDFixture )
{
	C2DLSDImageCost cost;
	cost.set_reference(*src);
	
	double cost_value = cost.value(*src);
	BOOST_CHECK_CLOSE(cost_value, 0.0, 0.1);

	C2DFVectorfield force(C2DBounds(8,8));

	BOOST_CHECK_CLOSE(cost.evaluate_force(*src, 0.5, force), 0.0, 0.1);

	BOOST_CHECK_EQUAL(force(1,1).x, 0.0f);
	BOOST_CHECK_EQUAL(force(1,1).y, 0.0f);
	
}


LSDFixture::LSDFixture():
	size(8,8)
{
	const float src_data[64] = {
		1, 1, 2, 2, 2, 3, 4, 4, 
		4, 4, 3, 3, 2, 2, 2, 1,
		2, 2, 3, 4, 5, 6, 7, 8, 
		9, 0, 2, 8, 3, 4, 2, 2,
		3, 1, 3, 4, 5, 6, 7, 8, 
		3, 4, 4, 5, 6, 4, 2, 2,
		0, 2, 3, 4, 5, 3, 1, 4, 
		5, 6, 7, 3, 2, 1, 2, 0
	};
	const float ref_data[64] = {
		8, 8, 9, 9, 9, 3, 4, 4, 
		4, 4, 3, 3, 9, 9, 9, 8,
		9, 9, 3, 4, 5, 6, 7, 3, 
		1, 0, 9, 3, 3, 4, 9, 9,
		3, 8, 3, 4, 5, 6, 7, 3, 
		3, 4, 4, 5, 6, 4, 9, 9,
		0, 9, 3, 4, 5, 3, 8, 4, 
		5, 6, 7, 3, 9, 8, 9, 0
	};

	src.reset(new C2DFImage(size, src_data ));
	ref.reset(new C2DFImage(size, ref_data ));
}
