/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <iterator>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <list>
#include <cassert>

#include <mia/core.hh>
#include <mia/2d/SegSet.hh>

#include <libxml++/libxml++.h>


using namespace std; 
using namespace mia; 
using xmlpp::DomParser; 


int do_main(int argc, const char *argv[])
{
	string src_filename; 
	int  enlarge_boundary = 5; 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", "input", true));
	options.add(make_opt( enlarge_boundary, "enlarge", 'e', "enlarge boundary by number of pixels", "enlarge"));

	
	options.parse(argc, argv);
		
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(src_filename);
	
	if (!parser)
		throw runtime_error(string("Unable to parse input file:") + src_filename); 
	
	CSegSet segset(*parser.get_document()); 
	const CSegSet::Frames& frames = segset.get_frames(); 

	if (ref_image >= frames.size()) {
		THROW(invalid_argument, "Request reference " << ref_image << " but input series has only " << 
		      frames.size() << " images"); 
	}
	
	SliceSegmentation ref_frame = get_frame_as_poly(frames[ ref_image ]);
	
	for (CSegSet::Frames::const_iterator i = frames.begin(); i != frames.end(); ++i) {
		SliceSegmentation frame = get_frame_as_poly(*i); 
		
		print_stats(frame, ref_frame); 
		
	}
	return EXIT_SUCCESS; 
}

int main(int argc, const char *argv[] )
{
	try {
		return do_main(argc, argv); 
		
		
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}
	return EXIT_FAILURE;
}

