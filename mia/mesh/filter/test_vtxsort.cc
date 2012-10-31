/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <mia/internal/plugintester.hh>
#include <mia/mesh/filter/vtxsort.hh>

using namespace mia; 
using namespace mia_meshfilter_vtxsort; 

class  MeshVtxsortFixture {
protected: 
	MeshVtxsortFixture(); 
	
	
	void test_expected(const CTriangleMesh& changed, const CTriangleMesh& test) const;
	
	typedef CTriangleMesh::CTrianglefield::value_type Triangle;
	CTriangleMesh mesh; 
}; 

BOOST_FIXTURE_TEST_CASE( test_simple_mesh_z_order, MeshVtxsortFixture) 
{
	auto testv = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({
				C3DFVector(3,4,1), C3DFVector(5,3,3), 
					C3DFVector(2,1,4), C3DFVector(1,5,8)})); 

	auto testt = CTriangleMesh::PTrianglefield(new CTriangleMesh::CTrianglefield({
				Triangle(3,1,0), Triangle(1,0,2), 
					Triangle(3,0,2), Triangle(3,2,1)})); 
	
	CTriangleMesh test(testt, testv); 

	auto scale = BOOST_TEST_create_from_plugin<CVtxSortMeshFilterPlugin>("vtxsort:dir=[<0,0,1>]");
	auto filtered_mesh = scale->filter(mesh); 
	
	test_expected(*filtered_mesh, test);
}


BOOST_FIXTURE_TEST_CASE( test_simple_mesh_y_order, MeshVtxsortFixture) 
{
	auto testv = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({
				C3DFVector(2,1,4), C3DFVector(5,3,3), C3DFVector(3,4,1), C3DFVector(1,5,8)
					})); 
	
	auto testt = CTriangleMesh::PTrianglefield(new CTriangleMesh::CTrianglefield({
				Triangle(3,1,2), Triangle(1,2,0), Triangle(3,2,0), Triangle(3,0,1)
					})); 
	
	CTriangleMesh test(testt, testv); 
	
	auto scale = BOOST_TEST_create_from_plugin<CVtxSortMeshFilterPlugin>("vtxsort:dir=[<0,1,0>]");
	auto filtered_mesh = scale->filter(mesh); 
	
	test_expected(*filtered_mesh, test);
}

BOOST_FIXTURE_TEST_CASE( test_simple_mesh_x_order, MeshVtxsortFixture) 
{
	auto testv = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({
				C3DFVector(1,5,8), C3DFVector(2,1,4), C3DFVector(3,4,1), C3DFVector(5,3,3)
					})); 

	auto testt = CTriangleMesh::PTrianglefield(new CTriangleMesh::CTrianglefield({
				Triangle(0,3,2), Triangle(3,2,1), Triangle(0,2,1), Triangle(0,1,3) 
					})); 
	
	CTriangleMesh test(testt, testv); 

	auto scale = BOOST_TEST_create_from_plugin<CVtxSortMeshFilterPlugin>("vtxsort:dir=[<1,0,0>]");
	auto filtered_mesh = scale->filter(mesh); 
	
	test_expected(*filtered_mesh, test);
}

BOOST_FIXTURE_TEST_CASE( test_simple_mesh_comb, MeshVtxsortFixture) 
{
	auto testv = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({
				C3DFVector(1,5,8), C3DFVector(2,1,4), C3DFVector(3,4,1),  C3DFVector(5,3,3), 
					})); 

	auto testt = CTriangleMesh::PTrianglefield(new CTriangleMesh::CTrianglefield({
				Triangle(0,3,2), Triangle(3,2,1), Triangle(0,2,1), Triangle(0,1,3) 
					})); 
	
	CTriangleMesh test(testt, testv); 

	auto scale = BOOST_TEST_create_from_plugin<CVtxSortMeshFilterPlugin>("vtxsort:dir=[<2,-1,-2>]");
	auto filtered_mesh = scale->filter(mesh); 
	
	test_expected(*filtered_mesh, test);
}

MeshVtxsortFixture::MeshVtxsortFixture():mesh(4,4)
{
	
	CTriangleMesh::CVertexfield vertices{
		C3DFVector(1,5,8), C3DFVector(5,3,3), C3DFVector(3,4,1), C3DFVector(2,1,4)}; 
	
	typedef T3DVector<unsigned int> Triangle; 
	CTriangleMesh::CTrianglefield triangles{
		Triangle(0,1,2), Triangle(1,2,3), Triangle(0,2,3), Triangle(0,3,1)}; 
	copy(vertices.begin(), vertices.end(), mesh.vertices_begin()); 
	copy(triangles.begin(), triangles.end(), mesh.triangles_begin());
}

void MeshVtxsortFixture::test_expected(const CTriangleMesh& changed, const CTriangleMesh& test) const
{
	BOOST_CHECK_EQUAL(changed.vertices_size(), test.vertices_size()); 
	BOOST_CHECK_EQUAL(changed.triangle_size(), test.triangle_size()); 
	BOOST_CHECK_EQUAL(changed.get_available_data(), test.get_available_data()); 
	
	for(auto ic = changed.vertices_begin(), it = test.vertices_begin(); 
	    ic != changed.vertices_end(); ++ic, ++it) {
		BOOST_CHECK_EQUAL(*ic, *it); 
	}

	for(auto ic = changed.triangles_begin(), it = test.triangles_begin(); 
	    ic != changed.triangles_end(); ++ic, ++it) {
		BOOST_CHECK_EQUAL(*ic, *it); 
	}
	
	if (test.get_available_data() & CTriangleMesh::ed_normal) {
		for(auto ic = changed.normals_begin(), it = test.normals_begin(); 
		    ic != changed.normals_end(); ++ic, ++it) {
			BOOST_CHECK_EQUAL(*ic, *it); 
		}
	}

	if (test.get_available_data() & CTriangleMesh::ed_color){
		for(auto ic = changed.color_begin(), it = test.color_begin(); 
		    ic != changed.color_end(); ++ic, ++it) {
			BOOST_CHECK_EQUAL(*ic, *it); 
		}
	}
	
	if (test.get_available_data() & CTriangleMesh::ed_scale){
		for(auto ic = changed.scale_begin(), it = test.scale_begin(); 
		    ic != changed.scale_end(); ++ic, ++it) {
			BOOST_CHECK_EQUAL(*ic, *it); 
		}
	}
}


