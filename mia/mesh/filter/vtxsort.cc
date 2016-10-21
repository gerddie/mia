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

#include <mia/mesh/filter/vtxsort.hh>

NS_BEGIN(mia_meshfilter_vtxsort) 
using namespace mia; 
using namespace std; 


CVtxSortMeshFilter::CVtxSortMeshFilter(const C3DFVector& sort_direction ):
	m_sort_direction(sort_direction) 
{
}
	

PTriangleMesh CVtxSortMeshFilter::do_filter(const CTriangleMesh& mesh) const
{
	struct collector {
		float product; 
		C3DFVector v; 
		long index;
	}; 
	
	vector<collector> help; 
	help.reserve(mesh.vertices_size()); 
	
	collector c; 
	c.index = 0; 
	
	for(auto k = mesh.vertices_begin(); k != mesh.vertices_end(); ++k, ++c.index) {
		c.product = dot(*k, m_sort_direction); 
		c.v = *k; 
		help.push_back(c); 
	}

	sort(help.begin(), help.end(), 
	     [](const collector& lhs, const collector& rhs) {
		     return lhs.product < rhs.product; 
	     }); 

	PTriangleMesh result(new CTriangleMesh(mesh.triangle_size(), mesh.vertices_size())); 
	
	vector<unsigned int> reindex(mesh.vertices_size()); 

	// copy the sorted vertices
	auto out_vert = result->vertices_begin(); 
	unsigned int pos = 0; 
	for (auto h = help.begin(); h != help.end(); ++h, ++out_vert, ++pos) {
		*out_vert = h->v; 
		reindex[h->index] = pos; 
	}

	// copy the triangle connectivity
	transform(mesh.triangles_begin(), mesh.triangles_end(), result->triangles_begin(), 
		  [&reindex](const CTriangleMesh::triangle_type& t) {
			  return CTriangleMesh::triangle_type(reindex[t.x], 
							      reindex[t.y], 
							      reindex[t.z]); 
		  }); 
		  
	// if existent move the color scales and 
	if (mesh.get_available_data() & CTriangleMesh::ed_scale) {
		transform( help.begin(), help.end(), result->scale_begin(), 
			   [&mesh](const collector& c){
				   return mesh.scale_at(c.index); }); 
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_color) {
		transform( help.begin(), help.end(), result->color_begin(), 
			   [&mesh](const collector& c){
				   return mesh.color_at(c.index);});
	}
	if (mesh.get_available_data() & CTriangleMesh::ed_normal) {
		transform( help.begin(), help.end(), result->normals_begin(), 
			   [&mesh](const collector& c){
				   return mesh.normal_at(c.index);});
	}
	return result; 
}

CVtxSortMeshFilterPlugin::CVtxSortMeshFilterPlugin():
	CMeshFilterPlugin("vtxsort"), 
	m_sort_direction(0,0,1)
{
	add_parameter("dir", make_param(m_sort_direction, false, 
					      "Sorting direction,"));
}

mia::CMeshFilter *CVtxSortMeshFilterPlugin::do_create()const
{
	return new CVtxSortMeshFilter(m_sort_direction);
}

const std::string CVtxSortMeshFilterPlugin::do_get_descr()const
{
	return "This filter sorts the vertices of a mesh according to the given direction. "
		"Specifically, the dot product between the given direction and the vertices "
		"will be ordered ascending"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CVtxSortMeshFilterPlugin();
}

NS_END
