/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_3d_camera_hh
#define mia_3d_camera_hh

#include <mia/3d/quaternion.hh>

NS_MIA_BEGIN

/**
   \ingroup visual
   \brief this class defines the view point for 3D visualization 
   
   
 */
class EXPORT_3D C3DCamera {
public: 
	C3DCamera() = default;

	/**
	   Constructor 
	   \param loc viewpoint location 
	   \param rot looking direction 
	   \param zoom camera zoom 
	*/
	C3DCamera(const C3DFVector& loc, const Quaternion& rot, float zoom); 

	/// \returns the location of the camera 
	C3DFVector get_location() const; 

	/// \returns the view direction as rotation 
	Quaternion get_rotation() const;

	/// \returns get the camera zoom 
	float get_zoom() const; 
 private: 
	C3DFVector m_location; 
	Quaternion m_rotation;
	float m_zoom; 
	
	

}; 

NS_MIA_END
#endif
