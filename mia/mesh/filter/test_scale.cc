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
#include <mia/mesh/filter/scale.hh>

using namespace mia;
using namespace mia_meshfilter_scale;

class  MeshScaleFixture
{
protected:
       MeshScaleFixture();

       void test_triangles_unchanged(const CTriangleMesh& test) const;

       void test_expected_vertices(const CTriangleMesh& test,
                                   const CTriangleMesh::CVertexfield& vertices)const;

       CTriangleMesh mesh;
};


BOOST_FIXTURE_TEST_CASE (test_mesh_scale_iso_fit_box, MeshScaleFixture)
{
       auto scale = BOOST_TEST_create_from_plugin<CScaleMeshFilterPlugin>("scale:strategy=iso-fitbox,box-start=[<0,0,0>],box-end=[<256,256,256>]");
       auto filtered_mesh = scale->filter(mesh);
       test_triangles_unchanged(*filtered_mesh);
       CTriangleMesh::CVertexfield vertices{C3DFVector(128, 96, 0), C3DFVector(64, 160, 0), C3DFVector(64, 96, 64),
                                            C3DFVector(192, 160, 256)};
       test_expected_vertices(*filtered_mesh, vertices);
}

BOOST_FIXTURE_TEST_CASE (test_mesh_scale_aniso_fit_box, MeshScaleFixture)
{
       auto scale = BOOST_TEST_create_from_plugin<CScaleMeshFilterPlugin>("scale:strategy=aniso-fitbox,box-start=[<0,0,0>],box-end=[<256,256,256>]");
       auto filtered_mesh = scale->filter(mesh);
       test_triangles_unchanged(*filtered_mesh);
       CTriangleMesh::CVertexfield vertices{C3DFVector(128, 0, 0), C3DFVector(0, 256, 0), C3DFVector(0, 0, 64),
                                            C3DFVector(256, 256, 256)};
       test_expected_vertices(*filtered_mesh, vertices);
}


BOOST_FIXTURE_TEST_CASE (test_mesh_scale_linear_isotropic, MeshScaleFixture)
{
       auto scale = BOOST_TEST_create_from_plugin<CScaleMeshFilterPlugin>("scale:strategy=iso-linear,isoscale=2.0,shift=[<-1,2,5>]");
       auto filtered_mesh = scale->filter(mesh);
       test_triangles_unchanged(*filtered_mesh);
       CTriangleMesh::CVertexfield vertices{C3DFVector(1, 2, 5), C3DFVector(-1, 4, 5), C3DFVector(-1, 2, 7),
                                            C3DFVector(3, 4, 13)};
       test_expected_vertices(*filtered_mesh, vertices);
}

BOOST_FIXTURE_TEST_CASE (test_mesh_scale_linear_anisotropic, MeshScaleFixture)
{
       auto scale = BOOST_TEST_create_from_plugin<CScaleMeshFilterPlugin>("scale:strategy=aniso-linear,anisoscale=[<2.0,3.0,4>],shift=[<-1,2,5>]");
       auto filtered_mesh = scale->filter(mesh);
       test_triangles_unchanged(*filtered_mesh);
       CTriangleMesh::CVertexfield vertices{C3DFVector(1, 2, 5), C3DFVector(-1, 5, 5), C3DFVector(-1, 2, 9),
                                            C3DFVector(3, 5, 19)};
       test_expected_vertices(*filtered_mesh, vertices);
}


MeshScaleFixture::MeshScaleFixture():
       mesh(4, 4)
{
       CTriangleMesh::CVertexfield vertices{C3DFVector(1, 0, 0), C3DFVector(0, 1, 0), C3DFVector(0, 0, 1),
                                            C3DFVector(2, 1, 4)};
       typedef T3DVector<unsigned int> Triangle;
       CTriangleMesh::CTrianglefield triangles{Triangle(0, 1, 2), Triangle(1, 2, 3), Triangle(0, 2, 3), Triangle(2, 3, 0)};
       copy(vertices.begin(), vertices.end(), mesh.vertices_begin());
       copy(triangles.begin(), triangles.end(), mesh.triangles_begin());
}

void MeshScaleFixture::test_triangles_unchanged(const CTriangleMesh& test) const
{
       BOOST_REQUIRE(test.triangle_size() == 4u);

       for ( auto it = test.triangles_begin(), im = mesh.triangles_begin();
             it != test.triangles_end(); ++it, ++im) {
              BOOST_CHECK_EQUAL(*it, *im);
       }
}

void MeshScaleFixture::test_expected_vertices(const CTriangleMesh& test,
              const CTriangleMesh::CVertexfield& vertices)const
{
       BOOST_REQUIRE(test.vertices_size() == vertices.size());

       for ( auto it = test.vertices_begin(), iv = vertices.begin();
             it != test.vertices_end(); ++it, ++iv) {
              BOOST_CHECK_CLOSE(it->x, iv->x, 0.1);
              BOOST_CHECK_CLOSE(it->y, iv->y, 0.1);
              BOOST_CHECK_CLOSE(it->y, iv->y, 0.1);
       }
}
