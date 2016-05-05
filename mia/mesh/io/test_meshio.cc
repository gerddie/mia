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
        C3DFVector(0.5, 0.6, 0.7), C3DFVector(0.6, 0.6, 0.7)
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

const char test_mesh_off[] =
		"OFF\n"
		"\n"
		"6 8 0\n"
		"2.000000 0.000000 0.000000\n"
		"-2.000000 0.000000 0.000000\n"
		"0.000000 2.000000 0.000000\n"
		"0.000000 -2.000000 0.000000\n"
		"0.000000 0.000000 1.000000\n"
		"0.000000 0.000000 -1.000000\n"
		"3 4 0 2\n"
		"3 4 2 1\n"
		"3 4 1 3\n"
		"3 4 3 0\n"
		"3 5 2 0\n"
		"3 5 1 2\n"
		"3 5 3 1\n"
		"3 5 0 3\n"; 

const char test_mesh_normals_off[] =
	"NOFF\n"
		"\n"
		"6 8 0\n"
		"2.000000 0.000000 0.000000 1.000000 0.000000 0.000000\n"
		"-2.000000 0.000000 0.000000 -1.000000 0.000000 0.000000\n"
		"0.000000 2.000000 0.000000 0.000000 1.000000 0.000000\n"
		"0.000000 -2.000000 0.000000 0.000000 -1.000000 0.000000\n"
		"0.000000 0.000000 1.000000 0.000000 0.000000 1.000000\n"
		"0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000\n"
		"3 4 0 2\n"
		"3 4 2 1\n"
		"3 4 1 3\n"
		"3 4 3 0\n"
		"3 5 2 0\n"
		"3 5 1 2\n"
		"3 5 3 1\n"
		"3 5 0 3\n";
const char test_mesh_normals_colors_off[] = 
			       "CNOFF\n"
			       "\n"
			       "6 8 0\n"
			       "2.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.100000 0.500000 0.500000\n"
			       "-2.000000 0.000000 0.000000 -1.000000 0.000000 0.000000 0.200000 0.600000 0.700000\n"
			       "0.000000 2.000000 0.000000 0.000000 1.000000 0.000000 0.300000 0.600000 0.700000\n"
			       "0.000000 -2.000000 0.000000 0.000000 -1.000000 0.000000 0.400000 0.600000 0.700000\n"
			       "0.000000 0.000000 1.000000 0.000000 0.000000 1.000000 0.500000 0.600000 0.700000\n"
			       "0.000000 0.000000 -1.000000 0.000000 0.000000 -1.000000 0.600000 0.600000 0.700000\n"
			       "3 4 0 2\n"
			       "3 4 2 1\n"
			       "3 4 1 3\n"
			       "3 4 3 0\n"
			       "3 5 2 0\n"
			       "3 5 1 2\n"
			       "3 5 3 1\n"
			       "3 5 0 3\n"; 



BOOST_AUTO_TEST_CASE( test_load_save_octaedron_off )
{
        string filename(MIA_SOURCE_ROOT"/testdata/octahedron.off");

        auto mesh = CMeshIOPluginHandler::instance().load(filename);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save("testmesh.OFF", *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load("testmesh.OFF");

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 

	// check file output 
	FILE *testfile = fopen("testmesh.OFF", "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = sizeof(test_mesh_off) - 1; //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_mesh_off));

	cvdebug() << "Read: '" << buffer << "'\n"; 
	
	unlink("testmesh.OFF"); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_off )
{
        string filename(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals.off");

        auto mesh = CMeshIOPluginHandler::instance().load(filename);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 
        test_set_equal(mesh->normals_begin(), mesh->normals_end(), test_normals);

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save("testmesh-n.OFF", *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load("testmesh-n.OFF");

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 
        test_set_equal(mesh2->normals_begin(), mesh2->normals_end(), test_normals);

	// check file output 
	FILE *testfile = fopen("testmesh-n.OFF", "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = sizeof(test_mesh_normals_off) - 1; //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_mesh_normals_off));

	cvdebug() << "Read: '" << buffer << "'\n";
	
	unlink("testmesh-n.OFF"); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_colors_off )
{
        string filename(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals-and-color.off");

        auto mesh = CMeshIOPluginHandler::instance().load(filename);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 
        test_set_equal(mesh->normals_begin(), mesh->normals_end(), test_normals);
	test_set_equal(mesh->color_begin(), mesh->color_end(), test_colors);

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save("testmesh-c.OFF", *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load("testmesh-c.OFF");

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 
        test_set_equal(mesh2->normals_begin(), mesh2->normals_end(), test_normals);
	test_set_equal(mesh2->color_begin(), mesh2->color_end(), test_colors);

	// check file output 
	FILE *testfile = fopen("testmesh-c.OFF", "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = sizeof(test_mesh_normals_colors_off) - 1; //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_mesh_normals_colors_off));

	cvdebug() << "Read: '" << buffer << "'\n";
	
	unlink("testmesh-c.OFF"); 
}


BOOST_AUTO_TEST_CASE( test_load_off_errors )
{
        BOOST_CHECK_THROW(CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/4D.off"), invalid_argument);
        BOOST_CHECK_THROW(CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/ND.off"), invalid_argument);
}


	
