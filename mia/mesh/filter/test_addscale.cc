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
#include <mia/mesh/filter/addscale.hh>
#include <mia/3d/imageio.hh>

using namespace mia; 
using namespace mia_meshfilter_addscale; 

struct DefineMeshFixture {

	DefineMeshFixture(); 

	PTriangleMesh mesh; 
}; 

BOOST_FIXTURE_TEST_CASE(test_add_scale, DefineMeshFixture) 
{
	C3DFImage *scales = new C3DFImage(C3DBounds(3,3,3)); 
	P3DImage pimg(scales); 
	
	float values[27] = { 1, 2, 3,  4, 5, 6,  7, 8, 9, 
			     11, 12, 13,   14, 15, 16,   17, 18, 19, 
			     31, 32, 33,   34, 35, 36,   37, 38, 39}; 

	copy(values, values+27, scales->begin()); 

	save_image("scales.@", pimg); 
	
	auto add = BOOST_TEST_create_from_plugin<CAddScaleMeshFilterPlugin>("addscale:img=scales.@");
	
	auto new_mesh = add->filter(*mesh); 
	
	float expect[] = {2.5f, 1.5f, 5.5f, 6.5f, 21.f, 11.5f}; 
	
	BOOST_CHECK_EQUAL(new_mesh->vertices_size(), 6u); 
	BOOST_CHECK_EQUAL(new_mesh->triangle_size(), 8u); 

	for (unsigned i = 0; i < 6; ++i) {
		BOOST_CHECK_CLOSE(new_mesh->scale_at(i), expect[i], 0.01); 
		BOOST_CHECK_EQUAL(new_mesh->vertex_at(i), mesh->vertex_at(i));
	}
	for (unsigned i = 0; i < 8; ++i) {
		BOOST_CHECK_EQUAL(new_mesh->triangle_at(i), mesh->triangle_at(i));
	}
}

DefineMeshFixture::DefineMeshFixture()
{
	auto vertices = CTriangleMesh::PVertexfield(
		new CTriangleMesh::CVertexfield(
			{C3DFVector(1.5,0,0), C3DFVector(0.5,0,0), 
					C3DFVector(0,1.5,0), C3DFVector(1,1.5,0), 
					C3DFVector(0,0,1.5), C3DFVector(0.5,0,1)})); 
	
	typedef CTriangleMesh::triangle_type Triangle; 
	
	auto triangles = CTriangleMesh::PTrianglefield(
		new CTriangleMesh::CTrianglefield(
			{Triangle(4, 0, 2), Triangle(4, 2, 1), 
					Triangle(4, 1, 3), Triangle(4, 3, 0), 
					Triangle(5, 2, 0), Triangle(5, 1, 2), 
					Triangle(5, 3, 1), Triangle(5, 0, 3)})); 
	
	mesh = std::make_shared<CTriangleMesh>(triangles, vertices); 
	
}


