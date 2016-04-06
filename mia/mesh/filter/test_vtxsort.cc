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

#include <mia/internal/plugintester.hh>
#include <mia/mesh/filter/vtxsort.hh>

using namespace mia; 
using namespace mia_meshfilter_vtxsort; 

class  MeshVtxsortFixture {
protected: 
	MeshVtxsortFixture(); 
	
	
	void test_expected(const CTriangleMesh& changed, const CTriangleMesh& test) const;
	
	typedef CTriangleMesh::CTrianglefield::value_type Triangle;
	typedef CTriangleMesh::CColorfield::value_type Color;
	CTriangleMesh mesh; 
}; 

class MeshVtxsortWithAttrFixture : public MeshVtxsortFixture{
protected:
	MeshVtxsortWithAttrFixture(); 
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

BOOST_FIXTURE_TEST_CASE( test_mesh_z_order_with_attr, MeshVtxsortWithAttrFixture) 
{
	auto testv = CTriangleMesh::PVertexfield(new CTriangleMesh::CVertexfield({
				C3DFVector(3,4,1), C3DFVector(5,3,3), 
					C3DFVector(2,1,4), C3DFVector(1,5,8)})); 

	auto testt = CTriangleMesh::PTrianglefield(new CTriangleMesh::CTrianglefield({
				Triangle(3,1,0), Triangle(1,0,2), 
					Triangle(3,0,2), Triangle(3,2,1)})); 

	auto testn = CTriangleMesh::PNormalfield(new CTriangleMesh::CNormalfield({
				C3DFVector(0,0,3), C3DFVector(0,0,2), 
				 C3DFVector(0,0,4), C3DFVector(0,0,1)
					})); 

	auto testc = CTriangleMesh::PColorfield(new CTriangleMesh::CColorfield({
				Color(1,0,0), Color(0,1,0), Color(1,1,0) , Color(0,0,1)
					})); 

	auto tests = CTriangleMesh::PScalefield(new CTriangleMesh::CScalefield({3,2,4,1})); 
	
	CTriangleMesh test(testt, testv, testn, testc, tests); 

	auto scale = BOOST_TEST_create_from_plugin<CVtxSortMeshFilterPlugin>("vtxsort:dir=[<0,0,1>]");
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

MeshVtxsortWithAttrFixture::MeshVtxsortWithAttrFixture()
{
	CTriangleMesh::CNormalfield nfield({C3DFVector(0,0,1), C3DFVector(0,0,2), 
				C3DFVector(0,0,3), C3DFVector(0,0,4)}); 
	CTriangleMesh::CColorfield cfield({Color(0,0,1), Color(0,1,0), Color(1,0,0), Color(1,1,0)});
	CTriangleMesh::CScalefield sfield({1,2,3,4}); 
	
	copy(nfield.begin(), nfield.end(), mesh.normals_begin()); 
	copy(cfield.begin(), cfield.end(), mesh.color_begin()); 
	copy(sfield.begin(), sfield.end(), mesh.scale_begin()); 

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


