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


class CAffinTransformMatrix {

public: 
        CAffinTransformMatrix(); 
        CAffinTransformMatrix(const CAffinTransformMatrix& org) = default; 

        void rotate_x(const C3FVector& center, float angle); 
        void rotate_y(const C3FVector& center, float angle); 
        void rotate_z(const C3FVector& center, float angle); 
        void rotate(const Quaternion& q);
        void scale(const C3FVector& center, const C3FVector& scale); 
        void translate(const C3FVector& shift); 


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
        CAffinTransformMatrix(const CAffinTransformMatrix& org) = default; 

        void rotate_x(const C3FVector& center, float angle); 
        void rotate_y(const C3FVector& center, float angle); 
        void rotate_z(const C3FVector& center, float angle); 
        void rotate(const Quaternion& q);
        void scale(const C3FVector& center, const C3FVector& scale); 
        void translate(const C3FVector& shift); 

        void identity();

        CAffinTransformMatrix& operator *= (const CAffinTransformMatrix& other); 
        
        const CAffinTransformMatrix inverse() const; 

        C3DFVector operator * (const C3DFVector& x) const;

        friend C3DFVector operator * (const C3DFVector& x, const CAffinTransformMatrix& m); 
private: 
        vector<float> m_matrix;

}; 


NS_MIA_END
#endif 
