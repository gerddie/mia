/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_3d_landmarklist_hh
#define mia_3d_landmarklist_hh

#include <map> 
#include <mia/core/iodata.hh>
#include <mia/3d/landmark.hh>

NS_MIA_BEGIN
/**
   @ingroup basic 
   \brief A list of named landmarks 
   
   This class implements a list of uniquely named landmarks. 
   
 */

class EXPORT_3D C3DLandmarklist : public CIOData {
	typedef std::map<std::string, P3DLandmark> CMap; 
public: 

	/// Type of the map values (needed for lambda arguments)
	typedef CMap::value_type value_type; 

	/// read only iterator 
	typedef CMap::const_iterator const_iterator; 

	/// read-write iterator 
	typedef CMap::iterator iterator;  

	/**
	   \cond INTERNAL 
	   \brief helper for plugin handling 
	 */
	static const char *data_descr;

	/// type helper for plugin handler 
	typedef C3DLandmarklist type; 
	/// @endcond 

	typedef std::shared_ptr<C3DLandmarklist> Pointer; 

	/**
	   Add a Landmark to the list. If the landmark name already exists 
	   it is siletly overwritten.  
	   \param lm the landmark to be added 
	 */
	void add(P3DLandmark lm); 
	
	
	/**
	   \returns a dynamically created copy of the landmark list 
	 */
	C3DLandmarklist *clone() const; 

	/**
	   \returns the landmark with the giben name. If this landmark 
	   doesn't exist the function returns a (shared) null pointer.
	 */
	P3DLandmark get(const std::string& name) const; 

	/// \returns a read-only STL iterator to the landmarks 
	const_iterator begin() const; 
	/// \returns a read-only STL iterator to the landmarks 
	const_iterator end() const; 

	/// \returns a read-write STL iterator to the landmarks 
	iterator begin(); 

	/// \returns a read-write STL iterator to the landmarks 
	iterator end(); 

	/**
	   Set the root path of the document (used to read the images) 
	   \param path
	 */
	void set_path(const std::string& path); 

	/**
	   Set the name of the landmark list
	   \param name 
	 */
	void set_name(const std::string &name); 

	/**
	   \returns the number of landmarks stored in the list 
	 */
	size_t size() const; 

	/// \returns the name of the landmark list 
	const std::string &get_name() const; 
private: 
	CMap m_list; 

	std::string m_path; 
	std::string m_name; 
}; 


/// Pointer type for land mark list 
typedef C3DLandmarklist::Pointer P3DLandmarklist; 

NS_MIA_END
#endif

