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


#define VSTREAM_DOMAIN "vtkMeshIOtest"


#include <mia/internal/autotest.hh>
#include <mia/3d/imageio.hh>
#include <vtk/vtkmesh.hh>
#include <unistd.h>

using namespace vtkmia; 
using namespace mia; 
using namespace std; 

class  MeshVtkIOFixture {
protected: 
	MeshVtkIOFixture(); 
	
	
	void test_expected(const CTriangleMesh& changed, const CTriangleMesh& test) const;
	
	typedef CTriangleMesh::CTrianglefield::value_type Triangle;
	typedef CTriangleMesh::CColorfield::value_type Color;
	CTriangleMesh mesh; 
}; 


BOOST_FIXTURE_TEST_CASE(test_simple_store_and_load, MeshVtkIOFixture)
{
	CVtkMeshIO io; 
	BOOST_REQUIRE(io.save("testsavemesh.vtk", mesh));
	
	auto loaded_mesh = io.load("testsavemesh.vtk");
	BOOST_REQUIRE(loaded_mesh); 
	
	test_expected(*loaded_mesh, mesh);

}

BOOST_FIXTURE_TEST_CASE(test_store_and_load_with_all, MeshVtkIOFixture)
{
	CTriangleMesh::CNormalfield nfield({C3DFVector(2,3,1), C3DFVector(7,6,2), 
				C3DFVector(2,4,3), C3DFVector(5,6,4)}); 
	CTriangleMesh::CColorfield cfield({Color(2,3,1), Color(4,1,5), Color(1,6,7), Color(1,1,8)});
	CTriangleMesh::CScalefield sfield({1,2,3,4}); 
	
	copy(nfield.begin(), nfield.end(), mesh.normals_begin()); 
	copy(cfield.begin(), cfield.end(), mesh.color_begin()); 
	copy(sfield.begin(), sfield.end(), mesh.scale_begin()); 



	CVtkMeshIO io; 
	BOOST_REQUIRE(io.save("testsavemesh.vtk", mesh));
	
	auto loaded_mesh = io.load("testsavemesh.vtk");
	BOOST_REQUIRE(loaded_mesh); 
	
	test_expected(*loaded_mesh, mesh);
	
	unlink("testsavemesh.vtk"); 
}


MeshVtkIOFixture::MeshVtkIOFixture():mesh(4,4)
{
	
	CTriangleMesh::CVertexfield vertices{
		C3DFVector(1,5,8), C3DFVector(5,3,3), C3DFVector(3,4,1), C3DFVector(2,1,4)}; 
	
	typedef T3DVector<unsigned int> Triangle; 
	CTriangleMesh::CTrianglefield triangles{
		Triangle(0,1,2), Triangle(1,2,3), Triangle(0,2,3), Triangle(0,3,1)}; 
	copy(vertices.begin(), vertices.end(), mesh.vertices_begin()); 
	copy(triangles.begin(), triangles.end(), mesh.triangles_begin());
}

void MeshVtkIOFixture::test_expected(const CTriangleMesh& changed, const CTriangleMesh& test) const
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




