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

#ifndef mia_3d_trackpoint_hh
#define mia_3d_trackpoint_hh

#include <mia/3d/transform.hh>

NS_MIA_BEGIN

/**
   \brief Class to track pixel movement based on a transformation

   This class holds the information to track a single pixel in 3D space 
   over time and with a given transformation.  
*/


class C3DTrackPoint {
public: 
	C3DTrackPoint(); 

	/**
	   Initialize a new tracking point
	   \param id 
	   \param time 
	   \param pos 
	   \param reserved 
	 */


	C3DTrackPoint(int id, float time, const C3DFVector& pos, const std::string& reserved); 
	
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
	void move(float timestep, const C3DTransformation& t); 


	/// \returns the current position
	const C3DFVector&  get_pos() const; 

	/// \returns the point ID
	int get_id() const;
	
	/// \returns the current time value 
	float get_time() const; 
	
	/// returns the reserved field 
	const std::string&  get_reserved() const; 
private:
	int m_id; 
	float m_time; 
	C3DFVector m_pos; 
	std::string m_reserved; 
}; 

inline std::ostream& operator << (std::ostream& os,  C3DTrackPoint tp) 
{
	tp.print(os); 
	return os; 
}


/**
   Load the trackpoints from an input file 
   \param in_filename input file name in csv format 
   \returns the list of track points 
 */

std::vector< C3DTrackPoint > load_trackpoints(const std::string& in_filename); 

NS_MIA_END

#endif 

