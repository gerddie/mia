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

#include <fstream>
#include <ostream>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/internal/main.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/trackpoint.hh>


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
				    "input point set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output point set", CCmdOptionFlags::required_output)); 

	options.add(make_opt( trans_filename, "transformation", 't', 
			      "transformation describing the monitored change", 
			      CCmdOptionFlags::required_input, &C3DTransformationIOPluginHandler::instance())); 

        options.set_group("\nParameters"); 
        options.add(make_opt( time_step, "time-step", 'T', 
			      "time step to use for the position update")); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read the transformation 
	auto t = C3DTransformationIOPluginHandler::instance().load(trans_filename);

	// read the trackpoints 
	vector< C3DTrackPoint > trackpoints = C3DTrackPoint::load_trackpoints(in_filename); 

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
	

