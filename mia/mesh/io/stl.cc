/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <map>
#include <fstream>
#include <iostream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/mesh/triangularMesh.hh>


NS_BEGIN(stl_mesh_io)

NS_MIA_USE;
using namespace std;


static char const * const format = "stl";

class CSTLMeshIO: public CMeshIOPlugin {
public:
	CSTLMeshIO();

private:
	virtual PTriangleMesh do_load(string const &  filename)const;
	virtual bool do_save(string const &  filename, const CTriangleMesh& data)const;
	virtual const std::string  do_get_descr()const;

	struct vless {
		bool operator ()(const C3DFVector& a, const C3DFVector& b)const
		{
			return (a.z < b.z) ||
				((a.z == b.z) && ((a.y < b.y) ||
						  ((a.y == b.y) && (a.x < b.x))
						  )
				 );
		}
	};

	typedef map<CTriangleMesh::vertex_type, size_t, vless> CVertexMap;
	PTriangleMesh load_ascii(istream& f)const;
	bool read_vertex(CVertexMap& vmap, unsigned int& index, istream& f)const;
	bool save_ascii(ostream& of, const CTriangleMesh& data)const;
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
};


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CSTLMeshIO;
}

const string start_solid("solid");
const string end_solid("endsolid");
const string start_face("facet");
const string end_face("endfacet");
const string vertex_tag("vertex");
const string normal_tag("normal");
const string start_loop("outer");
const string tag_loop("loop");
const string end_loop("endloop");

CSTLMeshIO::CSTLMeshIO():
	CMeshIOPlugin(format)
{
}

bool CSTLMeshIO::read_vertex(CVertexMap& vmap, unsigned int& index, istream& f)const
{
	string tag;
	C3DFVector vertex;

	f >> tag >> vertex.x >> vertex.y >> vertex.z;
	if (tag != vertex_tag) {
		cverr() << "not a valid ascii STL file\n";
		return false;
	}
	cvdebug() << "Read vertex: " << vertex << "\n";

	CVertexMap::const_iterator v = vmap.find(vertex);

	if (v != vmap.end()) {
		index = v->second;
	}else {
		index = vmap.size();
		vmap[vertex] = index;
	}
	return true;
}

PTriangleMesh CSTLMeshIO::load_ascii(istream& f)const
{
	string attribute;
	string tag;
	string solid_name;
	CVertexMap vmap;


	vector<CTriangleMesh::triangle_type> faces;


	f >> attribute;

	if (attribute != start_solid)
		return PTriangleMesh();

	f >> tag;
	if (tag != start_face) {
		cvdebug() << "Loading solid " <<solid_name << "\n";
		f >> tag;
	}

	while (tag != end_solid) {
		CTriangleMesh::triangle_type face;
		C3DFVector face_normal;

		if (tag != start_face)
			goto fail;

		cverb << "start face ";

		f >> tag;
		if (tag != normal_tag)
			goto fail;

		f >> face_normal.x >> face_normal.y >> face_normal.z;
		//	face_normals.push_back(face_normal);

		cverb << " normal " << face_normal;
		f >> tag;
		if (tag != start_loop)
			goto fail;

		f >> tag;
		if (tag != tag_loop)
			goto fail;

		if (!read_vertex(vmap, face.x, f))
			goto fail;
		if (!read_vertex(vmap, face.y, f))
			goto fail;
		if (!read_vertex(vmap, face.z, f))
			goto fail;

		f >> tag;
		if (tag != end_loop)
			goto fail;

		f >> tag;
		if (tag != end_face)
			goto fail;

		cverb << " finished\n";

		faces.push_back(face);

		f >> tag;

		if (!f.good())
			goto fail;
	}
	{
		cvmsg() << "Got a mesh with " << vmap.size() << " vertices and " <<
			faces.size() << " faces\n";

		CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(vmap.size()));

		// remap the vertices
		for (CVertexMap::const_iterator i = vmap.begin();
		     i != vmap.end(); ++i)
			(*vertices)[i->second] = i->first;

		cvwarn() << "For nor we throw away the normals of the file\n";

		CTriangleMesh::PTrianglefield tri(new CTriangleMesh::CTrianglefield(faces.size()));
		copy(faces.begin(), faces.end(), tri->begin());

		return PTriangleMesh(new CTriangleMesh(tri,vertices));
	}

fail:
	cverr() << "not a valid ascii STL mesh file\n";
	return PTriangleMesh();
}

PTriangleMesh CSTLMeshIO::do_load(string const &  filename)const
{
	PTriangleMesh result;

	cvdebug() << "try stl mesh\n";

	if (filename != "-") {
		ifstream inf(filename.c_str());
		result = load_ascii(inf);
	}else
		result = load_ascii(cin);

	if (!result) { // probably a binary file

		cvdebug() << "Try STL binary file\n";
		CInputFile f(filename);

		if (!f)
			return PTriangleMesh();

		cvdebug() << "If this is a binary STL mesh then it can not yet be loaded\n";
		return PTriangleMesh();
	}
	return PTriangleMesh(result);
}

bool CSTLMeshIO::save_ascii(ostream& of, const CTriangleMesh& data)const
{
	of << start_solid << '\n';

	//	int s = data.triangle_size();
	int i = 0;

	for (CTriangleMesh::const_triangle_iterator t = data.triangles_begin();
	     t != data.triangles_end(); ++t, ++i) {

		const C3DFVector& vx = data.vertex_at(t->x);
		const C3DFVector& vy = data.vertex_at(t->y);
		const C3DFVector& vz = data.vertex_at(t->z);

		C3DFVector e1 = vx - vy;
		C3DFVector e2 = vz - vy;

		C3DFVector normal = e2 ^ e1;
		double n = normal.norm2();
		if (n > 0.0)
			normal /= sqrt(n);

		of << "  " << start_face << " " << normal_tag << " "
		   << normal.x << " " << normal.y << " " << normal.z << "\n";
		of << "    " << start_loop << " " << tag_loop << "\n";
		of << "      "  << vertex_tag << " "
		   << vx.x << " " << vx.y << " " << vx.z<< "\n";
		of << "      "  << vertex_tag << " "
		   << vy.x << " " << vy.y << " " << vy.z<< "\n";
		of << "      "  << vertex_tag << " "
		   << vz.x << " " << vz.y << " " << vz.z<< "\n";
		of << "    "  << end_loop << "\n";
		of << "  "  << end_face << "\n";

	}
	of << end_solid << "\n";
	return of.good();
}

bool CSTLMeshIO::do_save(string const &  filename, const CTriangleMesh& data)const
{
	if (filename != "-") {
		ofstream of(filename.c_str());
		return save_ascii(of, data);
	}else
		return save_ascii(cout, data);

}

const string CSTLMeshIO::do_get_descr()const
{
	return "STL mesh io plugin";
}

void CSTLMeshIO::do_add_suffixes(std::multimap<std::string, std::string>& map) const
{
	map.insert(pair<string,string>(".stl", get_name()));
	map.insert(pair<string,string>(".STL", get_name()));
}

NS_END
