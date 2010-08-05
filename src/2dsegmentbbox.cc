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


int do_main(int argc, const char *args[])
{
	string src_filename; 
	int  enlarge_boundary = 5; 

	CCmdOptionList options;
	options.push_back(make_opt( src_filename, "in-file", 'i', "input segmentation set", "input", true));
	options.push_back(make_opt( enlarge_boundary, "enlarge", 'e', "enlarge boundary by number of pixels", "enlarge"));

	
	options.parse(argc, args);
		
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

int main(int argc, const char *args[] )
{
	try {
		return do_main(argc, args); 
		
		
	}
	catch (const runtime_error &e){
		cerr << args[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << args[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << args[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << args[0] << " unknown exception" << endl;
	}
	return EXIT_FAILURE;
}

