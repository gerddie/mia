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

#ifndef addons_vtk_vtkmesh_hh
#define addons_vtk_vtkmesh_hh

#include <mia/mesh/triangularMesh.hh>

namespace vtkmia {

class CVtkMeshIO: public mia::CMeshIOPlugin {
public:
	typedef mia::CTriangleMesh::CTrianglefield CTrianglefield; 
	typedef mia::CTriangleMesh::CVertexfield   CVertexfield; 
	typedef mia::CTriangleMesh::CNormalfield   CNormalfield; 
	typedef mia::CTriangleMesh::CColorfield    CColorfield; 
	typedef mia::CTriangleMesh::CScalefield    CScalefield;  

	typedef mia::CTriangleMesh::PTrianglefield PTrianglefield; 
	typedef mia::CTriangleMesh::PVertexfield   PVertexfield; 
	typedef mia::CTriangleMesh::PNormalfield   PNormalfield; 
	typedef mia::CTriangleMesh::PColorfield    PColorfield; 
	typedef mia::CTriangleMesh::PScalefield    PScalefield;  

	static const char * const  s_scale_array;
	static const char * const  s_normal_array;
	static const char * const  s_color_array;

	CVtkMeshIO();
private:
        virtual mia::PTriangleMesh do_load(std::string const &  filename) const;
        virtual bool do_save(std::string const &  filename, const mia::CTriangleMesh& data) const;
        const std::string  do_get_descr() const;


};

}

#endif
