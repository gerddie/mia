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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstring>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/mesh/triangulate.hh>

NS_BEGIN(ply_mesh_io)

NS_MIA_USE;
using namespace std;

static char const * const format = "ply";

class TPlyMeshIO: public CMeshIOPlugin {
public:
	TPlyMeshIO();

private:
	virtual PTriangleMesh do_load(string const &  filename)const;
	virtual bool do_save(string const &  filename, const CTriangleMesh& data)const;

	virtual const std::string  do_get_descr()const;
	bool do_save_it(const CTriangleMesh& mesh, std::ostream& os)const;
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new TPlyMeshIO();
}

TPlyMeshIO::TPlyMeshIO():CMeshIOPlugin(format)
{
	add_suffix(".ply");
	add_suffix(".PLY");

}

const string  TPlyMeshIO::do_get_descr()const
{
	return string("Ply triangle mesh input/output support");
}

// read the next line and skip comments
// 
static void get_line(char *buffer, string const &  filename, FILE *file)
{
	do {
		if (!fgets(buffer, 2048, file)) {
			throw create_exception<runtime_error>("Ply: Bougus file '", filename, "'");		
		}
	} while (!strncmp(buffer, "comment ", 8));
	cvdebug() << "Read line '"  << buffer << "'\n"; 
	
}


static vector<pair<string, vector<string>> > get_properties(char *buffer, string const &filename, FILE *file)
{
	vector<pair<string, vector<string>>> result;
	while (!strncmp(buffer, "property ", 9)) {
		istringstream is(buffer);
		string key, name, type;  
		is >> key >> type;
		if (type == "list") {
			vector<string> s;
			while (is.eof()) {
				is >> name;
				if (!name.empty()) 
					s.push_back(name); 
			}
		}else{ 
			is >> name;
			result.push_back(make_pair(type, vector<string>{name}));
		}
		get_line(buffer, filename, file); 
	}
	return result; 
}

map<string, int> key_flag_mapping = {
	{"x", 1},
	{"y", 2},
	{"z", 4},
	{"nx", 8},
	{"ny",16},
	{"nz",32},
	{"red", 64},
	{"green",128},
	{"blue",256},
	{"scale",512}}; 

void read_vertex_data(CTriangleMesh::CVertexfield& v,
		      CTriangleMesh::PNormalfield n, 
		      CTriangleMesh::PColorfield c,
		      CTriangleMesh::PScalefield s,
		      char *buffer, const string&  filename, FILE *file)
{
	for (unsigned i = 0; i < v.size(); ++i) {
		get_line(buffer, filename, file);
		
		istringstream buf(buffer);

		buf >> v[i].x >> v[i].y >> v[i].z;
		if (n) {
			buf >> (*n)[i].x >> (*n)[i].y >> (*n)[i].z;
		}
		if (c) {
			buf >> (*c)[i].x >> (*c)[i].y >> (*c)[i].z;
		}
		if (s) {
			buf >> (*s)[i];
		}
	}
}

void read_faces(CTriangleMesh::CTrianglefield& triangles, unsigned n_faces, 
		const CTriangleMesh::CVertexfield& vertices,
		char *buffer, const string&  filename, FILE *file)
{
	unsigned count;
	vector<unsigned> v(3);
	typedef TPolyTriangulator<CTriangleMesh::CVertexfield, vector<unsigned int> >  CPolyTriangulator;
	
	CPolyTriangulator triangulator(vertices); 
	
	for (unsigned i = 0; i < n_faces; ++i) {
		get_line(buffer, filename, file);

		istringstream buf(buffer);
		buf >> count;
		if (count > v.max_size()) {
			throw create_exception<runtime_error>("PLY: ", filename, ": ", count, " vertices specified ",
							      "but implementation only supports up to ", v.max_size()); 
		}
		
		v.resize(count);
		for (unsigned i = 0; i < count; ++i) {
			buf >> v[i];
		}
		if (count < 3) {
			cvwarn() << "PLY_Face with less than 3 vertices in '" << filename << "' ignoring";
		}
		if (count > 3) {
			vector<CTriangleMesh::triangle_type> tri; 
			triangulator.triangulate(tri, v);
			for (auto t: tri)
				triangles.push_back(t); 
		}else{
			cvdebug() << "Read triangle <" << v << "> from buffer '" << buffer << "'\n"; 
			triangles.push_back(CTriangleMesh::triangle_type(v[0], v[1], v[2])); 
		}
	}
}

