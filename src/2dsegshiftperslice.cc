
/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

static string get_number(const string& fname)
{
	bfs::path f(fname);
	string the_stem = f.stem();
	auto rs = the_stem.rbegin();
	string result;
	while (rs != the_stem.rend() && isdigit(*rs))
		result.insert(0,1,*rs++);
	return result;
}

const char *g_description = 
	"This program is used to shift the 2d segmentation of a segmentation set individually "
	"on a per slice base." 
	;

int do_main(int argc, const char *args[])
{
	string src_filename;
	string out_filename;
	string shift_filename("crop");

	string shift_value_filebase("shift");

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in-file", 'i', "input segmentation set", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output segmentation set", "out", true));
	options.push_back(make_opt( shift_filename, "image-file", 'g', "output image filename base", "image", false));

	options.push_back(make_opt( shift_value_filebase, "shift", 'S', "shift of segmentation - base name ",
				    "shift", true));


	options.parse(argc, args);

	CSegSet src_segset = load_segmentation(src_filename);
	CSegSet::Frames& frames = src_segset.get_frames();

	for (auto i = frames.begin(); i != frames.end(); ++i) {
		string nr = get_number(i->get_imagename());
		stringstream shift_file_name;
		shift_file_name << shift_value_filebase << nr << ".txt";
		ifstream shift_file(shift_file_name.str());
		C2DFVector shift;
		shift_file >> shift;
		i->shift(shift, i->get_imagename());
	}


	auto_ptr<xmlpp::Document> outset(src_segset.write());

	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();

	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
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


