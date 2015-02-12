/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/3d/imagedraw.hh>
#include <mia/3d/imagedraw.cxx>

#include <cstdlib>
#include <cmath>

NS_MIA_BEGIN

using std::max; 
using std::min; 
using std::swap; 
using std::fabs; 

C3DDrawBox::C3DDrawBox(const C3DBounds& size, const C3DFVector& origin, const C3DFVector& spacing):
        m_size(size), 
        m_fsize(size - C3DBounds::_1), 
        m_origin(origin)
{
        assert(spacing > C3DFVector::_0); 
        m_stepping = C3DFVector::_1 / spacing;         
}

void C3DDrawBox::draw_point(const C3DFVector& p)
{
        C3DBounds ip(static_cast<unsigned>(floor(p.x + 0.5)), 
                     static_cast<unsigned>(floor(p.y + 0.5)), 
                     static_cast<unsigned>(floor(p.z + 0.5)));
        
        if (ip < m_size) 
                do_draw_point(ip);
}


bool C3DDrawBox::is_inside(const C3DFVector& p) const 
{
        return p >= C3DFVector::_0 && p < m_fsize; 
}

// 
// This code follows zacharmarz answer in 
//
// http://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
// our AABB is [(0,0,0), box_size )
// 
bool C3DDrawBox::make_inside(C3DFVector& p, const C3DFVector& searchdir) const 
{
        if (is_inside(p))
                return true; 

        // find intersection of the ray with the box
        auto inv_searchdir = C3DFVector::_1 / searchdir; 
        
        auto p0 = (-1.0f * p) * inv_searchdir;
        auto p1 = (m_fsize - p) * inv_searchdir; 
        
        auto tmin = max(max(min(p0.x, p1.x), min(p0.y, p1.y)), min(p0.z, p1.z)); 
        auto tmax = min(min(max(p0.x, p1.x), max(p0.y, p1.y)), max(p0.z, p1.z)); 


        // the ray would hit, but the box is behind us 
        if (tmax < 0) 
                return false; 

        // ray doesn't intersect 
        if (tmin > tmax) 
                return false; 
        
        // ray intersects, adjust point 
        p += tmin * searchdir; 
        return true; 
}

void C3DDrawBox::draw_line_pivot_x(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
        if (v.x < 0) {
                swap(x,y); 
                v /= v.x; 
        }
        while (x.x <= y.x) {
                draw_point(x); 
                x += v; 
        }
}

void C3DDrawBox::draw_line_pivot_y(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
        if (v.y < 0) {
                swap(x,y); 
                v /= v.y; 
        }
        while (x.y <= y.y) {
                draw_point(x); 
                x += v; 
        }
}

void C3DDrawBox::draw_line_pivot_z(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
        if (v.z < 0) {
                swap(x,y); 
                v /= v.z; 
        }
        while (x.z <= y.z) {
                draw_point(x); 
                x += v; 
        }
}


void C3DDrawBox::draw_line(const C3DFVector& a, const C3DFVector& b)
{
       C3DFVector x = (a - m_origin ) * m_stepping; 
       C3DFVector y = (b - m_origin ) * m_stepping; 
       draw_line_internal(x, y); 
}

void C3DDrawBox::draw_line_internal(const C3DFVector& _x, const C3DFVector& _y)
{        
        C3DFVector x = _x;
        C3DFVector y = _y;
        
        C3DFVector v = y - x;
        float vn2 = v.norm2(); 
        
        if (vn2 < 0.5f) { // the line is only one point
                // testing if it is inside is done 
                // within the method
                draw_point(x); 
                return; 
        }
       
        // clip the line with the box boundaries 
        // early return if the ray does not hit the box in the search direction 
        if (!make_inside(x,v)) 
                return; 
        
        if (!make_inside(y,-1.0f*v)) 
                return;
        
        C3DFVector av(fabs(v.x), fabs(v.y), fabs(v.z));  
        
        // draw along the fastest changing coordinate 
        if (av.x > av.y) {
                if (av.x > av.z) {
                        draw_line_pivot_x(x, y, v); 
                }else{
                        draw_line_pivot_z(x, y, v); 
                }
        } else {
                if (av.y > av.z) {
                        draw_line_pivot_y(x, y, v); 
                }else{
                        draw_line_pivot_z(x, y, v); 
                }
        }
}

void C3DDrawBox::draw_triangle(const C3DFVector& a, const C3DFVector& b, const C3DFVector& c)
{
        C3DFVector x = (a - m_origin ) * m_stepping; 
        C3DFVector y = (b - m_origin ) * m_stepping; 
        C3DFVector z = (c - m_origin ) * m_stepping; 
        
        C3DFVector e1 = x - y; 
        C3DFVector e2 = y - z; 
        C3DFVector e3 = z - x; 

        auto e1n = e1.norm2(); 
        auto e2n = e2.norm2(); 
        auto e3n = e3.norm2(); 

        enum pivot_edge { pe_xy, pe_yz, pe_zx}; 
        
        pivot_edge pe = pe_xy; 

        if (e1n < e2n) {
                swap(e1,e2); 
                swap(e1n,e2n); 
                pe = pe_yz; 
        }
        if (e1n < e3n) {
                pe = pe_zx; 
        }

        switch (pe) {
        case pe_xy: draw_triangle_internal(a, b, c); break; 
        case pe_yz: draw_triangle_internal(b, c, a); break; 
        case pe_zx: draw_triangle_internal(c, a, b); break; 
        }
}


static float distance_point_infinite_line(const C3DFVector& point,
                                          const C3DFVector& a,
                                          const C3DFVector& b)
{
	if (point == a || point == b)
		return 0.0;

	const C3DFVector line = a - b;
	const C3DFVector p_end = point - b;
	const float nline =  line.norm2();
	const float npoint = p_end.norm2();
	if (nline > 0.0 ) {
		const float dotlplt = dot(line, p_end) / nline;

		const C3DFVector proj = b + dotlplt * line;
		return (point - proj).norm();
	}else // special case: line segment is actually a point
		return sqrt(npoint);
}

void C3DDrawBox::draw_triangle_internal(const C3DFVector& a, const C3DFVector& b, const C3DFVector& c)
{
        // a-b is the longest edge and we draw all lines parallel to it
        
        float dist = distance_point_infinite_line(c, a, b);
        draw_line_internal(a, b); 

        if (dist > 0.5f) {
                C3DFVector e1 = (c - a) / dist; 
                C3DFVector e2 = (c - b) / dist; 
                
                for (float d = 0; d < dist; d += 0.9) {
                        draw_line_internal(a + d * e1, b + d * e2);
                }
        }
}

template class EXPORT_3D T3DImageDrawTarget<bool>; 
template class EXPORT_3D T3DImageDrawTarget<unsigned char>; 
template class EXPORT_3D T3DImageDrawTarget<signed char>; 
template class EXPORT_3D T3DImageDrawTarget<unsigned short>; 
template class EXPORT_3D T3DImageDrawTarget<signed short>; 
template class EXPORT_3D T3DImageDrawTarget<unsigned int>; 
template class EXPORT_3D T3DImageDrawTarget<signed int>; 
template class EXPORT_3D T3DImageDrawTarget<float>; 
template class EXPORT_3D T3DImageDrawTarget<double>; 

NS_MIA_END
