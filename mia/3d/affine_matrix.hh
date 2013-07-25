/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_affine_matrix_hh
#define mia_3d_affine_matrix_hh

#include <mia/3d/quaternion.hh>

NS_MIA_BEGIN
	
class CAffinTransformMatrix {

public: 
        CAffinTransformMatrix(); 
        CAffinTransformMatrix(float a11, float a12, float a13, float a14, 
			      float a21, float a22, float a23, float a24, 
			      float a31, float a32, float a33, float a34); 

        void rotate_x(const C3DFVector& center, float angle); 
        void rotate_y(const C3DFVector& center, float angle); 
        void rotate_z(const C3DFVector& center, float angle); 
        void rotate(const C3DFVector& center, const Quaternion& q);
        void scale(const C3DFVector& center, const C3DFVector& scale); 
        void translate(const C3DFVector& shift); 
        void transpose(); 

        void identity();

        CAffinTransformMatrix& operator *= (const CAffinTransformMatrix& other); 
        
        const CAffinTransformMatrix inverse() const; 
	
        C3DFVector operator * (const C3DFVector& x) const;

	const std::vector<float>& data() const; 
private: 
	std::vector<float> m_matrix;
	
}; 
	
CAffinTransformMatrix operator * (const CAffinTransformMatrix& lhs, const CAffinTransformMatrix& rhs); 


NS_MIA_END
#endif 
