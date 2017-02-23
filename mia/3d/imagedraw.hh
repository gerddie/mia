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

#ifndef mia_3d_imagedraw_hh
#define mia_3d_imagedraw_hh


#include <mia/3d/image.hh>

NS_MIA_BEGIN

/**
   \brief base class for a 3D volume raster graphics draw target 
   
   
*/

class EXPORT_3D C3DDrawBox {
public: 
	C3DDrawBox(const C3DBounds& size, const C3DFVector& origin, const C3DFVector& spacing); 
	
	void draw_point(const C3DFVector& p); 

        /**
           Draw a line from a to b in the volume with the current draw color. 
           The line will be clipped at the output boundaries.
           \param a 
           \param b 
         */
	void draw_line(const C3DFVector& a, const C3DFVector& b); 

        /**
           Draw a filled triangle with the corners a, b, d in the volume with the 
           current draw color. 
           The triangle will be clipped at the output boundaries.
           \param a 
           \param b 
           \param c
        */

	void draw_triangle(const C3DFVector& a, const C3DFVector& b, const C3DFVector& c); 
	
private: 
	/**
	   \returns true if the given point is inside the drawing volume
	*/

	bool is_inside(const C3DFVector& p) const; 

	/**
	   This function moves p along and in the direction of searchdir
	   so that it is inside the draw volume
	 */
	bool make_inside(C3DFVector& p, const C3DFVector& searchdir) const;

	void draw_line_pivot_x(C3DFVector& x, C3DFVector& y, C3DFVector& v); 
	void draw_line_pivot_y(C3DFVector& x, C3DFVector& y, C3DFVector& v); 
	void draw_line_pivot_z(C3DFVector& x, C3DFVector& y, C3DFVector& v); 

	void draw_triangle_internal(const C3DFVector& a, const C3DFVector& b, const C3DFVector& c); 
	void draw_line_internal(const C3DFVector& x, const C3DFVector& y); 

	// check wether the bounding box enclosing the triangle overlaps with the drawing area
	bool has_overlap(const C3DFVector& x, const C3DFVector& y, const C3DFVector& z); 

	virtual void do_draw_point(const C3DBounds& p) = 0; 
	
	C3DBounds m_size; 
	C3DFVector m_fsize; 
	C3DFVector m_origin; 
	C3DFVector m_stepping; 
	
}; 


/**
   \brief implements T3DImage as a target for drawing operations 

*/
template <typename T> 
class T3DImageDrawTarget : public C3DDrawBox {
        
public: 
        /**
           Create a draw target of the given output size and the given voxel spacing
         */
        T3DImageDrawTarget(const C3DBounds& size, const C3DFVector& origin, const C3DFVector& spacing); 


        /// \param c New drawing color  
        void set_color(T c); 

        
        /// \returns the draw target 
        const T3DImage<T>& get_image() const; 
private: 
	void do_draw_point(const C3DBounds& p); 
	
        T3DImage<T> m_target; 
        T m_color; 
}; 

typedef T3DImageDrawTarget<bool> C3DBitImageDrawTarget; 
typedef T3DImageDrawTarget<unsigned char> C3DUBImageDrawTarget; 
typedef T3DImageDrawTarget<signed char> C3DSBImageDrawTarget; 
typedef T3DImageDrawTarget<unsigned short> C3DUSImageDrawTarget; 
typedef T3DImageDrawTarget<signed short> C3DSSImageDrawTarget; 
typedef T3DImageDrawTarget<unsigned int> C3DUIImageDrawTarget; 
typedef T3DImageDrawTarget<signed int> C3DSIImageDrawTarget; 
typedef T3DImageDrawTarget<float> C3DFImageDrawTarget; 
typedef T3DImageDrawTarget<double> C3DDImageDrawTarget; 

NS_MIA_END

#endif
