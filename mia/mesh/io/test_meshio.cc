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

const char test_mesh_ply[] = "ply\n"
			       "format ascii 1.0\n"
			       "element vertex 6\n"
			       "property float32 x\n"
			       "property float32 y\n"
			       "property float32 z\n"
			       "element face 8\n"
			       "property list uint8 uint32 vertex_index\n"
			       "end_header\n"
			       "2 0 0\n"
			       "-2 0 0\n"
			       "0 2 0\n"
			       "0 -2 0\n"
			       "0 0 1\n"
			       "0 0 -1\n"
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

const char test_mesh_normals_colors_ply[] =
			       "ply\n"
			       "format ascii 1.0\n"
			       "element vertex 6\n"
			       "property float32 x\n"
			       "property float32 y\n"
			       "property float32 z\n"
			       "property float32 nx\n"
			       "property float32 ny\n"
			       "property float32 nz\n"
			       "property float32 red\n"
			       "property float32 green\n"
			       "property float32 blue\n"
			       "element face 8\n"
			       "property list uint8 uint32 vertex_index\n"
			       "end_header\n"
			       "2 0 0 1 0 0 0.1 0.5 0.5\n"
			       "-2 0 0 -1 0 0 0.2 0.6 0.7\n"
			       "0 2 0 0 1 0 0.3 0.6 0.7\n"
			       "0 -2 0 0 -1 0 0.4 0.6 0.7\n"
			       "0 0 1 0 0 1 0.5 0.6 0.7\n"
			       "0 0 -1 0 0 -1 0.6 0.6 0.7\n"
			       "3 4 0 2\n"
			       "3 4 2 1\n"
			       "3 4 1 3\n"
			       "3 4 3 0\n"
			       "3 5 2 0\n"
			       "3 5 1 2\n" 
			       "3 5 3 1\n"
			       "3 5 0 3\n"; 

const char test_mesh_normals_scale_ply[] =
			       "ply\n"
			       "format ascii 1.0\n"
			       "element vertex 6\n"
			       "property float32 x\n"
			       "property float32 y\n"
			       "property float32 z\n"
			       "property float32 nx\n"
			       "property float32 ny\n"
			       "property float32 nz\n"
			       "property float32 scale\n"
			       "element face 8\n"
			       "property list uint8 uint32 vertex_index\n"
			       "end_header\n"
			       "2 0 0 1 0 0 0.1\n"
			       "-2 0 0 -1 0 0 2.2\n"
			       "0 2 0 0 1 0 4.3\n"
			       "0 -2 0 0 -1 0 3.4\n"
			       "0 0 1 0 0 1 0.5\n"
			       "0 0 -1 0 0 -1 0.6\n"
			       "3 4 0 2\n"
			       "3 4 2 1\n"
			       "3 4 1 3\n"
			       "3 4 3 0\n"
			       "3 5 2 0\n"
			       "3 5 1 2\n" 
			       "3 5 3 1\n"
			       "3 5 0 3\n"; 


const char test_mesh_normals_ply[] =
			       "ply\n"
			       "format ascii 1.0\n"
			       "element vertex 6\n"
			       "property float32 x\n"
			       "property float32 y\n"
			       "property float32 z\n"
			       "property float32 nx\n"
			       "property float32 ny\n"
			       "property float32 nz\n"
			       "element face 8\n"
			       "property list uint8 uint32 vertex_index\n"
			       "end_header\n"
			       "2 0 0 1 0 0\n"
			       "-2 0 0 -1 0 0\n"
			       "0 2 0 0 1 0\n"
			       "0 -2 0 0 -1 0\n"
			       "0 0 1 0 0 1\n"
			       "0 0 -1 0 0 -1\n"
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



static void run_simple_octaedron_test(const char *in_file, const char *test_file, const string& test_string); 
static void run_octaedron_vertex_normal_test(const char *in_file, const char *test_file, const string& test_string); 
static void run_octaedron_vertex_normal_color_test(const char *in_file, const char *test_file, const string& test_string);
static void run_octaedron_vertex_normal_scale_test(const char *in_file, const char *test_file, const string& test_string); 


BOOST_AUTO_TEST_CASE( test_load_save_octaedron_off )
{
	run_simple_octaedron_test(MIA_SOURCE_ROOT"/testdata/octahedron.off", "testmesh.OFF", test_mesh_off); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_ply )
{
	run_simple_octaedron_test(MIA_SOURCE_ROOT"/testdata/octahedron.ply", "testmesh.ply", test_mesh_ply); 
}


BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_off )
{
	run_octaedron_vertex_normal_test(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals.off", "testmesh-n.OFF", test_mesh_normals_off); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_ply )
{
	run_octaedron_vertex_normal_test(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals.ply", "testmesh-n.PLY", test_mesh_normals_ply); 
}


BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_colors_off )
{
	run_octaedron_vertex_normal_color_test(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals-and-color.off",
					       "octahedron-nc.OFF", test_mesh_normals_colors_off); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_colors_ply )
{
	run_octaedron_vertex_normal_color_test(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals-and-color.ply",
					       "octahedron-nc.PLY", test_mesh_normals_colors_ply); 
}

BOOST_AUTO_TEST_CASE( test_load_save_octaedron_with_vertex_normals_scale_ply )
{

	run_octaedron_vertex_normal_scale_test(MIA_SOURCE_ROOT"/testdata/octahedron-with-normals-and-scale.ply",
					       "octahedron-ns.PLY", test_mesh_normals_scale_ply); 
}


BOOST_AUTO_TEST_CASE( test_load_off_errors )
{
        BOOST_CHECK_THROW(CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/4D.off"), invalid_argument);
        BOOST_CHECK_THROW(CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/ND.off"), invalid_argument);
	BOOST_CHECK_THROW(CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/vertex_error.off"), runtime_error);
}

set<C3DFVector, vector3d_less<C3DFVector> > test_poly_vertices = {
        C3DFVector(1,0,0), C3DFVector(1,1,0), 
        C3DFVector(2,1,0), C3DFVector(2,3,0)
};


BOOST_AUTO_TEST_CASE( test_load_off_poly )
{
	auto mesh = CMeshIOPluginHandler::instance().load(MIA_SOURCE_ROOT"/testdata/poly.off");
	test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_poly_vertices);
	BOOST_CHECK_EQUAL(mesh->triangle_size(), 2u);

	BOOST_CHECK_EQUAL(mesh->triangle_at(0), Triangle(2,3,0)); 
	BOOST_CHECK_EQUAL(mesh->triangle_at(1), Triangle(1,2,0));
	
}

static void run_simple_octaedron_test(const char *in_file, const char *test_file, const string& test_string)
{
        auto mesh = CMeshIOPluginHandler::instance().load(in_file);

	BOOST_CHECK_EQUAL(mesh->get_available_data(), CTriangleMesh::ed_vertex); 
	
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save(test_file, *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load(test_file);

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 

	// check file output 
	FILE *testfile = fopen(test_file, "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = test_string.length(); //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_string.c_str()));

	cvdebug() << "Read: '" << buffer << "'\n"; 
	
	unlink(test_file); 
}

	
static void run_octaedron_vertex_normal_test(const char *in_file, const char *test_file, const string& test_string)
{


        auto mesh = CMeshIOPluginHandler::instance().load(in_file);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 
        test_set_equal(mesh->normals_begin(), mesh->normals_end(), test_normals);

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save(test_file, *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load(test_file);

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 
        test_set_equal(mesh2->normals_begin(), mesh2->normals_end(), test_normals);

	// check file output 
	FILE *testfile = fopen(test_file, "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = test_string.length();
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_string.c_str()));

	cvdebug() << "Read: '" << buffer << "'\n";
	
	unlink(test_file); 
	
}

void run_octaedron_vertex_normal_color_test(const char *in_file, const char *test_file, const string& test_string)
{
	auto mesh = CMeshIOPluginHandler::instance().load(in_file);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 
        test_set_equal(mesh->normals_begin(), mesh->normals_end(), test_normals);
	test_set_equal(mesh->color_begin(), mesh->color_end(), test_colors);

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save(test_file, *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load(test_file);

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 
        test_set_equal(mesh2->normals_begin(), mesh2->normals_end(), test_normals);
	test_set_equal(mesh2->color_begin(), mesh2->color_end(), test_colors);

	// check file output 
	FILE *testfile = fopen(test_file, "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = test_string.length(); //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_string.c_str()));

	cvdebug() << "Read: '" << buffer << "'\n";
	
	unlink(test_file); 
}

void run_octaedron_vertex_normal_scale_test(const char *in_file, const char *test_file, const string& test_string)
{
	auto mesh = CMeshIOPluginHandler::instance().load(in_file);
        
        test_set_equal(mesh->vertices_begin(), mesh->vertices_end(), test_vertices);
        test_set_equal(mesh->triangles_begin(), mesh->triangles_end(), test_triangles); 
        test_set_equal(mesh->normals_begin(), mesh->normals_end(), test_normals);
	test_set_equal(mesh->scale_begin(), mesh->scale_end(), test_scale);

	BOOST_REQUIRE(CMeshIOPluginHandler::instance().save(test_file, *mesh));

	auto mesh2 = CMeshIOPluginHandler::instance().load(test_file);

        test_set_equal(mesh2->vertices_begin(), mesh2->vertices_end(), test_vertices);
        test_set_equal(mesh2->triangles_begin(), mesh2->triangles_end(), test_triangles); 
        test_set_equal(mesh2->normals_begin(), mesh2->normals_end(), test_normals);
	test_set_equal(mesh2->scale_begin(), mesh2->scale_end(), test_scale);

	// check file output 
	FILE *testfile = fopen(test_file, "r");
	BOOST_REQUIRE(testfile); 
	char buffer[2000];
	memset(buffer, 0, 2000); 
	size_t flen = test_string.length(); //don't count terminating 0 
	size_t read_bytes = fread(buffer, 1, 2000, testfile);
	fclose(testfile);

	BOOST_CHECK_EQUAL(read_bytes, flen);
	BOOST_CHECK(!strcmp(buffer, test_string.c_str()));

	cvdebug() << "Read: '" << buffer << "'\n";
	
	unlink(test_file); 
}
