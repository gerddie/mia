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
#include <vector>
#include <cstring>


#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/mesh/triangulate.hh>


NS_BEGIN(off_mesh_io)

NS_MIA_USE;
using namespace std;

static char const * const format = "off";

#define USE_FILEIO



class COffMeshIO: public CMeshIOPlugin {
public:
	COffMeshIO();
private:
	typedef TPolyTriangulator<CTriangleMesh::CVertexfield, vector<unsigned int> >  CPolyTriangulator;
	virtual PTriangleMesh do_load(string const &  filename)const;
	virtual bool do_save(string const &  filename, const CTriangleMesh& data)const;
	virtual const string  do_get_descr()const;

	PTriangleMesh do_load_it(CInputFile& inp)const;

	bool load_vertices(CInputFile &inp,
			   CTriangleMesh::PVertexfield vertices,
			   CTriangleMesh::PNormalfield normals,
			   CTriangleMesh::PColorfield colors,
			   CTriangleMesh::PScalefield scale,
			   bool has_texturcoord,
			   unsigned int nvertices)const;

	bool load_triangles(CInputFile& inp, vector<CTriangleMesh::triangle_type>& tri,
			    unsigned int nfaces, unsigned int nvertice,
			    const CPolyTriangulator& triangulator)const;
	bool read_polygon(istream& inp, vector<CTriangleMesh::triangle_type>& tri,
			  unsigned int nvertices,const CPolyTriangulator& triangulator)const;


	void skip_to_newline(istream& inp)const;
};


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new COffMeshIO();
}

COffMeshIO::COffMeshIO():CMeshIOPlugin(format)
{
	add_suffix(".off");
	add_suffix(".OFF");
}


const string  COffMeshIO::do_get_descr()const
{
	return string("plugin to load/store some Geomview OFF Files");
}

PTriangleMesh COffMeshIO::do_load(string const &  filename)const
{
	CInputFile f(filename);
	if (!f)
		return PTriangleMesh();
	return do_load_it(f);
}

void COffMeshIO::skip_to_newline(istream& inp)const
{
	char c;
	do  {
		inp >> c;
	}while (c != '\n' && inp.good());
}

static bool read_line(char *buf, size_t size, FILE *f)
{
	char *success;
	do {
		success = fgets( buf, size, f);
		while (success && (*success == ' ' || *success == '\t'))  ++success;
	} while (success && (*success == '#' || *success == '\n' || *success == '\r'));
	return success != NULL;
}

bool COffMeshIO::load_vertices(CInputFile& inf,
				  CTriangleMesh::PVertexfield vertices,
				  CTriangleMesh::PNormalfield normals,
				  CTriangleMesh::PColorfield colors,
				  CTriangleMesh::PScalefield scale,
				  bool has_texturcoord,
				  unsigned int nvertices)const
{
	unsigned int i = 0;

	CTriangleMesh::vertex_iterator v = vertices->begin();

	char buf[2048];

	bool success = read_line(buf, 2048, inf);
	if (!success)
		return false;


	if (normals && colors &&  scale) {
		CTriangleMesh::normal_iterator n = normals->begin();
		CTriangleMesh::color_iterator  c = colors->begin();
		CTriangleMesh::scale_iterator  s = scale->begin();

		if (has_texturcoord) {
			do {
				float sink; // texture coordinates are currently rejected
				sscanf(buf, "%f %f %f %f %f %f %f %f %f %f %f %f",
				       &v->x, &v->y, &v->z,
				       &n->x, &n->y, &n->z,
				       &c->x, &c->y, &c->z,
				       &*s,
				       &sink,
				       &sink);
				++v; ++n; ++s; ++c; ++i;
			}while (i < nvertices && read_line(buf, 2048, inf));

		}else
			do  {
				sscanf(buf, "%f %f %f %f %f %f %f %f %f %f",
				       &v->x, &v->y, &v->z,
				       &n->x, &n->y, &n->z,
				       &c->x, &c->y, &c->z,
				       &*s);
				++v; ++n; ++s; ++c; ++i;
			}  while (i < nvertices && read_line(buf, 2048, inf));
	}else if (normals) {
		CTriangleMesh::normal_iterator n = normals->begin();
		do  {
			int read = sscanf(buf, "%f %f %f %f %f %f",
					   &v->x, &v->y, &v->z,
					   &n->x, &n->y, &n->z);
			if (read != 6)
				throw create_exception<runtime_error>("Bogus OFF file: Expect 6 values (vertices and normals) but got only ",
								      read, " numbers"); 
			
			    ++v; ++n; ++i;
		}while (i < nvertices && read_line(buf, 2048, inf));
	}else
		do  {
			istringstream line(buf); line >> v->x >> v->y >>v->z;

			++v; ++i;

		}while (i < nvertices && read_line(buf, 2048, inf));
	return i == nvertices;
}

