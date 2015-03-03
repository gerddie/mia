/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/mesh/filter/deltrianglesbynormal.hh>
#include <mia/mesh/triangle_neighbourhood.hh>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <mia/core/threadedmsg.hh>



#include <cmath>
#include <set>

NS_BEGIN(mia_meshfilter_deltribynormal) 
using namespace mia; 
using namespace std; 
using tbb::blocked_range; 
using tbb::parallel_for; 

struct VertexWithIndex {
	unsigned idx;
	C3DFVector v; 

	VertexWithIndex(const CTriangleMesh& mesh, unsigned i):
		idx(i), 
		v(mesh.vertex_at(i)){
		
	}
}; 

struct compare_vertex  {
	bool operator () (const VertexWithIndex& lhs, const VertexWithIndex& rhs) {
		return (lhs.v.z < rhs.v.z) || 
			((lhs.v.z == rhs.v.z) && ((lhs.v.y < rhs.v.y) ||
						  ((lhs.v.y == rhs.v.y) && (lhs.v.x < rhs.v.x)))); 
	}
}; 



CDeleteTriangleByNormalMeshFilter::CDeleteTriangleByNormalMeshFilter(const C3DFVector& point_direction, float angle):
	m_point_direction(point_direction), 
	m_test_angle_cos(cosf(angle / 180.0f * M_PI))
{
	auto pn = m_point_direction.norm(); 
	if (pn < 1e-5)
		throw invalid_argument("CDeleteTriangleByNormalMeshFilter:you provided a very small normal direction vector."); 
	
	m_point_direction /= pn; 

	cvdebug() << "CDeleteTriangleByNormalMeshFilter:dir=" << m_point_direction << ", cos(angle)= "<< m_test_angle_cos << "\n";
}
	

PTriangleMesh CDeleteTriangleByNormalMeshFilter::do_filter(const CTriangleMesh& mesh) const
{
	// evaluate the triangle normals, 
	vector<bool>  keep(mesh.triangle_size(), false); 

	auto run_triangles = [this, &mesh, &keep](const blocked_range<unsigned>& range) {
		CThreadMsgStream thread_stream;		
		for (auto i = range.begin(); i != range.end(); ++i) {
			auto& t = mesh.triangle_at(i); 
			auto& vx = mesh.vertex_at(t.x); 
			auto& vy = mesh.vertex_at(t.y); 
			auto& vz = mesh.vertex_at(t.z); 

			auto e1 = vx - vy;
			auto e2 = vz - vy;
			auto normal = cross(e2, e1);
			auto nn = normal.norm(); 

			// very small triangles will be discarded
			// this should become a separate filter 
			if (nn < 1e-5) 
				continue; 

			auto test_dir = dot(normal, m_point_direction) / nn; 

			cvdebug() << "Triangle:" << t << ": test-angle-cos = " << test_dir << "\n"; 
			if (test_dir >= m_test_angle_cos) 
				keep[i] = true; 
		}; 
	}; 
	
	// estimate which triangles to keep 
	parallel_for(blocked_range<unsigned>(0, mesh.triangle_size()), run_triangles); 

	// re-add all triangles that have all neighbours within the keep 
	CTrianglesWithAdjacentList trineigh(mesh); 
	for (size_t i = 0; i < mesh.triangle_size(); ++i) {
		if (!keep[i]) {
			auto& ajd =  trineigh[i]; 
			bool k = true; 
			for ( auto a: ajd) 
				k &= keep[a]; 
			keep[i] = k; 
		}
	}

	// remove isolated triangles 
	for (size_t i = 0; i < mesh.triangle_size(); ++i) {
		if (keep[i]) {
			auto& ajd =  trineigh[i]; 
			bool k = false; 
			for ( auto a: ajd) 
				k |= keep[a];
			keep[i] = k;
		}
	}
	
	auto new_triangles = make_shared<CTriangleMesh::CTrianglefield>(); 
	for (size_t i = 0; i < mesh.triangle_size(); ++i) {
		if (keep[i]) 
			new_triangles->push_back(mesh.triangle_at(i)); 
	}
	
	// collect the vertices 
	set<VertexWithIndex, compare_vertex> vtxset;
	for (auto t = new_triangles->begin(); new_triangles->end() != t; ++t) {
		vtxset.insert(VertexWithIndex(mesh, t->x)); 
		vtxset.insert(VertexWithIndex(mesh, t->y)); 
		vtxset.insert(VertexWithIndex(mesh, t->z)); 
	}

	vector<unsigned> reindex(mesh.vertices_size(), mesh.vertices_size()); 
	unsigned out_index = 0; 

	auto new_vertices = make_shared<CTriangleMesh::CVertexfield>(); 
	new_vertices->reserve(vtxset.size()); 
	
	for (auto iv = vtxset.begin(); iv != vtxset.end(); ++iv) {
		new_vertices->push_back(iv->v); 
		cvdebug() << "vertex[" << out_index << "]=" <<iv->v << "\n"; 
		reindex[iv->idx] = out_index++; 
	}
	
	// re-index the triangles 
	for_each(new_triangles->begin(), new_triangles->end(), [&reindex](CTriangleMesh::triangle_type& t){
			t.x = reindex[t.x]; 
			t.y = reindex[t.y]; 
			t.z = reindex[t.z]; 
			cvdebug() << "triangle:" << t << "\n"; 
		}); 
	
	return make_shared<CTriangleMesh>(new_triangles, new_vertices);
}

CDeleteTriangleByNormalMeshFilterPlugin::CDeleteTriangleByNormalMeshFilterPlugin():
	CMeshFilterPlugin("deltribynormal"), 
	m_point_direction(0,1,0), 
	m_tolerance_angle(5)
{
	add_parameter("dir", make_param(m_point_direction, true, 
					"Direction the triangle normals are expected to point to. The "
					"orientation of the vector is taken into account."));
	add_parameter("angle", make_param(m_tolerance_angle, false, 
					  "Tolerance angle (in degree) that the normal can diviate from "
					  "the provided direction."));
}

mia::CMeshFilter *CDeleteTriangleByNormalMeshFilterPlugin::do_create()const
{
	return new CDeleteTriangleByNormalMeshFilter(m_point_direction, m_tolerance_angle);
}

const std::string CDeleteTriangleByNormalMeshFilterPlugin::do_get_descr()const
{
	return "This filter deletes all triangles that have a normal that differs by the  " 
		"given angle from the provided direction."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CDeleteTriangleByNormalMeshFilterPlugin();
}

NS_END
