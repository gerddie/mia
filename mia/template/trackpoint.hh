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

#ifndef mia_template_trackpoint_hh
#define mia_template_trackpoint_hh

#include <istream>
#include <ostream>

#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

/**
   \brief Class to track pixel movement based on a transformation
   \tparam transformation type to be considered 
   
   This class holds the information to track a single pixel in ND space (defined 
   by the transformation) over time and with a given transformation.  
*/


template <typename Transformation>
class EXPORT_HANDLER TTrackPoint {
public: 
	typedef typename Transformation::Vector Vector; 
	
	
	TTrackPoint(); 

	/**
	   Initialize a new tracking point
	   \param id 
	   \param time 
	   \param pos 
	   \param reserved 
	 */


	TTrackPoint(int id, float time, const Vector& pos, const std::string& reserved); 
	
	/**
	   Read the point from an input line. The point must be of format 
	   id;time;x;y;z[;reserved]
	   \param is input string to read the point from
	   \returns true if the track point was read successfully, false otherwise 
	*/
	bool read(std::string& is); 
	
	/**
	   print the point to an output stream 
	   \param os output stream 
	 */
	void print(std::ostream& os) const; 

	/**
	   Mova the pixel according tho a time step and based on the given transformation
	   \param timestep the time step to apply
	   \param t the transformation to apply 
	 */
	void move(float timestep, const Transformation& t); 


	/// \returns the current position
	const Vector&  get_pos() const; 

	/// \returns the point ID
	int get_id() const;
	
	/// \returns the current time value 
	float get_time() const; 
	
	/// returns the reserved field 
	const std::string&  get_reserved() const; 
	
	/**
	   Load the trackpoints from an input file 
	   \param in_filename input file name in csv format 
	   \returns the list of track points 
	*/
	
	static std::vector< TTrackPoint > load_trackpoints(const std::string& in_filename); 

private:
	int m_id; 
	float m_time; 
	Vector m_pos; 
	std::string m_reserved; 
}; 

template <typename Transformation>
std::ostream& operator << (std::ostream& os,  const TTrackPoint<Transformation>& tp)
{
	tp.print(os); 
	return os; 
}


NS_MIA_END



#endif 
