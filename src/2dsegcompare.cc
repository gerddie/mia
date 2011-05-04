
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
	"This program is used to evaluate the Hausdorff distance between each frame "
	"of a perfusion time series and a reference frame.";

int do_main(int argc, const char *argv[])
{
	string src_filename;
	string ref_filename;

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref-file", 'r', "reference frame", CCmdOption::required));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset = load_segmentation(src_filename);
	CSegSet ref_segset = load_segmentation(ref_filename);

	const CSegSet::Frames& src_frames = src_segset.get_frames();
	const CSegSet::Frames& ref_frames = ref_segset.get_frames();

	if (ref_frames.size() != src_frames.size())
		throw invalid_argument("segmentations have different frame numbers");


	CSegSet::Frames::const_iterator iframe = src_frames.begin();
	CSegSet::Frames::const_iterator rframe = ref_frames.begin();
	CSegSet::Frames::const_iterator eframe = src_frames.end();

	while (iframe != eframe) {
		const CSegFrame::Sections& src_sections = iframe->get_sections();
		const CSegFrame::Sections& ref_sections = rframe->get_sections();

		cout << iframe->get_hausdorff_distance(*rframe) << " ";
		for (size_t j = 0; j < src_sections.size(); ++j)
			cout << src_sections[j].get_hausdorff_distance(ref_sections[j]) << " ";
		cout << "\n";

		++iframe;
		++rframe;
	}
	return 0;

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


