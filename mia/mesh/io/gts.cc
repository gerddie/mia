/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <string>

#include <glib.h>

// very wired thing, this should work automatically ...
#ifdef G_HAVE_INLINE
#define G_CAN_INLINE 1
#endif

#include <gts.h>

#include <libmia/plugin_handler.hh>
#include <libmia/triangularMesh.hh>
#include <libmia/monaFileIO.hh>

namespace gts_mesh_io {
using namespace mia;
using namespace std; 

static char const * format = "gts";

class TGtsMeshIO: public CTriangleMeshIO {
public:
	TGtsMeshIO();
	virtual CTriangleMesh* load(string const&  filename, bool, CProgressCallback *cb)const;
	virtual bool save(const CTriangleMesh& data, string const&  filename,bool, CProgressCallback *cb)const;
	virtual const string short_descr()const; 
};


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new TGtsMeshIO;
}


TGtsMeshIO::TGtsMeshIO(): CTriangleMeshIO(format)
{
}

const string  TGtsMeshIO::short_descr()const
{
	return string("GNU Triangulates Surface input/output support");
}

typedef struct  {
	CTriangleMesh::vertex_iterator vi; 
	guint32  c_index;
}vertex_data_t;


typedef struct {
	CTriangleMesh::triangle_iterator ti; 
	guint32 c_index;
}triangle_data_t;

static void copy_vertices(GtsPoint *p, vertex_data_t *v) 
{
	CTriangleMesh::vertexfield_type::value_type& vec = *v->vi;
	vec.x = p->x;
	vec.y = p->y; 
	vec.z = p->z; 
	GTS_OBJECT(p)->reserved = GUINT_TO_POINTER(v->c_index++);
	++v->vi;
}

static void copy_triangles(GtsTriangle *tri, CTriangleMesh::triangle_iterator *ti) 
{
	GtsVertex * v1;
	GtsVertex * v2;
	GtsVertex * v3;
	
	gts_triangle_vertices(tri,&v1,&v2,&v3);
	
	
	(**ti).x = GPOINTER_TO_UINT( GTS_OBJECT(v1)->reserved);
	(**ti).y = GPOINTER_TO_UINT( GTS_OBJECT(v2)->reserved);
	(**ti).z = GPOINTER_TO_UINT( GTS_OBJECT(v3)->reserved);
	++(*ti);
}

CTriangleMesh *TGtsMeshIO::load(string const&  filename, bool from_stdin, CProgressCallback *cb) const
{
	CTriangleMesh * result = NULL; 
	
	
	CInputFile f(filename,from_stdin);
	if (!f)
		return NULL; 
	
	GtsSurface * surface = gts_surface_new (gts_surface_class (),gts_face_class (),
						gts_edge_class (),gts_vertex_class ());
	
	GtsFile *gtsf = gts_file_new(f);
	
	if (gts_surface_read (surface, gtsf) == 0) {
		int n_vertices = gts_surface_vertex_number(surface);
		int n_faces = gts_surface_face_number(surface);
		
		CTriangleMesh::vertexfield_type *vertices  = new  CTriangleMesh::vertexfield_type(n_vertices);
		CTriangleMesh::trianglefield_type *triangles = new  CTriangleMesh::trianglefield_type(n_faces);
		
		vertex_data_t vdata = {vertices->begin(), 0};
		
		gts_surface_foreach_vertex(surface, (GtsFunc)copy_vertices,&vdata);
		CTriangleMesh::triangle_iterator ti = triangles->begin();
		
		gts_surface_foreach_face(surface, (GtsFunc)copy_triangles,&ti);
		
		result = new CTriangleMesh(triangles, vertices, NULL, NULL, NULL);
	}else {
		cvdebug() << "no gts mesh\n"; 
	}
	
	if (GTS_IS_SURFACE(surface))
		gts_object_destroy((GtsObject *)surface);
	
	gts_file_destroy(gtsf);
	
	return result; 
}

class FCopyVertex {
	vector<GtsVertex*>::iterator target;
public:
	FCopyVertex(vector<GtsVertex*>::iterator i):target(i){}
	void operator()(const CTriangleMesh::vertexfield_type::value_type& vertex){
		*target = gts_vertex_new(gts_vertex_class(),vertex.x, vertex.y, vertex.z);
		++target; 
	}
};

class FBuildSurface {
	vector<GtsVertex*>& v; 
	GtsSurface *s; 
public:
	FBuildSurface(vector<GtsVertex*>& _v, GtsSurface *_s): 
		v(_v),s(_s){}
	
	GtsEdge *get_edge(GtsVertex *a, GtsVertex *b) {
		GtsSegment *seg = gts_vertices_are_connected (a, b);
		if (GTS_IS_EDGE (seg)) 
			return GTS_EDGE(seg);
		else 
			return gts_edge_new (s->edge_class, a, b);
	}
	
	void operator()(const CTriangleMesh::trianglefield_type::value_type& t){
		
		GtsVertex *v1 = v[t.x];
		GtsVertex *v2 = v[t.y];
		GtsVertex *v3 = v[t.z];
		
		GtsEdge *e1 = get_edge(v1, v2);
		GtsEdge *e2 = get_edge(v2, v3);
		GtsEdge *e3 = get_edge(v3, v1);
		
		gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));
	}
};

bool TGtsMeshIO::save(const CTriangleMesh& mesh, string const&  filename, bool to_stdout, CProgressCallback *cb) const
{
	COutputFile f(filename,to_stdout);
	if (!f)
		return false; 
	
	GtsSurface * surface = gts_surface_new (gts_surface_class (),gts_face_class (),
						gts_edge_class (),gts_vertex_class ());
	
	vector<GtsVertex *> GtsV(mesh.vertices_size());
	for_each(mesh.vertices_begin(), mesh.vertices_end(), FCopyVertex(GtsV.begin()));
	
	for_each(mesh.triangles_begin(), mesh.triangles_end(), FBuildSurface(GtsV, surface));
	
	GtsFile *gtsf = gts_file_new(f);
	
	gts_surface_write(surface, f);
	
	gts_file_destroy(gtsf);
	
	gts_object_destroy(GTS_OBJECT(surface));
	
	return true; 
}

}
