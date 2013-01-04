/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 David Paster, Gert Wollny
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


NS_MIA_BEGIN

template <typename Transformation>
TTrackPoint<Transformation>::TTrackPoint(int id, float time, const Vector& pos, const std::string& reserved):
	m_id(id), 
	m_time(time), 
	m_pos(pos), 
	m_reserved(reserved)
{
}


template <typename Transformation>
TTrackPoint<Transformation>::TTrackPoint():m_id(-1), 
	m_time(0.0)
{
}

template <typename Transformation>
bool TTrackPoint<Transformation>::read(std::string& istr)
{
	
	std::istringstream is(istr); 

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

	if (!NDVectorIOcvd<Vector>::read(is, m_pos))
		return false;

	if (is.eof()) 
		return true;

	is >> c; 
	if (c != ';') 
		return false; 

	// read the remaining fields as one string 
	getline(is, m_reserved); 
	return true; 
}

template <typename Transformation>
void TTrackPoint<Transformation>::print(std::ostream& os) const
{
	os << m_id << ';' << m_time << ';';
	NDVectorIOcvd<Vector>::write(os, m_pos); 

	if (!m_reserved.empty())
		os << ';' << m_reserved;
}

template <typename Transformation>
void TTrackPoint<Transformation>::move(float timestep, const Transformation& t)
{
	m_pos -= timestep * t.apply(m_pos);
	m_time += timestep; 
}

template <typename Transformation>
const typename TTrackPoint<Transformation>::Vector&  
TTrackPoint<Transformation>::get_pos() const
{
	return m_pos; 
}


template <typename Transformation>
int TTrackPoint<Transformation>::get_id() const
{
	return m_id; 
}


template <typename Transformation>
float TTrackPoint<Transformation>::get_time() const
{
	return m_time; 
}

template <typename Transformation>
const std::string&  TTrackPoint<Transformation>::get_reserved() const
{
	return m_reserved;
}

template <typename Transformation>
std::vector< TTrackPoint<Transformation> > 
TTrackPoint<Transformation>::load_trackpoints(const std::string& in_filename)
{
	std::vector< TTrackPoint<Transformation> > result;

	std::ifstream input(in_filename.c_str()); 

	if (input.bad()) 
		throw create_exception<std::runtime_error>("Unable to open file '", in_filename, "' for reading");

	while (input.good()) {
		std::string input_line; 
		getline(input, input_line);
		if (input_line.empty())
			break; 
		
		if (input.good()) {
			TTrackPoint<Transformation> pt; 
			if (pt.read(input_line)) 
				result.push_back(pt); 
			else 
				cverr() << "Bogus input line '" << input_line << "' ignored\n";
		}
	}
	return result; 
}

NS_MIA_END
