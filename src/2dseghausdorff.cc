
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

const char *g_description = 
	"Get the per-slice Hausdorff distance of a a segmentation with respectto a given reference frame."; 

int do_main(int argc, const char *args[])
{
	string src_filename;
	size_t reference = 0;

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.push_back(make_opt( reference, "ref-frame", 'r', "reference frame", CCmdOption::required));
	options.parse(argc, args);

	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(src_filename);

	if (!parser)
		throw runtime_error(string("Unable to parse input file:") + src_filename);

	CSegSet segset(*parser.get_document());

	const CSegSet::Frames& frames = segset.get_frames();

	if (reference >= frames.size())
		throw invalid_argument("Reference frame outside range");

	const CSegFrame& ref = segset.get_frames()[reference];

	CSegSet::Frames::const_iterator iframe = segset.get_frames().begin();
	CSegSet::Frames::const_iterator eframe = segset.get_frames().end();

	size_t i = 0;
	while (iframe != eframe) {
		if (i != reference) {
			cout << ref.get_hausdorff_distance(*iframe) << "\n";
		}else{
			cout << "0\n";
		}
		++i;
		++iframe;
	}
	return 0;

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


