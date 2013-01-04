/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

const string kennung("ply");
const string kelement("element");
const string kvertex("vertex");
const string kface("face");
const string end_header("end_header");


class FReadElement {
	FILE *f;
public:
	FReadElement(FILE *_f):f(_f){};

	template <typename Element>
	void operator()(Element& elm) {
		char buffer[2048];
		if (!fgets(buffer, 2048, f))
			throw runtime_error("Read PLY: unable to read sufficient data");
		istringstream s(buffer);
		s >> elm.x >> elm.y >> elm.z;
	}
};


class FReadFloatElement {
	FILE *f;
public:
	FReadFloatElement(FILE *_f):f(_f){};
	void operator()(CTriangleMesh::CVertexfield::value_type& elm) {
		if (fscanf(f,"%f %f %f",&elm.x, &elm.y, &elm.z) != 3) {
			throw runtime_error("Read PLY: unable to read sufficient data");
		}
	}
};


PTriangleMesh TPlyMeshIO::do_load(string const &  filename) const
{
	char buffer[1024];
	int n_vertices = 0;
	int n_face = 0;

	CInputFile f(filename);
	if (!f)
		return PTriangleMesh();

	if ( (!fgets(buffer, 1024, f)) || strncmp(buffer,"ply",3))
		return PTriangleMesh();

	if ( (!fgets(buffer, 1024, f)) || (sscanf(buffer,"element vertex %d",&n_vertices)!= 1))
		return PTriangleMesh();

	if ( (!fgets(buffer, 1024, f)) || (sscanf(buffer,"element face %d",&n_face)!= 1))
		return PTriangleMesh();

	if ( (!fgets(buffer, 1024, f)) || strncmp(buffer,"end_header",10))
		return PTriangleMesh();


	CTriangleMesh::PVertexfield vertices(new  CTriangleMesh::CVertexfield(n_vertices));
	if (!vertices)
		return PTriangleMesh();

	CTriangleMesh::PTrianglefield triangles(new  CTriangleMesh::CTrianglefield(n_face));


	try {
		for_each(vertices->begin(), vertices->end(),FReadElement(f));
		for_each(triangles->begin(), triangles->end(),FReadElement(f));
	}
	catch (exception& e) {
		cverr() << e.what() << "\n";
        return PTriangleMesh();
	}

	return PTriangleMesh(new CTriangleMesh(triangles, vertices));
}

template <class T>
class FWriteElement {
	ostream& os;
public:
	FWriteElement(ostream& __os):os(__os){};
	void operator()(const T3DVector<T>& elm) {
		os << elm.x << " " <<  elm.y << " " <<  elm.z << "\n";
	}
};

bool TPlyMeshIO::do_save_it(const CTriangleMesh& mesh, ostream& os)const
{
	os << "ply\n";
	os << "element vertex " <<  mesh.vertices_size() << "\n";
	os << "element face " << mesh.triangle_size() << "\n";
	os << "end_header\n";

	for_each(mesh.vertices_begin(),mesh.vertices_end(),   FWriteElement<float>(os));
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





