/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <mia/mesh/triangle_neighbourhood.hh>

using namespace std; 
using namespace mia; 

static void check_sets_equal(const set<unsigned>& lhs, const set<unsigned>& rhs) 
{
        BOOST_CHECK_EQUAL(lhs.size(), rhs.size()); 

        for (auto k: lhs)
                BOOST_CHECK(rhs.find(k) != rhs.end()); 
}

BOOST_AUTO_TEST_CASE(test_neighborhood) 
{
        auto vertices = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({C3DFVector(2,0,0), C3DFVector(-2,0,0), 
                                        C3DFVector(0,2,0), C3DFVector(0,-2,0), 
                                        C3DFVector(0,0,1), C3DFVector(0,0,-1)})); 
	
	typedef CTriangleMesh::triangle_type Triangle; 
	
	auto triangles = CTriangleMesh::PTrianglefield(
		new CTriangleMesh::CTrianglefield(
			{Triangle(4, 0, 2), Triangle(4, 2, 1), 
			 Triangle(4, 1, 3), Triangle(4, 3, 0), 
			 Triangle(5, 2, 0), Triangle(5, 1, 2), 
			 Triangle(5, 3, 1), Triangle(5, 0, 3)})); 
	
	CTriangleMesh mesh(triangles, vertices);

        CTrianglesWithAdjacentList adj(mesh); 
        
        set<unsigned> toadj0({3, 4, 1}); 
        set<unsigned> toadj1({0, 5, 2}); 
        set<unsigned> toadj2({1, 6, 3}); 
        set<unsigned> toadj3({2, 0, 7}); 

        set<unsigned> toadj4({7, 5, 0}); 
        set<unsigned> toadj5({4, 1, 6}); 
        set<unsigned> toadj6({5, 7, 2}); 
        set<unsigned> toadj7({6, 4, 3}); 
        
        check_sets_equal(adj[0], toadj0); 
        check_sets_equal(adj[1], toadj1); 
        check_sets_equal(adj[2], toadj2); 
        check_sets_equal(adj[3], toadj3); 
        check_sets_equal(adj[4], toadj4); 
        check_sets_equal(adj[5], toadj5); 
        check_sets_equal(adj[6], toadj6); 
        check_sets_equal(adj[7], toadj7);
        

}
