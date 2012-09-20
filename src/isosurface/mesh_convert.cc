/* -*- mia-c++  -*-
**
** Copyright (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollnyAtcbs.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser Public License for more details.
** 
** You should have received a copy of the GNU Lesser Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
/*   $Id: mesh_convert.cc 941 2006-07-11 11:59:35Z write1 $ */


// very wired thing, this should work automatically ...
#ifdef G_HAVE_INLINE
#define G_CAN_INLINE 1
#endif

#include <gts.h>

// MONA specific
#include <mia/mesh/triangularMesh.hh>

using namespace std; 
using namespace mia; 
	
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
	auto& vec = *v->vi;
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


CTriangleMesh *gts_to_mona_mesh(GtsSurface *surface)
{
	
	int n_vertices = gts_surface_vertex_number(surface);
	int n_faces = gts_surface_face_number(surface);
	
	auto vertices  = CTriangleMesh::PVertexfield(new  CTriangleMesh::CVertexfield(n_vertices));
	auto triangles = CTriangleMesh::PTrianglefield(new  CTriangleMesh::CTrianglefield(n_faces));
	
	vertex_data_t vdata = {vertices->begin(), 0};
	
	gts_surface_foreach_vertex(surface, (GtsFunc)copy_vertices,&vdata);
	auto ti = triangles->begin();
	
	gts_surface_foreach_face(surface, (GtsFunc)copy_triangles,&ti);
	
	return new CTriangleMesh(triangles, vertices, NULL, NULL, NULL);
}

