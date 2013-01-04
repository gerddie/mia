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
#include <mia/3d/cost/ssd.hh>


using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;
using namespace NS;

BOOST_AUTO_TEST_CASE( test_SSD_3D_norm )
{
	float src_data[27] = { 1, 2, 3,  2, 3, 4,  1, 4, 2,
			       3, 4, 6,  7, 8, 4,  2, 3, 1,
			       4, 5, 7,  3, 1, 3,  6, 7, 8 };

	float ref_data[27] = { 5, 6, 7,  2, 1, 2,  2, 3, 2,
			       6, 7, 8,  9, 0, 9,  7, 6, 5,
			       1, 2, 3,  4, 6, 8,  3, 2, 3 };

	C3DFImage *fsrc = new C3DFImage(C3DBounds(3,3,3), src_data );
	C3DFImage *fref = new C3DFImage(C3DBounds(3,3,3), ref_data );
	std::shared_ptr<C3DImage > src(fsrc);
	std::shared_ptr<C3DImage > ref(fref);

	C3DSSDCost cost(true);
	cost.set_reference(*ref); 
	BOOST_CHECK_CLOSE(cost.value(*src),  0.5 * 367.0 / 27.0, 0.1);

	C3DFVectorfield force(C3DBounds(3,3,3));

	cost.evaluate_force(*src, 0.5, force);

	BOOST_CHECK_EQUAL(force(1,1,1), C3DFVector(-6/ 27.0, -2/ 27.0, -4/ 27.0) );
}

BOOST_AUTO_TEST_CASE( test_SSD_3D )
{
	float src_data[27] = { 1, 2, 3,  2, 3, 4,  1, 4, 2,
			       3, 4, 6,  7, 8, 4,  2, 3, 1,
			       4, 5, 7,  3, 1, 3,  6, 7, 8 };

	float ref_data[27] = { 5, 6, 7,  2, 1, 2,  2, 3, 2,
			       6, 7, 8,  9, 0, 9,  7, 6, 5,
			       1, 2, 3,  4, 6, 8,  3, 2, 3 };

	C3DFImage *fsrc = new C3DFImage(C3DBounds(3,3,3), src_data );
	C3DFImage *fref = new C3DFImage(C3DBounds(3,3,3), ref_data );
	std::shared_ptr<C3DImage > src(fsrc);
	std::shared_ptr<C3DImage > ref(fref);

	C3DSSDCost cost(false);
	cost.set_reference(*ref); 
	BOOST_CHECK_CLOSE(cost.value(*src),  0.5 * 367.0, 0.1);

	C3DFVectorfield force(C3DBounds(3,3,3));

	cost.evaluate_force(*src, 0.5, force);

	BOOST_CHECK_EQUAL(force(1,1,1), C3DFVector(-6, -2, -4) );
}


