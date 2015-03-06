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

#include <queue>
#include <mia/mesh/filter/selectbig.hh>
#include <mia/mesh/triangle_neighbourhood.hh>

NS_BEGIN(mia_meshfilter_selectbig) 
using namespace mia; 
using namespace std; 


CSelectBigMeshFilter::CSelectBigMeshFilter()
{
}
	

PTriangleMesh CSelectBigMeshFilter::do_filter(const CTriangleMesh& mesh) const
{
        CTrianglesWithAdjacentList tri_neigh(mesh); 
        
        vector<bool> taken(mesh.triangle_size(), false); 
        vector<bool> added(mesh.triangle_size(), false); 
        
        size_t remaining = mesh.triangle_size(); 
        
        vector<unsigned> largest_section; 
                
        while (remaining) {
                vector<unsigned> section; 
                
                unsigned int idx = 0; 
                while (taken[idx]) 
                        ++idx; 
        
                cvdebug() << "Start section at " << idx << "\n"; 
                queue<unsigned> next_triangle; 
                next_triangle.push(idx); 
		added[idx] = true; 
                while (!next_triangle.empty()) {
                        unsigned t = next_triangle.front(); 
                        next_triangle.pop(); 
                        if (!taken[t]) {
				cvdebug() << "Add " << t << "\n"; 
                                section.push_back(t); 
                                taken[t] = true; 
                                --remaining; 
                        }
                        
                        auto& ajd =  tri_neigh[t]; 
                        for ( auto a: ajd) {
                                if (!taken[a] && !added[a]) {
                                        next_triangle.push(a); 
					added[a] = true; 
				}
                        }
                }
                if (section.size() > largest_section.size()) {
                        swap(largest_section, section); 
                        cvdebug() << "got section of size " << largest_section.size() << "\n"; 
                }
                if (largest_section.size() > remaining) {
                        cvdebug() << "Only remain " << remaining << " triangles, stop processing\n"; 
                        break; 
                }
        }
        
        return get_sub_mesh(mesh, largest_section);
}

CSelectBigMeshFilterPlugin::CSelectBigMeshFilterPlugin():
	CMeshFilterPlugin("selectbig")
{
}

mia::CMeshFilter *CSelectBigMeshFilterPlugin::do_create()const
{
	return new CSelectBigMeshFilter();
}

const std::string CSelectBigMeshFilterPlugin::do_get_descr()const
{
	return "This filter selects the component with the highest number of triangles from "
                "a mesh that is actually composed of disconnected components. Components are "
                "considered to be disconnected if they don't share a common edge (they might "
                "share a common vertex though"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CSelectBigMeshFilterPlugin();
}

NS_END
