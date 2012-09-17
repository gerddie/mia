/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 David Paster, Gert Wollny
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

#include <mia/3d/trackpoint.hh>

NS_MIA_BEGIN
using namespace std; 

C3DTrackPoint::C3DTrackPoint(int id, float time, const C3DFVector& pos, const std::string& reserved):
	m_id(id), 
	m_time(time), 
	m_pos(pos), 
	m_reserved(reserved)
{
}



C3DTrackPoint::C3DTrackPoint():m_id(-1), 
	m_time(0.0)
{
}

bool C3DTrackPoint::read(string& istr)
{
	
	istringstream is(istr); 

	char c; 
	is >> m_id; 
	
	// was only whitespaces 
	if (is.eof()) 
		return false; 
	is >> c; 
	if (c != ';') 
		return false; 
	is >> m_time; 
	is >> c; 
	if (c != ';') 
		return false; 

	is >> m_pos.x; 
	is >> c; 
	if (c != ';') 
		return false;

	is >> m_pos.y; 
	is >> c; 
	if (c != ';') 
		return false; 
	
	is >> m_pos.z;
	if (is.eof()) 
		return true;
	is >> c; 
	if (c != ';') 
		return false; 

	// read the remaining fields as one string 
	getline(is, m_reserved); 
	return true; 
}

void C3DTrackPoint::print(ostream& os) const
{
	os << m_id << ";" << m_time << ";"; 
	os << m_pos.x << ";" << m_pos.y << ";" << m_pos.z; 
	
	if (!m_reserved.empty())
		os << ";" << m_reserved;
}

void C3DTrackPoint::move(float timestep, const C3DTransformation& t)
{
	m_pos -= timestep * t.apply(m_pos);
	m_time += timestep; 
}


const C3DFVector&  C3DTrackPoint::get_pos() const
{
	return m_pos; 
}


int C3DTrackPoint::get_id() const
{
	return m_id; 
}


float C3DTrackPoint::get_time() const
{
	return m_time; 
}

const std::string&  C3DTrackPoint::get_reserved() const
{
	return m_reserved;
}

/**
   Load the trackpoints from an input file 
   \param in_filename input file name in csv format 
   \returns the list of track points 
*/
vector< C3DTrackPoint > load_trackpoints(const string& in_filename)
{
	vector< C3DTrackPoint > result;

	ifstream input(in_filename.c_str()); 

	if (input.bad()) 
		throw create_exception<runtime_error>("Unable to open file '", in_filename, "' for reading");

	while (input.good()) {
		string input_line; 
		getline(input, input_line);
		if (input_line.empty())
			break; 
		
		if (input.good()) {
			C3DTrackPoint pt; 
			if (pt.read(input_line)) 
				result.push_back(pt); 
			else 
				cverr() << "Bogus input line '" << input_line << "' ignored\n";
		}
	}
	return result; 
}


NS_MIA_END
