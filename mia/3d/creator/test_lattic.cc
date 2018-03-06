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

#include <mia/internal/plugintester.hh>
#include <mia/3d/creator/lattic.hh>
#include <mia/3d/filter.hh>

using namespace creator_lattic_3d;
using namespace mia;

BOOST_AUTO_TEST_CASE ( test_sphere )
{
       C3DBounds size(10, 20, 30);
       auto f = BOOST_TEST_create_from_plugin<C3DLatticCreatorPlugin>("lattic:fx=2,fy=3,fz=4");
       auto image = (*f)(size, it_float);
       BOOST_CHECK_EQUAL(image->get_size(), size);
}
