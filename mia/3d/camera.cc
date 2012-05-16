
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

#include <mia/3d/camera.hh>

NS_MIA_BEGIN

C3DCamera::C3DCamera(const C3DFVector& loc, const Quaternion& rot, float zoom):
m_location(loc), 
	m_rotation(rot), 
	m_zoom(zoom)
{
}
  


C3DFVector C3DCamera::get_location() const
{
	return m_location; 
}


Quaternion C3DCamera::get_rotation() const
{
	return m_rotation; 
}


float C3DCamera::get_zoom() const
{
	return m_zoom; 
}

bool EXPORT_3D operator == (const C3DCamera& a, const C3DCamera& b)
{
	if (a.get_zoom() != b.get_zoom()) 
		return false; 

	if (a.get_location() != b.get_location()) 
		return false; 
	
	return a.get_rotation() == b.get_rotation(); 
}

void C3DCamera::print(std::ostream& os)const 
{
	os << "Camera{loc=" << m_location << ", zoom=" 
	   << m_zoom << ", rot=" << m_rotation << "}"; 
}

NS_MIA_END