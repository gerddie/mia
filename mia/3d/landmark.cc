/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/3d/landmark.hh>
#include <mia/2d/imageio.hh>

NS_MIA_BEGIN

using boost::filesystem::path; 
using namespace std; 

C3DLandmark::C3DLandmark(const string& _name):
	m_name(_name), 
	m_has_location(false), 
	m_isovalue(0.0)
{
}

C3DLandmark::C3DLandmark(const std::string& name, const C3DFVector& location):
	m_name(name),
	m_has_location(true), 
	m_location(location), 
	m_isovalue(0.0)
{
}

const C3DCamera& C3DLandmark::get_view() const
{
	return m_view; 
}

const string& C3DLandmark::get_name() const
{
	return m_name; 
}

const C3DFVector& C3DLandmark::get_location() const
{
	if (!m_has_location) 
		throw create_exception<runtime_error>("C3DLandmark::get_location: Landmark '", get_name(), "' has no location");
	return m_location; 
}

bool C3DLandmark::has_location() const 
{
	return m_has_location; 
}

void C3DLandmark::set_location(const C3DFVector& pos)
{
	m_location = pos; 
	m_has_location = true;
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

const std::string& C3DLandmark::get_picture_file()const
{
	return m_picfile; 
}

void C3DLandmark::set_isovalue(float value)
{
	m_isovalue = value; 
}

float C3DLandmark::get_isovalue()const
{
	return m_isovalue; 
}

P2DImage C3DLandmark::get_picture(const path& root_path) const
{
	if (!m_picture && !m_picfile.empty()) {
		path full_path = root_path / path(m_picfile); 
		m_picture = load_image2d(full_path.string()); 
	}
	return m_picture; 
}

void C3DLandmark::print(std::ostream& os) const
{
	os << "Landmark{" 
	   << "   name='" << m_name  << "'\n"
	   << "   iso=" << m_isovalue << "'\n"
	   << "   location=";  
	
	if (m_has_location) 
		os << m_location; 
	else 
		os << "none"; 
	os << "\n"; 
	os << "   view=" << m_view << "\n}";
}

bool EXPORT_3D operator == (const C3DLandmark& a, const C3DLandmark& b)
{
	if (a.get_name() != b.get_name()) 
		return false; 
	if (a.get_isovalue() != b.get_isovalue())
		return false; 
	if (a.has_location() != b.has_location())
		return false; 

	if (a.has_location()) {
		if (a.get_location() != b.get_location()) 
			return false; 
	}
	
	return a.get_view() == b.get_view(); 
	
}


NS_MIA_END


