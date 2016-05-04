/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/mesh/triangularMesh.hh>

#include <set>

using namespace mia;
using namespace std;


set<C3DFVector> test_vertices = {
        C3DFVector(2,0,0), C3DFVector(-2,0,0), 
        C3DFVector(0,2,0), C3DFVector(0,-2,0), 
        C3DFVector(0,0,2), C3DFVector(0,0,-2)
};

typedef CTriangleMesh::triangle_type Triangle; 

set<Triangle>  test_triangles = 
        {Triangle(4, 0, 2), Triangle(4, 2, 1), 
         Triangle(4, 1, 3), Triangle(4, 3, 0), 
         Triangle(5, 2, 0), Triangle(5, 1, 2), 
         Triangle(5, 3, 1), Triangle(5, 0, 3)};

void test_vertex_set_equal(const CTriangleMesh::CVertexfield& vertices,
                           const set<C3DFVector>& expect_vertices)
{
        set<C3DFVector> loaded_vertices;
        loaded_vertices.insert(vertices.begin(), vertices.end());
        BOOST_CHECK(loaded_vertices == expect_vertices); 
        
}

void test_triangle_set_equal(const CTriangleMesh::CTrianglefield& triangles,
                             const set<Triangle>& expect)
{
        set<Triangle> loaded;
        loaded.insert(triangles.begin(), triangles.end());
        BOOST_CHECK(loaded == expect); 
        
}


BOOST_AUTO_TEST_CASE( test_load_save_octaedron_off )
{
        string filename(MIA_SOURCE_ROOT"/testdata/octahedron.off");

        auto mesh = CMeshIOPluginHandler::instance().load(filename);
        
        test_vertex_set_equal(mesh->get_vertices(), test_vertices);
        test_triangle_set_equal(mesh->get_triangles(), test_triangles); 
        
}


