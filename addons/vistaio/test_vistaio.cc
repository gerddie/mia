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

// since vista meshes are not defined elsewhere we only need to test that
// a loading-saving roundtrip gives the same result

#define VSTREAM_DOMAIN "test_vistavmesh"

#include <mia/internal/autotest.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/2d/vfio.hh>
#include <mia/3d/vfio.hh>

#include <set>
#include <ostream>


using namespace mia;
using namespace std;


template <typename V>
struct vector3d_less {
	bool operator()( const V& lhs, const V& rhs ) const {
		return (lhs.x < rhs.x) || (lhs.x == rhs.x && (lhs.y < rhs.y || (lhs.y == rhs.y && lhs.z < rhs.z))); 
	}
}; 

set<C3DFVector, vector3d_less<C3DFVector> > test_vertices = {
        C3DFVector(2,0,0), C3DFVector(-2,0,0), 
        C3DFVector(0,2,0), C3DFVector(0,-2,0), 
        C3DFVector(0,0,1), C3DFVector(0,0,-1)
};

set<C3DFVector, vector3d_less<C3DFVector> > test_normals = {
        C3DFVector(1,0,0), C3DFVector(-1,0,0), 
        C3DFVector(0,1,0), C3DFVector(0,-1,0), 
        C3DFVector(0,0,1), C3DFVector(0,0,-1)
};

set<C3DFVector, vector3d_less<C3DFVector> > test_colors = {
        C3DFVector(0.1, 0.5, 0.5), C3DFVector(0.2, 0.6, 0.7), 
        C3DFVector(0.3, 0.6, 0.7), C3DFVector(0.4, 0.6, 0.7), 
        C3DFVector(0.5, 0.6, 0.7), C3DFVector(0.6, 0.6, 1)
};

set<float> test_scale = {
	0.1, 2.2, 4.3, 3.4, 0.5, 0.6
};

typedef CTriangleMesh::triangle_type Triangle; 

set<Triangle, vector3d_less<Triangle>>  test_triangles = 
        {Triangle(4, 0, 2), Triangle(4, 2, 1), 
         Triangle(4, 1, 3), Triangle(4, 3, 0), 
         Triangle(5, 2, 0), Triangle(5, 1, 2), 
         Triangle(5, 3, 1), Triangle(5, 0, 3)};


template <typename T>
ostream& operator << (ostream& os, const set<T, vector3d_less<T>>& s)
{
	os << "set:[";
	for (auto x: s)
		os << "<" << x << "> ";
	os << "]\n";
	return os; 
}


template <typename IT, typename T> 
void test_set_equal(const IT begin, IT end,
		    const set<T, vector3d_less<T>>& expect)
{
        set<T, vector3d_less<T>> loaded(begin, end);
        BOOST_CHECK_EQUAL(loaded.size(), expect.size());

	BOOST_CHECK(loaded == expect);
	cvdebug() << loaded << "\n";
	cvdebug() << expect << "\n"; 
        
}

template <typename IT, typename T> 
void test_set_equal(const IT begin, IT end,
		    const set<T>& expect)
{
        set<T> loaded(begin, end);
        BOOST_CHECK_EQUAL(loaded.size(), expect.size());

	BOOST_CHECK(loaded == expect);
}

struct CVistaMeshtestFixtureV {

        CVistaMeshtestFixtureV();

        virtual void test_loaded( const CTriangleMesh& loaded);  

        void run(const char *testfilename); 
        
        PTriangleMesh mesh;
}; 

CVistaMeshtestFixtureV::CVistaMeshtestFixtureV()
{
        auto vertices = CTriangleMesh::PVertexfield(
                          new CTriangleMesh::CVertexfield(test_vertices.begin(), test_vertices.end()));

        auto triangles = CTriangleMesh::PTrianglefield(
                          new CTriangleMesh::CTrianglefield(test_triangles.begin(), test_triangles.end()));
        mesh.reset(new CTriangleMesh(triangles, vertices)); 
}

void CVistaMeshtestFixtureV::test_loaded( const CTriangleMesh& loaded)
{
        BOOST_CHECK_EQUAL(loaded.get_available_data(), mesh->get_available_data()); 
        
        test_set_equal(loaded.vertices_begin(), loaded.vertices_end(), test_vertices);
        test_set_equal(loaded.triangles_begin(), loaded.triangles_end(), test_triangles); 
}

void CVistaMeshtestFixtureV::run(const char *testfilename)
{
        BOOST_REQUIRE(CMeshIOPluginHandler::instance().save(testfilename, *mesh));
        auto loaded_mesh = CMeshIOPluginHandler::instance().load(testfilename);
        BOOST_REQUIRE(loaded_mesh);
        test_loaded(*loaded_mesh); 
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_v, CVistaMeshtestFixtureV)
{
        run("mesh-v.vmesh"); 
}


struct CVistaMeshtestFixtureVN : public CVistaMeshtestFixtureV{
        CVistaMeshtestFixtureVN();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVN::CVistaMeshtestFixtureVN()
{
        copy(test_normals.begin(), test_normals.end(), mesh->normals_begin()); 
}

void CVistaMeshtestFixtureVN::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureV::test_loaded(loaded);

        test_set_equal(loaded.normals_begin(), loaded.normals_end(), test_normals);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vn, CVistaMeshtestFixtureVN)
{
        run("mesh-vn.vmesh"); 
}




struct CVistaMeshtestFixtureVNC : public CVistaMeshtestFixtureVN{
        CVistaMeshtestFixtureVNC();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVNC::CVistaMeshtestFixtureVNC()
{
        copy(test_colors.begin(), test_colors.end(), mesh->color_begin()); 
}

void CVistaMeshtestFixtureVNC::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureVN::test_loaded(loaded);

