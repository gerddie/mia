/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/mesh/triangle_neighbourhood.hh>
#include <map>

NS_MIA_BEGIN

using namespace std; 

struct Edge {
        unsigned a; 
        unsigned b;
        Edge(unsigned _a, unsigned _b); 
        void add_triangle(unsigned t); 
}; 


Edge::Edge(unsigned _a, unsigned _b):
        a(_a), b(_b)
{
        if (a > b) 
                swap(a,b); 
}


struct compare_edge {
        bool operator () ( const Edge& lhs, const Edge& rhs) const {
                return (lhs.a < rhs.a) || ((lhs.a == rhs.a) && lhs.b < rhs.b); 
        }
}; 

CTrianglesWithAdjacentList::CTrianglesWithAdjacentList(const CTriangleMesh& mesh):
m_triangle_neighborhood(mesh.triangle_size())
{
        // collect the edges and adjacent triangles 

        map<Edge, set<unsigned>, compare_edge> edges;

        auto add_edge = [&edges](unsigned a, unsigned b, unsigned t) {
                Edge e(a,b); 
                auto ie = edges.find(e); 
                if (ie == edges.end()) {
                        set<unsigned> n({t}); 
                        edges[e] = n;
                        
                } else 
                        ie->second.insert(t);
        }; 

        // collect all edges and 
        for (unsigned it = 0; it < mesh.triangle_size(); ++it) {
                auto t = mesh.triangle_at(it); 
                add_edge(t.x, t.y, it); 
                add_edge(t.y, t.z, it); 
                add_edge(t.z, t.x, it); 
        }

        // now collect the adjesent triangles
        for (auto e : edges) {
                for (auto t1 : e.second) {
                        for (auto t2 : e.second) {
                                if (t1 != t2) {
                                        m_triangle_neighborhood[t1].insert(t2); 
                                        m_triangle_neighborhood[t2].insert(t1);
                                }
                        }
                }
        }
}
        
const std::set<unsigned>& CTrianglesWithAdjacentList::operator [](unsigned idx) const
{
        assert(idx < m_triangle_neighborhood.size()); 
        return m_triangle_neighborhood[idx]; 
}

NS_MIA_END
