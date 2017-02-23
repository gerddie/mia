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

#ifndef mia_3d_rot_hh
#define mia_3d_rot_hh

#include <mia/core/attributes.hh>
#include <mia/3d/quaternion.hh>
#include <mia/3d/matrix.hh>

NS_MIA_BEGIN

class EXPORT_3D C3DRotation {
public: 
	C3DRotation(); 
	~C3DRotation();
	
	C3DRotation(const C3DRotation& other); 
	C3DRotation& operator = (const C3DRotation& other); 

	C3DRotation(const C3DDMatrix& m); 
	C3DRotation(const Quaternion& q); 
	C3DRotation(const std::string& s); 

	C3DDMatrix as_matrix_3x3() const;
	Quaternion as_quaternion() const; 
	std::string as_string() const; 

	static const C3DRotation _1; 

private: 
	class C3DRotationImpl *impl;
}; 




template <>
struct attribute_type<C3DRotation>  {
        static const int value = 0x4000;
}; 

template <>
struct dispatch_attr_string<C3DRotation> {
	static std::string val2string(const C3DRotation& value) {
		return value.as_string();
	}
	static C3DRotation string2val(const std::string& str) {
		return C3DRotation(str); 
	}
}; 


EXPORT_3D bool operator == (const C3DRotation& lhs, const C3DRotation& rhs); 
EXPORT_3D bool operator < (const C3DRotation& lhs, const C3DRotation& rhs); 

typedef TAttribute<C3DRotation> C3DRotationAttribute; 
typedef TTranslator<C3DRotation> C3DRotationAttributeTranslate;

NS_MIA_END
#endif
