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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <queue>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <mia/core/labelmap.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

NS_MIA_USE; 
using namespace std; 

const SProgramDescription g_description = {
	{pdi_group, "Miscellaneous programs"}, 
	{pdi_short, "Create sorted histogram mapping"}, 
	{pdi_description, 	"This program is used create a mapping based on a histogram that "
	 "puts the intensities with high values at the beginning of the histogram. The main "
	 "use case will be to sort labels of connected components in out-of-core image processing."}, 
}; 

typedef pair<unsigned short, size_t> CEntry; 

struct FEntryCompare {
	bool operator () (const CEntry& a, const CEntry& b) const {
		return a.second < b.second; 
		
	}
}; 


int do_main(int argc, char *argv[]) 
{
	string in_filename; 
	string out_filename; 
	CCmdOptionList options(g_description);

	options.add(make_opt( in_filename, "in-file", 'i', "input file name containing the histogram", 
			      CCmdOptionFlags::required_input)); 
	options.add(make_opt( out_filename, "out-file", 'o', "output file name to store probabilities", 
			      CCmdOptionFlags::required_output)); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	priority_queue<CEntry, vector<CEntry>, FEntryCompare> hist; 
	
	ifstream ifs( in_filename); 
	
	CEntry entry; 
	while (ifs.good()) {
		ifs >> entry.first >> entry.second; 
		if (ifs.good())
			hist.push(entry); 
	} ; 
	
	ifs.close(); 
		

	cvmsg() << "got a (label) histogram with " << hist.size() << " values\n";
		
	CLabelMap result; 

	unsigned short pos = 0; 		
	while (!hist.empty()) {
		CEntry e = hist.top(); 
		hist.pop(); 
		if (e.second > 0) {
			result[e.first] = pos++;
		}
	}

	
	unique_ptr<ostream> os(out_filename == string("-") ? &cout : new ofstream(out_filename)); 
	result.save(*os); 
	return os->good() ? EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
