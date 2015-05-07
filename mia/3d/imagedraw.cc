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
        m_fsize(size), 
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
       
	cvdebug() << "about to draw " << ip << " from " << p << "\n"; 
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
inline bool evaluate_t(float& _min, float& _max, float size, float p, float dir, bool t_initialized)
{
	if (dir != 0.0f)  {
		float invx = 1.0f / dir; 
		float p0 = - p * invx; 
		float p1 = (size - p) * invx; 
		if (!t_initialized) {
			_min = min(p0, p1); 
			_max = max(p0, p1); 
		}else{
			_min = max(min(p0, p1), _min); 
			_max = min(max(p0, p1), _max); 
		}
		return true; 
	}else{
		return false; 
	}
} 

bool C3DDrawBox::make_inside(C3DFVector& p, const C3DFVector& searchdir) const 
{
        if (is_inside(p))
                return true; 

        // find intersection of the ray with the box
	float  tmin = 0; 
	float  tmax = 0;  

	// check if we move parallel to the box and are outside
	if (searchdir.x == 0.0) {
		if (p.x < 0 || p.x > m_size.x) 
			return false; 
	}
	if (searchdir.y == 0.0) {
		if (p.y < 0 || p.y > m_size.y) 
			return false; 
	}
	if (searchdir.x == 0.0) {
		if (p.z < 0 || p.z > m_size.z) 
			return false; 
	}
	
	
	bool t_initialized = evaluate_t(tmin, tmax, m_fsize.x, p.x, searchdir.x, false);
	t_initialized = evaluate_t(tmin, tmax, m_fsize.y, p.y, searchdir.y, t_initialized); 
	t_initialized = evaluate_t(tmin, tmax, m_fsize.z, p.z, searchdir.z, t_initialized);

	cvdebug() << "search = " << p << " -> " <<  searchdir
		  << "; tmin = " << tmin << ", tmax= "<< tmax 
		  << "\n"; 

        // the ray would hit, but the box is behind us 
        if (tmax < 0) 
                return false; 

        // ray doesn't intersect 
        if (tmin > tmax) 
                return false; 
        
        // ray intersects, adjust point 
        p += tmin * searchdir; 
	cvdebug() << "boundary point = " << p << "\n"; 
        return true; 
}

void C3DDrawBox::draw_line_pivot_x(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
        if (v.x < 0) {
                swap(x,y); 
		v *= -1.0f; 
        }
	v /= 2.0 * v.x; 
        while (x.x <= y.x) {
                draw_point(x); 
                x += v; 
        }
}

void C3DDrawBox::draw_line_pivot_y(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
	cvdebug() << "C3DDrawBox::draw_line_pivot_y: " << x << " - " << y <<   " > " << v << "\n"; 
        if (v.y < 0) {
                swap(x,y); 
		v *= -1.0f; 
        }
	v /= 2.0 * v.y; 
        while (x.y <= y.y) {
                draw_point(x); 
                x += v; 
        }
}

void C3DDrawBox::draw_line_pivot_z(C3DFVector& x, C3DFVector& y, C3DFVector& v)
{
        if (v.z < 0) {
                swap(x,y); 
		v *= -1.0f; 
        }
	v /= 2.0 * v.z; 
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
	cvdebug() << "C3DDrawBox::draw_line_internal: " << _x << " - " << _y << "\n"; 
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

bool C3DDrawBox::has_overlap(const C3DFVector& x, const C3DFVector& y, const C3DFVector& z) 
{
	// check if at least one corner is inside 
	// 
	if (x >= C3DFVector::_0 &&  x <= m_fsize) 
		return true; 

	if (y >= C3DFVector::_0 &&  y <= m_fsize) 
		return true; 
	
	if (z >= C3DFVector::_0 &&  z <= m_fsize) 
		return true; 

	C3DFVector c0(x); 
	C3DFVector c1(x); 

	if (y.x < c0.x) c0.x = y.x; 
	if (y.y < c0.y) c0.y = y.y; 
	if (y.z < c0.z) c0.z = y.z; 

	if (y.x > c1.x) c1.x = y.x; 
	if (y.y > c1.y) c1.y = y.y; 
	if (y.z > c1.z) c1.z = y.z; 

	if (z.x < c0.x) c0.x = z.x; 
	if (z.y < c0.y) c0.y = z.y; 
	if (z.z < c0.z) c0.z = z.z; 

	if (z.x > c1.x) c1.x = z.x; 
	if (z.y > c1.y) c1.y = z.y; 
	if (z.z > c1.z) c1.z = z.z; 

	if (c1.x < 0.0 || c0.x > m_fsize.x) 
		return false; 
	
	if (c1.y < 0.0 || c1.y > m_fsize.y) 
		return false; 

	if (c1.z < 0.0 || c1.z > m_fsize.z) 
		return false; 

	return true; 
	
}

void C3DDrawBox::draw_triangle(const C3DFVector& a, const C3DFVector& b, const C3DFVector& c)
{
	cvdebug() << "C3DDrawBox::draw_triangle: [" << a << "],[" << b << "],[" << c << "]\n"; 

        C3DFVector x = (a - m_origin ) * m_stepping; 
        C3DFVector y = (b - m_origin ) * m_stepping; 
        C3DFVector z = (c - m_origin ) * m_stepping; 

	if (!has_overlap(x,y,z)) {
		cvdebug() << "skip triangle (["<< a <<"]["<< b <<"]["<< c <<"]) since it it doesn't overlap \n"; 
		return; 
	}
        
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
                
                for (float d = 0; d < dist; d += 0.5) {
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
