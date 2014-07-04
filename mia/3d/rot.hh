/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_3d_rot_hh
#define mia_3d_rot_hh

#include <mia/3d/quaternion.hh>
#include <mia/3d/matrix.hh>

NS_MIA_BEGIN

class C3DRotation {
        
public: 
        virtual C3DDMatrix as_matrix_3x3() const = 0;
        virtual Quaternion as_quaternion() const = 0; 
        virtual std::string as_string() const = 0; 
        
        static C3DRotation* from_string(const std::string& s) __attribute__((warn_unused_result));

}; 

class C3DQuaternionRotation: public C3DRotation {

        C3DQuaternionRotation(const Quaternion& q); 
        
        virtual C3DDMatrix as_matrix_3x3() const;
        virtual Quaternion as_quaternion() const; 
        virtual std::string as_string() const; 

private: 
        Quaternion m_q;
        
}; 

class C3DMatrix3x3Rotation: public C3DRotation {

        C3DMatrix3x3Rotation(const C3DDMatrix& q); 
        
        virtual C3DDMatrix as_matrix_3x3() const;
        virtual Quaternion as_quaternion() const; 
        virtual std::string as_string() const; 

private: 

        C3DDMatrix m_q;
}; 

NS_MIA_END
#endif
