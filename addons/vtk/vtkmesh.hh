/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifndef addons_vtk_vtkmesh_hh
#define addons_vtk_vtkmesh_hh

#include <mia/mesh/triangularMesh.hh>

namespace vtkmia {

class CVtkMeshIO: public mia::CMeshIOPlugin {
public:
	CVtkMeshIO();
private:
        virtual PTriangleMesh do_load(std::string const &  filename) const;
        virtual bool do_save(std::string const &  filename, const mia::CTriangleMesh& data) const;
        const std::string  do_get_descr() const;

        const char * const s_scale_array = "scale"; 
        const char * const s_normal_array = "normals"; 
        const char * const s_color_array = "colors"; 

	typedef CTriangleMesh::CTrianglefield CTrianglefield; 
	typedef CTriangleMesh::CVertexfield   CVertexfield; 
	typedef CTriangleMesh::CNormalfield   CNormalfield; 
	typedef CTriangleMesh::CColorfield    CColorfield
	typedef CTriangleMesh::CScalefield    CScalefield;  

	typedef CTriangleMesh::PTrianglefield PTrianglefield; 
	typedef CTriangleMesh::PVertexfield   PVertexfield; 
	typedef CTriangleMesh::PNormalfield   PNormalfield; 
	typedef CTriangleMesh::PColorfield    PColorfield
	typedef CTriangleMesh::PScalefield    PScalefield;  
       
};

}

#endif
