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


EXPORT  CTriangleMesh *gts_to_mona_mesh(GtsSurface *surface, bool reverse)
{
	
	int n_vertices = gts_surface_vertex_number(surface);
	int n_faces = gts_surface_face_number(surface);
	
	CTriangleMesh::CVertexfield vinitial(n_vertices);

	
	vertex_data_t vdata = {vinitial.begin(), 0};
	
	gts_surface_foreach_vertex(surface, (GtsFunc)copy_vertices,&vdata);

	CTriangleMesh::CTrianglefield initriangles(n_faces); 
	auto ti = initriangles.begin();
	
	gts_surface_foreach_face(surface, (GtsFunc)copy_triangles,&ti);
	
	auto vertices  = CTriangleMesh::PVertexfield(new  CTriangleMesh::CVertexfield);
	vertices->reserve(n_vertices); 

	vector<int> remap(n_vertices); 
	auto r = remap.begin(); 
	for (auto v = vinitial.begin(); v != vinitial.end(); ++v, ++r) {
		auto ov = find(vertices->begin(), vertices->end(), *v); 
		if (ov != vertices->end()) {
			cvdebug() << "found duplicate vertex\n"; 
			*r = distance(vertices->begin(), ov);
		}else {
			*r = vertices->size(); 
			vertices->push_back(*v); 
		}
	}
	
	
	auto triangles = CTriangleMesh::PTrianglefield(new  CTriangleMesh::CTrianglefield);
	triangles->reserve(n_faces); 

	for_each(initriangles.begin(), initriangles.end(), 
		 [&remap, &triangles](CTriangleMesh::triangle_type& t) -> void {
			 t.x = remap[t.x]; 
			 t.y = remap[t.y]; 
			 t.z = remap[t.z]; 
			 if (t.x != t.y && t.x != t.z && t.y != t.z) 
				 triangles->push_back(t); 
		 }); 
	
	if (reverse) {
		cvmsg() << "revert triangles\n"; 
		transform(triangles->begin(), triangles->end(), triangles->begin(), 
			 [](const CTriangleMesh::triangle_type& t) {
				  return CTriangleMesh::triangle_type(t.x, t.z, t.y); 
				  
			 }); 
	}
	

	
	return new CTriangleMesh(triangles, vertices, NULL, NULL, NULL);
}

