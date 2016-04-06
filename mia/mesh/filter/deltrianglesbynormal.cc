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

#include <mia/mesh/filter/deltrianglesbynormal.hh>
#include <mia/mesh/triangle_neighbourhood.hh>

#include <mia/core/parallel.hh>
#include <mia/core/threadedmsg.hh>



#include <cmath>
#include <set>

NS_BEGIN(mia_meshfilter_deltribynormal) 
using namespace mia; 
using namespace std; 

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

	auto run_triangles = [this, &mesh, &keep](const C1DParallelRange& range) {
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
	pfor(C1DParallelRange(0, mesh.triangle_size()), run_triangles); 

	// iteratively re-add all triangles that have at least two neighbours within the keep 
	CTrianglesWithAdjacentList trineigh(mesh); 

	int changed;
	do {
		changed = 0; 
		for (size_t i = 0; i < mesh.triangle_size(); ++i) {
			if (!keep[i]) {
				auto& ajd =  trineigh[i]; 
				int k = 0; 
				for ( auto a: ajd) 
					if (keep[a]) 
						++k; 
				
				if (k >= 2) {
					++changed; 
					keep[i] = true; 
				}
			}
		} 
		cvdebug() << "readded " << changed << " triangles, because of neighbourhood\n"; 
	} while (changed > 0); 

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
	
	vector<unsigned> triangle_indices; 
	for (size_t i = 0; i < mesh.triangle_size(); ++i) {
		if (keep[i]) 
			triangle_indices.push_back(i);
	}
	
	return get_sub_mesh(mesh, triangle_indices); 
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
