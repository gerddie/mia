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

// documentation of the format:
// http://www.geomview.org/docs/html/OFF.html

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
			   bool has_texture_coordinates, 
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
	return string("plugin to load/store some Geomview OFF files. "
		      "Supported is only the ASCII format 3D meshes with normales and per-vertex RGB colors. "
		      "Alpha values and texture coordinates, as well as per face properties are ignored. "
		      "Polygons are read and triangulated. "
		);
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
			       bool has_texture_coordinates, 
			       unsigned int nvertices)const
{
	unsigned int i = 0;

	CTriangleMesh::vertex_iterator v = vertices->begin();

	CTriangleMesh::normal_iterator n;
	CTriangleMesh::color_iterator  c; 

	int expect_count = has_texture_coordinates ? 5 : 2; 
	int c_offest = 0;
	if (normals) {
		n = normals->begin();
		c_offest += 3;
		expect_count += 3; 
	}
	
	if (colors) {
		c = colors->begin();
		expect_count += 3; 
	}

	char buf[2048];

	bool success = read_line(buf, 2048, inf);
	if (!success)
		return false;

	int first_count = 0; 

	do {
		float x[9];
		float *xc = &x[c_offest];
		
		int count = sscanf(buf, "%f %f %f %f %f %f %f %f %f %f %f %f",
				   &v->x, &v->y, &v->z,
				   &x[0], &x[1], &x[2],
				   &x[3], &x[4], &x[5], &x[6],
				   &x[7], &x[8]);

		if (count < expect_count) {
			throw create_exception<runtime_error>("OFF: unsupported file type. "
							      "Most likely the file type uses color map indices."); 
		}

		if (first_count > 0) {
			if (first_count != count) {
				throw create_exception<runtime_error>("OFF: inconsistent vertex definitions"); 
			}
		}else{
			first_count = count; 
		}
		
		if (normals) {
			n->x = x[0];
			n->y = x[1];
			n->z = x[2];
			++n;
			count -= 6; 
		}
		
		if (colors) {
			if (count < 3)
				throw create_exception<invalid_argument>("OFF: color indices not supported"); 
			c->x = xc[0] < 1.0 ? xc[0] : xc[0]/ 255.0f;
			c->y = xc[1] < 1.0 ? xc[1] : xc[1]/ 255.0f;
			c->z = xc[2] < 1.0 ? xc[2] : xc[2]/ 255.0f;
			++c; 
		}
		
		++v;
		++i; 
	} while (i < nvertices && read_line(buf, 2048, inf));

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
	if (!read_line(buffer, 2048, inp))
		throw create_exception<runtime_error>("OFF: Unable to read from input file.");
	
	if (sscanf(buffer, "%d %d %d", &n_vertices, &n_faces, &n_edges)!= 3) {
		throw create_exception<runtime_error>("OFF: parse error reading from '", buffer, "'");
	}

	cvdebug() << "found file with " <<n_vertices << " vertices, and " <<  n_faces << " faces\n";

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(n_vertices));
	CTriangleMesh::PNormalfield normals;
	CTriangleMesh::PColorfield  colors;

	if (flags & vt_color) {
		colors.reset(new CTriangleMesh::CColorfield(n_vertices));
	}

	if (flags & vt_normal) {
		normals.reset(new CTriangleMesh::CNormalfield(n_vertices));
	}

	if ( !load_vertices(inp, vertices, normals, colors, flags & vt_texture, n_vertices)) {
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
	return PTriangleMesh(new CTriangleMesh(triangles, vertices, normals, colors, nullptr));

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

	if (flags & CTriangleMesh::ed_scale) {
		cvwarn() << "OFF-save: Mesh has scale values, but they are "
			 << "not supported by the OFF file format and will be lost\n"; 
	}

	for (size_t i = 0; i < data.vertices_size() && written > 0; ++i) {
		const CTriangleMesh::vertex_type& v = data.vertex_at(i);
		fprintf(f, "%f %f %f",v.x,v.y, v.z);

		if (flags & CTriangleMesh::ed_normal) {
			const CTriangleMesh::normal_type& n = data.normal_at(i);
			written += fprintf(f, " %f %f %f",n.x,n.y, n.z);
		}
		if (flags & CTriangleMesh::ed_color) {
			const CTriangleMesh::color_type& c = data.color_at(i);
			written += fprintf(f, " %f %f %f",c.x,c.y, c.z);
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

