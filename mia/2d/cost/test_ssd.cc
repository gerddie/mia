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
#include <mia/2d/cost/ssd.hh>


using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;
using namespace mia_2dcost_ssd;

BOOST_AUTO_TEST_CASE( test_SSD_2D )
{
	const float src_data[16] = {
		0, 0, 0, 0,
		0, 3, 1, 0,
		0, 6, 7, 0,
		0, 0, 0, 0
	};
	const float ref_data[16] = {
		0, 0, 0, 0,
		0, 2, 3, 0,
		0, 1, 2, 0,
		0, 0, 0, 0
	};

	P2DImage src(new C2DFImage(C2DBounds(4,4), src_data ));
	P2DImage ref(new C2DFImage(C2DBounds(4,4), ref_data ));

	C2DSSDCost cost(true);
	cost.set_reference(*ref); 
	double cost_value = cost.value(*src);
	BOOST_CHECK_CLOSE(cost_value, 0.5 * 55.0 / 16.0, 0.1);

	C2DFVectorfield force(C2DBounds(4,4));

	BOOST_CHECK_CLOSE(cost.evaluate_force(*src, force), 0.5 * 55.0 / 16.0, 0.1);

	BOOST_CHECK_CLOSE(force(1,1).x, 0.25f / 8.0f, 0.1);
	BOOST_CHECK_CLOSE(force(1,1).y, 1.50f / 8.0f, 0.1);
}