pair<int, map<string, int>> get_data_flags(const vector<pair<string, vector<string>> >& vertex_properties,
					   const string& filename)
{
	int available_flags = 0;
	
	map<string, int> mapping; 
	for (unsigned i = 0; i < vertex_properties.size(); ++i) {
		mapping[vertex_properties[i].first] = i;

		auto key_flag = key_flag_mapping.find(vertex_properties[i].second[0]); 
		if (key_flag != key_flag_mapping.end()) {
			available_flags |= key_flag->second;
			cvdebug() << "available_flags= " << available_flags << " with key " << vertex_properties[i].second[0] << "\n"; 
		} else
			cvwarn() << "PLY: unsupported property '" << vertex_properties[i].second[0]
				 << "' found in '" << filename << ", ignoring\n";
	}

	int type_flags = 0; 
	if ((available_flags  & 7) == 7)
		type_flags |= CTriangleMesh::ed_vertex;
	
	if ((available_flags  & 0x38) == 0x38)
		type_flags |= CTriangleMesh::ed_normal; 

	if ((available_flags  & 0x1C0) == 0x1C0)
		type_flags |= CTriangleMesh::ed_color;

	if ((available_flags  & 0x200) == 0x200)
		type_flags |= CTriangleMesh::ed_scale;
	
	return make_pair(type_flags, mapping); 
	
}

PTriangleMesh TPlyMeshIO::do_load(string const &  filename) const
{
	cvdebug() << "Load as PLY?\n"; 
	char buffer[2048];
	int n_vertices = 0;
	unsigned n_face = 0;

	CInputFile f(filename);
	if (!f)
		return PTriangleMesh();

	if ( (!fgets(buffer, 1024, f)) || strncmp(buffer,"ply",3)) {
		cvdebug() << "Not a PLY file\n"; 
		return PTriangleMesh();
	}

	if ( (!fgets(buffer, 1024, f)) || strncmp(buffer,"format ",7)) {
		throw create_exception<runtime_error>("Ply: Format specifier missing '", filename, "'");
	}

	// now we must check the type of file, only support ascii though
	
	get_line(buffer, filename, f);
	
	if (sscanf(buffer,"element vertex %d",&n_vertices)!= 1)
		throw create_exception<runtime_error>("Ply: Bougus file '", filename,
						      "', can't parse vertex count from '",buffer,"'.");
	
	get_line(buffer, filename, f);

	auto vertex_properties = get_properties(buffer, filename, f); 

	if (sscanf(buffer,"element face %d",&n_face)!= 1)
		throw create_exception<runtime_error>("Ply: Unsupported file '", filename, "', can't parse face count from '",
						      buffer, "'");

	get_line(buffer, filename, f);
	auto face_properties = get_properties(buffer, filename, f); 

	if (strncmp(buffer,"end_header",10))
		throw create_exception<runtime_error>("Ply: Header end marker not found in '", filename, "'.");


	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(n_vertices));
	CTriangleMesh::PNormalfield normals;
	CTriangleMesh::PColorfield colors;
	CTriangleMesh::PScalefield scales; 

	auto flags_and_map = get_data_flags(vertex_properties, filename);
	auto flags = flags_and_map.first; 
	
	if (!(flags & CTriangleMesh::ed_vertex))
		throw create_exception<runtime_error>("Ply: No supported vertex properties found in '", filename, "'.");

	if (flags & CTriangleMesh::ed_normal) {
		cvdebug() << "PLY: have normals\n"; 
		normals.reset(new CTriangleMesh::CNormalfield(n_vertices));
	}
	if (flags & CTriangleMesh::ed_color)
		colors.reset(new CTriangleMesh::CColorfield(n_vertices));
	if (flags & CTriangleMesh::ed_scale)
		scales.reset(new CTriangleMesh::CScalefield(n_vertices));

	read_vertex_data(*vertices, normals, colors, scales, buffer, filename, f);
	
	CTriangleMesh::PTrianglefield triangles(new  CTriangleMesh::CTrianglefield);

	if (n_face > triangles->max_size()) {
		throw create_exception<runtime_error>("PLY: ", filename, ": ", n_face, " triangles specified ",
						      "but implementation only supports up to ", triangles->max_size()); 
	}
		
	triangles->reserve(n_face); 

	read_faces(*triangles, n_face, *vertices, buffer, filename, f);

	return PTriangleMesh(new CTriangleMesh(triangles, vertices, normals, colors, scales));
}

