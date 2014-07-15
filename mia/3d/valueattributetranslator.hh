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

#ifndef mia_3d_valueattributetranslator_hh
#define mia_3d_valueattributetranslator_hh

#include <mia/core/attributes.hh>
#include <mia/3d/vector.hh>
#include <mia/3d/defines3d.hh>

NS_MIA_BEGIN

/**
   @ingroup basic 
   @brief a translater for 3D vectors to and from a std::string
*/
template <typename T>
class EXPORT_3D C3DValueAttributeTranslator: public CAttrTranslator {
public:
	static  bool register_for(const std::string& key);
private:
	PAttribute do_from_string(const std::string& value) const;
};

/**
   This class is a hack to work around the vista voxel size stringyfied  value. 
   Normaly one would write "x,y,z" but in vista it is "x y z", which means a different 
   translator is needed as compared to a T3DVector. 
   For everything else the T3DVector interpretation is used (based on type_id); 
   

   @ingroup basic 
   @brief a 3D vector value used in attributes 
   @tparam T the data type of the vector elements 
*/
template <typename T>
class EXPORT_3D C3DValueAttribute : public CAttribute {
public:

	/**
	   Constructor to initialize the attribute by using a 3D Vector value 
	   @param value 
	 */
	C3DValueAttribute(const T3DVector<T>& value);
	
	/// \returns the value of the attribute as 3D vector 
	operator T3DVector<T>()const;

	/**
	   Obtain a run-time unique type description of the value type 
	   @returns the typeid of the T3DVector<T>
	 */
	const char *typedescr() const	{
		return typeid(T3DVector<T>).name();
	}
	
	// 
	int type_id() const {
		return 	 attribute_type<T3DVector<T>>::value; 
	}
private:
	std::string do_as_string() const;
	bool do_is_equal(const CAttribute& other) const;
	bool do_is_less(const CAttribute& other) const;
	T3DVector<T> m_value;
};

/**
   @ingroup basic 
   @brief a 3D floating point vector used for the voxel size attribute 
*/
typedef C3DValueAttribute<float> CVoxelAttribute;

/**
   @ingroup basic 
   @brief attribute translator for a 3D floating point vector used for the voxel size
*/
typedef C3DValueAttributeTranslator<float> CVoxelAttributeTranslator;

/**
   @ingroup basic 
   @brief a 3D integer vector
*/
typedef C3DValueAttribute<int> C3DIntAttribute;

/**
   @ingroup basic 
   @brief attribute translator for a 3D integer vector
*/
typedef C3DValueAttributeTranslator<int> C3DIntAttributeTranslator;

NS_MIA_END

#endif 
