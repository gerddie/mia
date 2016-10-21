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

#ifndef mia_mesh_triangleneighborhood_hh
#define mia_mesh_triangleneighborhood_hh

#include <mia/mesh/triangularMesh.hh>

NS_MIA_BEGIN

class EXPORT_MESH  CTrianglesWithAdjacentList {
public: 
        CTrianglesWithAdjacentList(const CTriangleMesh& mesh); 

        const std::set<unsigned>& operator [](unsigned idx) const; 

private: 

        std::vector<std::set<unsigned> > m_triangle_neighborhood; 

        
}; 


NS_MIA_END

#endif 