        test_set_equal(loaded.color_begin(), loaded.color_end(), test_colors);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vnc, CVistaMeshtestFixtureVNC)
{
        run("mesh-vnc.vmesh"); 
}


struct CVistaMeshtestFixtureVC : public CVistaMeshtestFixtureV{
        CVistaMeshtestFixtureVC();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVC::CVistaMeshtestFixtureVC()
{
        copy(test_colors.begin(), test_colors.end(), mesh->color_begin()); 
}

void CVistaMeshtestFixtureVC::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureV::test_loaded(loaded);

        test_set_equal(loaded.color_begin(), loaded.color_end(), test_colors);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vc, CVistaMeshtestFixtureVC)
{
        run("mesh-vc.vmesh"); 
}


struct CVistaMeshtestFixtureVS : public CVistaMeshtestFixtureV{
        CVistaMeshtestFixtureVS();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVS::CVistaMeshtestFixtureVS()
{
        copy(test_scale.begin(), test_scale.end(), mesh->scale_begin()); 
}

void CVistaMeshtestFixtureVS::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureV::test_loaded(loaded);

        test_set_equal(loaded.scale_begin(), loaded.scale_end(), test_scale);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vs, CVistaMeshtestFixtureVS)
{
        run("mesh-vs.vmesh"); 
}


struct CVistaMeshtestFixtureVCS : public CVistaMeshtestFixtureVC{
        CVistaMeshtestFixtureVCS();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVCS::CVistaMeshtestFixtureVCS()
{
        copy(test_scale.begin(), test_scale.end(), mesh->scale_begin()); 
}

void CVistaMeshtestFixtureVCS::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureVC::test_loaded(loaded);

        test_set_equal(loaded.scale_begin(), loaded.scale_end(), test_scale);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vcs, CVistaMeshtestFixtureVCS)
{
        run("mesh-vcs.vmesh"); 
}

struct CVistaMeshtestFixtureVNCS : public CVistaMeshtestFixtureVNC{
        CVistaMeshtestFixtureVNCS();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVNCS::CVistaMeshtestFixtureVNCS()
{
        copy(test_scale.begin(), test_scale.end(), mesh->scale_begin()); 
}

void CVistaMeshtestFixtureVNCS::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureVNC::test_loaded(loaded);

        test_set_equal(loaded.scale_begin(), loaded.scale_end(), test_scale);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vncs, CVistaMeshtestFixtureVNCS)
{
        run("mesh-vncs.vmesh"); 
}


struct CVistaMeshtestFixtureVNS : public CVistaMeshtestFixtureVN {
        CVistaMeshtestFixtureVNS();
        virtual void test_loaded( const CTriangleMesh& loaded);  
};

CVistaMeshtestFixtureVNS::CVistaMeshtestFixtureVNS()
{
        copy(test_scale.begin(), test_scale.end(), mesh->scale_begin()); 
}

void CVistaMeshtestFixtureVNS::test_loaded( const CTriangleMesh& loaded)
{
        CVistaMeshtestFixtureVN::test_loaded(loaded);

        test_set_equal(loaded.scale_begin(), loaded.scale_end(), test_scale);
}

BOOST_FIXTURE_TEST_CASE( test_vista_meshio_vns, CVistaMeshtestFixtureVNS)
{
        run("mesh-vns.vmesh"); 
}



BOOST_AUTO_TEST_CASE ( test_3dvfio )
{
        C3DBounds size(1,2,3); 
        C3DFVectorfield vf(size);

        vector<C3DFVector> test_data = {
                C3DFVector(1, 2, 3), C3DFVector(2, 3, 4), C3DFVector(3, 4, 5),
                C3DFVector(4, 5, 6), C3DFVector(5, 6, 7), C3DFVector(7, 8, 9)
        };

        copy(test_data.begin(), test_data.end(), vf.begin()); 

        C3DIOVectorfield iovf(vf); 
        BOOST_REQUIRE(C3DVFIOPluginHandler::instance().save("3d.vf", iovf));

        auto loaded = C3DVFIOPluginHandler::instance().load("3d.vf");
        BOOST_REQUIRE(loaded);
        
        BOOST_CHECK_EQUAL(loaded->get_size(), size);
        auto i = loaded->begin();
        auto e = loaded->end();
        auto t = vf.begin();

        while (i != e) {
                BOOST_CHECK_EQUAL(*i, *t);
                ++i; ++t; 
        }
}

BOOST_AUTO_TEST_CASE ( test_2dvfio )
{
        C2DBounds size(2,3); 
        C2DFVectorfield vf(size);

        vector<C2DFVector> test_data = {
                C2DFVector(2, 3), C2DFVector(3, 4), C2DFVector(4, 5),
                C2DFVector(5, 6), C2DFVector(6, 7), C2DFVector(8, 9)
        };

        copy(test_data.begin(), test_data.end(), vf.begin()); 

        C2DIOVectorfield iovf(vf); 
        BOOST_REQUIRE(C2DVFIOPluginHandler::instance().save("2d.vf", iovf));

        auto loaded = C2DVFIOPluginHandler::instance().load("2d.vf");
        BOOST_REQUIRE(loaded);
        
        BOOST_CHECK_EQUAL(loaded->get_size(), size);
        auto i = loaded->begin();
        auto e = loaded->end();
        auto t = vf.begin();

        while (i != e) {
                BOOST_CHECK_EQUAL(*i, *t);
                ++i; ++t; 
        }
}





