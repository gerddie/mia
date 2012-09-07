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

#define VSTREAM_DOMAIN "3dtrack-pixelmovement"

#include <fstream>
#include <ostream>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/internal/main.hh>
#include <mia/3d/transformio.hh>


using namespace std;
using namespace mia;

const SProgramDescription g_description = {
        {pdi_group, "Tools for the Analysis of 3D image series"}, 
	{pdi_short, "Track positions in 3D"}, 
	{pdi_description, "Track the position of a pixel by means of a transformation obtained "
	 "by using image registration. The pixel data is given in CSV format\n"
	 "   id;time;x;y;z;reserved\n\n"
	 "The fields 'time', 'x', 'y', and 'z' are updated, and the fields 'id' and 'reserved' are preserved, "
	 "empty lines are ignored."}, 
	{pdi_example_descr, "This program evaluated the changed position of pixels given in input.csv, by using  a "
	"time step of 2.0 time units and the transformation change.v3df, and writes the updated "
	"positions to updated.csv."}, 
	{pdi_example_code, "-i input.csv -o output.cvs --time-step 2.0 --transformation change.v3df"}
}; 

/**
   \brief Class to track pixel movement based on a transformation

   This class holds the information to track a single pixel in 3D space 
   over time and with a given transformation.  
*/


class C3DTrackPoint {
public: 
	C3DTrackPoint(); 
	
	/**
	   Read the point from an input line. The point must be of format 
	   id;time;x;y;z[;reserved]
	   \param is input stream to read the point from
	   \returns true if the track point was read successfully, false otherwise 
	*/
	bool read(istream& is); 
	
	/**
	   print the point to an output stream 
	   \param os output stream 
	 */
	void print(ostream& os) const; 

	/**
	   Mova the pixel according tho a time step and based on the given transformation
	   \param timestep the time step to apply
	   \param t the transformation to apply 
	 */
	void move(float timestep, const C3DTransformation& t); 
private:
	int m_id; 
	float m_time; 
	C3DFVector m_pos; 
	string m_reserved; 
}; 

C3DTrackPoint::C3DTrackPoint():m_id(-1), 
	m_time(0.0)
{
}

bool C3DTrackPoint::read(istream& is)
{
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
	
	is >> m_reserved;
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

ostream& operator << (ostream& os,  C3DTrackPoint tp) 
{
	tp.print(os); 
	return os; 
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
		throw create_exception<runtime_error>( "Unable to open file '", in_filename, "' for reading");

	while (input.good()) {
		string input_line; 
		getline(input, input_line);
		if (input_line.empty()) 
			continue; 
		
		if (input.good()) {
			C3DTrackPoint pt; 
			istringstream is(input_line);
			if (pt.read(is)) 
				result.push_back(pt); 
			else 
				cverr() << "Bogus input line '" << input_line << "' ignored\n";
		}
	}
	return result; 
}

int do_main( int argc, char *argv[] )
{
	// Parameters 
	string in_filename;
	string out_filename;
	string trans_filename; 
	float time_step = 1.0; 
	    

	// Option setup 
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input point set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output point set", CCmdOption::required)); 

	options.add(make_opt( trans_filename, "transformation", 't', 
				    "transformation describing the monitored change", CCmdOption::required)); 

	options.set_group("\nParameters"); 
	options.add(make_opt( time_step, "time-step", 'T', 
			      "time step to use for the position update")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read the transformation 
	auto t = C3DTransformationIOPluginHandler::instance().load(trans_filename);

	// read the trackpoints 
	vector< C3DTrackPoint > trackpoints = load_trackpoints(in_filename); 

	// transform the trackpoints 
	for_each(trackpoints.begin(), trackpoints.end(), 
		 [&time_step, &t](C3DTrackPoint& tp) { tp.move(time_step, *t);}); 
	

	// write the trackpoints 
	ofstream output(out_filename.c_str()); 
	if (!output.good()) 
		throw create_exception<runtime_error>( "Unable to open file '", out_filename, "' for writing "); 

	for_each(trackpoints.begin(), trackpoints.end(), 
		 [&output](const C3DTrackPoint& tp) { output << tp << "\n";}); 

	if (!output.good()) 
		throw create_exception<runtime_error>( "Error writing to '", out_filename, "'"); 

	
	return EXIT_SUCCESS;	

}

MIA_MAIN(do_main);
	