bool COffMeshIO::read_polygon(istream& inp, vector<CTriangleMesh::triangle_type>& tri,
			      unsigned int nvertices,
			      const COffMeshIO::CPolyTriangulator& triangulator)const
{
	int nvert;

	inp >> ws >>nvert;
	if (nvert == 3) {
		CTriangleMesh::triangle_type triangle;
		inp >> ws >> triangle.x >> ws >>  triangle.y >> ws >> triangle.z;
		tri.push_back(triangle);

		if (triangle.x >=  nvertices ||  triangle.y >=  nvertices ||  triangle.z >=  nvertices) {
			cverr() << "COffMeshIO::load_triangles: index out of range\n";
			return false;
		}

	}else {
		vector<unsigned int> poly(nvert);
		
		for (unsigned  k = 0; k < poly.size(); ++k) {
			inp >> ws >> poly[k];
			if (poly[k] >= nvertices) {
				cverr() << "COffMeshIO::load_triangles: index out of range\n";
				return false;
			}
		}

		triangulator.triangulate(tri, poly);
	}
	return true;
}

bool COffMeshIO::load_triangles(CInputFile& inp, vector<CTriangleMesh::triangle_type>& tri,
				unsigned int nfaces, unsigned int nvertices,
				const CPolyTriangulator& triangulator)const
{
	int show_thresh_start = nfaces / 50;
	int show_thresh = show_thresh_start;


	char line[2048];

	bool success = read_line(line, 2048, inp);

	while (nfaces-- && success){
		istringstream in_line(line);

		if (!read_polygon(in_line, tri, nvertices, triangulator))
			return false;

		show_thresh--;
		if (!show_thresh) {
			show_thresh = show_thresh_start;
		}
		success = read_line(line, 2048, inp);

	}
	return true;
}


class CTriangleChecker {
public:

	CTriangleChecker();

	void operator ()(const CTriangleMesh::triangle_type& tri);

	void print();
private:


	void evaluate();

	map<int, map< int, int > > m_count;

	int m_edges;
	int m_non_manifold;
};


CTriangleChecker::CTriangleChecker():
	m_edges(0),
	m_non_manifold(0)
{

}

void CTriangleChecker::operator ()(const CTriangleMesh::triangle_type& tri)
{
	if (tri.x > tri.y) m_count[tri.x][tri.y] += 1; else  m_count[tri.y][tri.x] += 1;
	if (tri.x > tri.z) m_count[tri.x][tri.z] += 1; else  m_count[tri.z][tri.x] += 1;
	if (tri.z > tri.y) m_count[tri.z][tri.y] += 1; else  m_count[tri.y][tri.z] += 1;
}

void CTriangleChecker::print()
{
	if (cverb.show_debug()) {
		evaluate();
		cvdebug() << "Mesh has \n";
		cvdebug() << "  nonmanifold edges: " << m_non_manifold << "\n";
		cvdebug() << "           of edges: " << m_edges << "\n";
	}
}

void CTriangleChecker::evaluate()
{
	map< int, map< int, int > >::const_iterator io = m_count.begin();
	map< int, map< int, int > >::const_iterator eo = m_count.end();

	while (io != eo) {

		m_edges += io->second.size();
		map< int, int >::const_iterator i1 = io->second.begin();
		map< int, int >::const_iterator e1 = io->second.end();

		while (i1 != e1) {
			if (i1->second != 2) {
				++m_non_manifold;

				if (m_non_manifold < 10) {
					cvdebug() << io->first << ":"
						  << i1->first << " "
						  << i1->second << "\n";
				}
			}
			++i1;
		}
		++io;
	}
}


enum EVertexFlags {vt_texture = 1,
		   vt_color   = 2,
		   vt_normal  = 4,
		   vt_size4   = 8,
		   vt_sizeN   = 16,
		   vt_is_off  = 32
};

int get_vertex_flags(const char *header)
{
	int flags = 0;
	if (!strncmp(header, "ST", 2)) {
		flags |= vt_texture;
		header += 2;
	}

	if (*header == 'C') {
		flags |= vt_color;
		++header;
	}

	if (*header == 'N') {
		flags |= vt_normal;
		++header;
	}

	if (*header == '4') {
		flags |= vt_size4;
		++header;
	}else if (*header == 'n') {
		flags |= vt_sizeN;
		++header;
	}

	if (!strncmp(header, "OFF", 3))
		return flags | vt_is_off;

	return 0;
}

