/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/mesh/filter/deltrianglesbynormal.hh>

#include <set>

using namespace mia; 
using namespace mia_meshfilter_deltribynormal; 

using std::set; 
using std::for_each; 
using std::make_shared; 

struct DefineMeshFixture {

	DefineMeshFixture(); 

	PTriangleMesh mesh; 
}; 

template <typename T> 
struct compare_vertex_ordered  {
	bool operator () (const T3DVector<T>& lhs, const T3DVector<T>& rhs) {
		return (lhs.z < rhs.z) || 
			((lhs.z == rhs.z) && ((lhs.y < rhs.y) ||
						  ((lhs.y == rhs.y) && (lhs.x < rhs.x)))); 
	}
}; 




BOOST_FIXTURE_TEST_CASE(test_positive_z, DefineMeshFixture) 
{
	auto clean = BOOST_TEST_create_from_plugin<CDeleteTriangleByNormalMeshFilterPlugin>("deltribynormal:dir=[0,0,1],angle=36");
	
	auto filtered_mesh = clean->filter(mesh); 

	BOOST_CHECK_EQUAL(filtered_mesh->triangle_size(), 4u); 
	BOOST_CHECK_EQUAL(filtered_mesh->vertices_size(), 5u); 
	
	set<C3DFVector, compare_vertex_ordered<float> > expect_vertices({C3DFVector(2,0,0), C3DFVector(-2,0,0), 
				C3DFVector(0,2,0), C3DFVector(0,-2,0), 
				C3DFVector(0,0,1)}); 
	
	for_each(filtered_mesh->vertices_begin(), filtered_mesh->vertices_end(), 
		 [&expect_vertices](const C3DFVector& x) {BOOST_CHECK(expect_vertices.find(x) != expect_vertices.end());}); 

	typedef CTriangleMesh::triangle_type Triangle; 
	
	
	set<Triangle, compare_vertex_ordered<unsigned int>> expect_triangles(
		{Triangle(4, 2, 3), Triangle(4, 3, 1), Triangle(4, 1, 0), Triangle(4, 0, 2)}
		); 

	for_each(filtered_mesh->triangles_begin(), filtered_mesh->triangles_end(), 
		 [&expect_triangles](const Triangle& x) {BOOST_CHECK(expect_triangles.find(x) != expect_triangles.end());});
}



DefineMeshFixture::DefineMeshFixture()
{
	auto vertices = CTriangleMesh::PVertexfield(
		new CTriangleMesh::CVertexfield(
			{C3DFVector(2,0,0), C3DFVector(-2,0,0), 
					C3DFVector(0,2,0), C3DFVector(0,-2,0), 
					C3DFVector(0,0,1), C3DFVector(0,0,-1)})); 
	
	typedef CTriangleMesh::triangle_type Triangle; 
	
	auto triangles = CTriangleMesh::PTrianglefield(
		new CTriangleMesh::CTrianglefield(
			{Triangle(4, 0, 2), Triangle(4, 2, 1), 
					Triangle(4, 1, 3), Triangle(4, 3, 0), 
					Triangle(5, 2, 0), Triangle(5, 1, 2), 
					Triangle(5, 3, 1), Triangle(5, 0, 3)})); 
	
	mesh = make_shared<CTriangleMesh>(triangles, vertices); 
	
	
}
