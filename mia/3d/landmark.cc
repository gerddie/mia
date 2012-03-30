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

#include <mia/3d/landmark.hh>
#include <mia/2d/2dimageio.hh>

NS_MIA_BEGIN

using boost::filesystem::path; 
using std::string; 

C3DLandmark::C3DLandmark(const string& _name, const C3DFVector& _position):
	m_name(_name), 
	m_position(_position) 
{
}

const string& C3DLandmark::get_name() const
{
	return m_name; 
}

const C3DFVector& C3DLandmark::get_position() const
{
	return m_position; 
}

void C3DLandmark::set_position(const C3DFVector& pos)
{
	m_position = pos; 
}


void C3DLandmark::set_view(const C3DCamera& view)
{
	m_view = view; 
}

void C3DLandmark::set_picture_file(const string& picfile)
{
	m_picfile = picfile; 
	m_picture.reset(); 
}

void C3DLandmark::set_isovalue(float value)
{
	m_isovalue = value; 
}

P2DImage C3DLandmark::get_picture(const path& root_path) const
{
	if (!m_picture && !m_picfile.empty()) {
		path full_path = root_path / path(m_picfile); 
		m_picture = load_image2d(full_path.string()); 
	}
	return m_picture; 
}
NS_MIA_END


