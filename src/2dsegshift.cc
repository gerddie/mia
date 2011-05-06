/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// TODO: segment set loading should use the relative path to the segment.set if there is
//       no absolute path in the file

#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cmath>
#include <list>
#include <cassert>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>

#include <mia/core.hh>
#include <mia/2d/SegSet.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>





using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;


CSegSet load_segmentation(const string& s)
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(s);
	return CSegSet(*parser.get_document());
}

const char *g_description = 
	"This program is used to remove images from the beginning of a segmentation set and at the "
	"same time move the segmentation by a given offset." 
	;

int do_main(int argc, const char *argv[])
{
	string src_filename;
	string out_filename;
	string shift_filename("crop");
	size_t skip = 2;

	C2DFVector shift;

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "input segmentation set", CCmdOption::required));
	options.push_back(make_opt( shift_filename, "image-file", 'g', "output image filename base"));

	options.push_back(make_opt(shift, "shift", 'S', "shift of segmentation"));
	options.push_back(make_opt(skip, "skip", 's', "skip frames at the begining"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset = load_segmentation(src_filename);

	cvinfo() << "shift by " << shift << ", skip " << skip << " and rename to base " << shift_filename << "\n";
	CSegSet shifted = src_segset.shift_and_rename(skip, shift, shift_filename);

	auto_ptr<xmlpp::Document> outset(shifted.write());

	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();

	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
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