PTriangleMesh COffMeshIO::do_load_it(CInputFile& inp)const
{
	int c;

	const int c_max_header = 10;
	int max_header = c_max_header;
	char header[c_max_header];
	char *p_header =  header;


	// read header
	while ( (c = fgetc(inp)) != EOF && strchr("STCNn4OF", c) && --max_header)
		*p_header++ = c;

	int flags = get_vertex_flags(header);

	if (!(flags & vt_is_off)) {
		cvdebug() << "no OFF mesh\n";
		return PTriangleMesh();
	}

	int vertex_size = 3;

	if (flags & vt_sizeN) {
		if (fscanf(inp, "%d\n", & vertex_size) != 1)
			throw runtime_error("OFF: Unable to read from file.");
	}else if (flags & vt_size4) {
		vertex_size = 4;
	}

	if (vertex_size != 3) {
		throw create_exception<invalid_argument>("OFF: currently only 3D vertexes are supported, ",
							 "file contains ", vertex_size, "D vertices");
	}

	int n_vertices = 0;
	int n_faces = 0;
	int n_edges = 0;

	char buffer[2049];
	read_line(buffer, 2048, inp); 
	
	
	if (sscanf(buffer, "%d %d %d", &n_vertices, &n_faces, &n_edges)!= 3) {
		throw create_exception<runtime_error>("OFF: parse error reading from '", buffer, "'");
	}

	cvdebug() << "found file with " <<n_vertices << " vertices, and " <<  n_faces << " faces\n";

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(n_vertices));
	CTriangleMesh::PNormalfield normals;
	CTriangleMesh::PColorfield  colors;
	CTriangleMesh::PScalefield  scale;



	if (flags & vt_color) {
		colors.reset(new CTriangleMesh::CColorfield(n_vertices));
		scale.reset(new CTriangleMesh::CScalefield(n_vertices));
	}

	if (flags & vt_normal) {
		normals.reset(new CTriangleMesh::CNormalfield(n_vertices));
	}

	if ( !load_vertices(inp, vertices, normals, colors, scale, flags & vt_texture, n_vertices)) {
		throw create_exception<runtime_error>("OFF: Error reading vertices");
	}

	vector<CTriangleMesh::triangle_type> tri;

	CPolyTriangulator triangulator(*vertices);

	if (!load_triangles(inp, tri, n_faces, n_vertices, triangulator)) {
		throw create_exception<runtime_error>("OFF: Error reading triangles");
	}

	CTriangleChecker tricheck;
	for(vector<CTriangleMesh::triangle_type>::const_iterator i = tri.begin();
	    i != tri.end(); ++i)
		tricheck(*i);


	cvdebug() << "tricheck.print();\n";
	tricheck.print();

	CTriangleMesh::PTrianglefield triangles(new CTriangleMesh::CTrianglefield(tri.size()));
	copy(tri.begin(), tri.end(), triangles->begin());
	return PTriangleMesh(new CTriangleMesh(triangles, vertices, normals, colors, scale));

}

bool COffMeshIO::do_save(string const &  filename, const CTriangleMesh& data)const
{
	COutputFile f(filename);
	if (!f)
		return false;


	int flags = data.get_available_data();

	stringstream h_stream;
	// write header
	if ( (flags & CTriangleMesh::ed_scale)  || (flags & CTriangleMesh::ed_color))
		h_stream << 'C';
	if (  flags & CTriangleMesh::ed_normal )
		h_stream << 'N';
	h_stream << "OFF\n";

	fprintf(f, "%s\n", h_stream.str().c_str());
	fprintf(f, "%d %d 0\n", data.vertices_size(), data.triangle_size());

	int written = 1;

	for (size_t i = 0; i < data.vertices_size() && written > 0; ++i) {
		const CTriangleMesh::vertex_type& v = data.vertex_at(i);
		fprintf(f, "%f %f %f",v.x,v.y, v.z);

		if (flags & CTriangleMesh::ed_normal) {
			const CTriangleMesh::normal_type& n = data.normal_at(i);
			written += fprintf(f, " %f %f %f",n.x,n.y, n.z);
		}
		if (flags & CTriangleMesh::ed_color && flags & CTriangleMesh::ed_scale) {
			const CTriangleMesh::color_type& c = data.color_at(i);
			written += fprintf(f, " %f %f %f %f",c.x,c.y, c.z, data.scale_at(i));
		}else if (flags & CTriangleMesh::ed_color && !(flags & CTriangleMesh::ed_scale)) {
			const CTriangleMesh::color_type& c = data.color_at(i);
			written += fprintf(f, " %f %f %f 1.0",c.x,c.y, c.z);
		}else if ( !(flags & CTriangleMesh::ed_color) && flags & CTriangleMesh::ed_scale) {
			written += fprintf(f, "1.0 1.0 1.0");
		}
		written += fprintf(f,"\n");
	}

	CTriangleMesh::const_triangle_iterator tb = data.triangles_begin();
	CTriangleMesh::const_triangle_iterator te = data.triangles_end();

	while (tb != te && written > 0) {
		written = fprintf(f,"3 %d %d %d\n", tb->x,tb->y,tb->z);
		++tb;
	}
	return written > 0;
}

}

