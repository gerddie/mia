/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <iostream>
#include <iomanip>
#include <string>


#include <mia/internal/main.hh>
#include <mia/core/filetools.hh>
#include <mia/core/cmdlineparser.hh>

using namespace std;
using namespace mia;



const SProgramDescription g_description = {
	{pdi_group, "Miscellaneous programs"}, 
	{pdi_short, "File name number pattern analysis"}, 
	{pdi_description, "This program is used to extract a extract the number pattern for "
	 "file012312.ext type file names."}
}; 

int do_main( int argc, char *argv[] )
{
	string in_filename;
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image example name", 
			      CCmdOptionFlags::required_input));
	options.set_stdout_is_result(); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	size_t format_width = get_filename_number_pattern_width(in_filename);
	cout << setw(format_width) << setfill('0') << 0;

	// end of program, so it's not important to restore the stream state
	// coverity[STREAM_FORMAT_STATE] 
	return 0;
}

MIA_MAIN(do_main); 
