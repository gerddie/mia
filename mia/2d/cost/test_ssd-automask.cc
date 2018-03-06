/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/2d/cost/ssd-automask.hh>


using namespace std;
using namespace boost;
namespace bfs =::boost::filesystem;
using namespace mia;
using namespace NS;

BOOST_AUTO_TEST_CASE( test_SSD_2D_autmask )
{
       float src_data[25] = { 2, 3, 2, 3, 4,
                              1, 4, 2, 3, 4,
                              6, 7, 4, 2, 3,
                              1, 4, 5, 7, 3,
                              1, 3, 6, 7, 8
                            };
       float ref_data[25] = { 6, 7, 2, 1, 2,
                              2, 3, 2, 6, 7,
                              8, 9, 9, 7, 6,
                              5, 1, 2, 3, 4,
                              6, 8, 3, 2, 3
                            };
       C2DFImage *fsrc = new C2DFImage(C2DBounds(5, 5), src_data );
       C2DFImage *fref = new C2DFImage(C2DBounds(5, 5), ref_data );
       std::shared_ptr<C2DImage > src(fsrc);
       std::shared_ptr<C2DImage > ref(fref);
       C2DSSDAutomaskCost cost(2, 1);
       cost.set_reference(*ref);
       BOOST_CHECK_CLOSE(cost.value(*src),  0.5 * 245.0 / 22.0, 0.1);
       C2DFVectorfield force(C2DBounds(5, 5));
       BOOST_CHECK_CLOSE(cost.evaluate_force(*src, force),  0.5 * 245.0 / 22.0, 0.1);
       auto f = force(1, 1);
       BOOST_CHECK_CLOSE(f.x,  0.5 / 22.0, 0.01);
       BOOST_CHECK_CLOSE(f.y,  2.0 / 22.0, 0.01);
}