template <class T>
class FWriteElement {
	ostream& os;
public:
	FWriteElement(ostream& __os):os(__os){};
	void operator()(const T3DVector<T>& elm) {
		os  <<  "3 " << elm.x << " " <<  elm.y << " " <<  elm.z << "\n";
	}
};

class CElementWriter {
public: 
	CElementWriter(): m_next(nullptr) {}
	void write(const CTriangleMesh& mesh, ostream& os, unsigned i)  {
		do_write(mesh, os, i);
		if (m_next) {
			os << " "; 
			m_next->write(mesh, os, i);
		}
	}

	void append(CElementWriter *w) {
		if (m_next)
			m_next->append(w);
		else
			m_next.reset(w); 
	}
private:
	virtual void do_write(const CTriangleMesh& mesh, ostream& os, unsigned i) = 0; 
	unique_ptr<CElementWriter> m_next; 
}; 

class CVertexWriter : public CElementWriter{
	virtual void do_write(const CTriangleMesh& mesh, ostream& os, unsigned i) {
		auto v = mesh.vertex_at(i);
		os << v.x << " " << v.y << " " << v.z; 
	}
}; 

class CNormalWriter : public CElementWriter{
	virtual void do_write(const CTriangleMesh& mesh, ostream& os, unsigned i) {
		auto v = mesh.normal_at(i);
		os << v.x << " " << v.y << " " << v.z; 
	}
}; 

class CColorWriter : public CElementWriter{
	virtual void do_write(const CTriangleMesh& mesh, ostream& os, unsigned i) {
		auto v = mesh.color_at(i);
		os << v.x << " " << v.y << " " << v.z; 
	}
}; 

class CScaleWriter : public CElementWriter{
	virtual void do_write(const CTriangleMesh& mesh, ostream& os, unsigned i) {
		auto v = mesh.scale_at(i);
		os << v; 
	}
}; 


bool TPlyMeshIO::do_save_it(const CTriangleMesh& mesh, ostream& os)const
{
	os << "ply\n";
	os << "format ascii 1.0\n";
	os << "element vertex " <<  mesh.vertices_size() << "\n";
	// add properties
	os << "property float32 x\n";
	os << "property float32 y\n";
	os << "property float32 z\n";

	CVertexWriter writer; 
	
	if (mesh.get_available_data() & CTriangleMesh::ed_normal) {
		os << "property float32 nx\n";
		os << "property float32 ny\n";
		os << "property float32 nz\n";
		writer.append(new CNormalWriter); 
	}
	
	if (mesh.get_available_data() & CTriangleMesh::ed_color) {
		os << "property float32 red\n";
		os << "property float32 green\n";
		os << "property float32 blue\n";
		writer.append(new CColorWriter); 
	}
	
	if (mesh.get_available_data() & CTriangleMesh::ed_scale) {
		os << "property float32 scale\n";
		writer.append(new CScaleWriter); 
	}
	os << "element face " << mesh.triangle_size() << "\n";
	os << "property list uint8 uint32 vertex_index\n"; 
	os << "end_header\n";

	for (unsigned i = 0; i < mesh.vertices_size(); ++i) {
		writer.write(mesh, os, i);
		os << "\n"; 
	}

	for_each(mesh.triangles_begin(),mesh.triangles_end(), FWriteElement<unsigned int>(os));
	
	return true;
}

bool TPlyMeshIO::do_save(string const &  filename, const CTriangleMesh& mesh) const
{
	if ( filename == "-")
		return do_save_it(mesh, cout);
	else {
		ofstream os(filename.c_str());
		return do_save_it(mesh, os);
	}

}

NS_END





