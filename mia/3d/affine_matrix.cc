

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


#include <mia/3d/affine_matrix.hh>

NS_MIA_BEGIN

CAffinTransformMatrix::CAffinTransformMatrix():m_matrix(16)
{
        identity();
}


void CAffinTransformMatrix::rotate_x(const C3FVector& center, float angle)
{
        float c, s; 
        sincosf(angle, &s, &c); 

        float ac[8]; 
        float as[8]; 
        
        for (int i =0; i < 8; ++i) {
                ac[i] = m_matrix[4 + i] * c;
                as[i] = m_matrix[4 + i] * s;
        }

      
}

void CAffinTransformMatrix::rotate_y(const C3FVector& center, float angle); 
void CAffinTransformMatrix::rotate_z(const C3FVector& center, float angle); 
void CAffinTransformMatrix::rotate(const C3FVector& center, const Quaternion& q);
void CAffinTransformMatrix::scale(const C3FVector& center, const C3FVector& scale); 
void CAffinTransformMatrix::translate(const C3FVector& shift); 

void CAffinTransformMatrix::identity()
{
        m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = 1.0f; 
}

CAffinTransformMatrix& CAffinTransformMatrix::operator *= (const CAffinTransformMatrix& other); 

const CAffinTransformMatrix CAffinTransformMatrix::inverse() const; 

C3DFVector CAffinTransformMatrix::operator * (const C3DFVector& x) const;

C3DFVector operator * (const C3DFVector& x, const CAffinTransformMatrix& m); 

NS_MIA_END
