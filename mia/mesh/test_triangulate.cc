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

#define BOOST_TEST_MODULE FIFO_FILTER_TEST
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK

#include <mia/internal/autotest.hh>

#include <vector>
#include <iostream>

#include <mia/core/cmdlineparser.hh>
#include <mia/3d/vector.hh>
#include <mia/mesh/triangulate.hh>


NS_MIA_USE

BOOST_AUTO_TEST_CASE(  test_trianguale )
{
       std::vector<C3DFVector> vlist;
       vlist.push_back(C3DFVector(0.0, 0.0, 0.0));
       vlist.push_back(C3DFVector(0.0, 3.0, 0.0));
       vlist.push_back(C3DFVector(3.0, 3.0, 0.0));
       vlist.push_back(C3DFVector(3.0, 1.0, 0.0));
       vlist.push_back(C3DFVector(4.0, 1.0, 0.0));
       vlist.push_back(C3DFVector(4.0, 3.0, 0.0));
       vlist.push_back(C3DFVector(5.0, 3.0, 0.0));
       vlist.push_back(C3DFVector(5.0, 0.0, 0.0));
       vlist.push_back(C3DFVector(2.0, 0.0, 0.0));
       vlist.push_back(C3DFVector(2.0, 2.0, 0.0));
       vlist.push_back(C3DFVector(1.0, 2.0, 0.0));
       vlist.push_back(C3DFVector(1.0, 0.0, 0.0));
       TPolyTriangulator<std::vector<C3DFVector>, std::vector<int>>  triangulator(vlist);
       std::vector<int> polygon;
       polygon.push_back(0);
       polygon.push_back(1);
       polygon.push_back(2);
       polygon.push_back(3);
       polygon.push_back(4);
       polygon.push_back(5);
       polygon.push_back(6);
       polygon.push_back(7);
       polygon.push_back(8);
       polygon.push_back(9);
       polygon.push_back(10);
       polygon.push_back(11);
       std::vector<C3DBounds> triangles;
       triangulator.triangulate(triangles, polygon);

       for (size_t i = 0; i < triangles.size(); ++i) {
              cvdebug() << triangles[i] << std::endl;
       }

       BOOST_CHECK_EQUAL(triangles.size(), 10u);
}

