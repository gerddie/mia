/*
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <iomanip>
#include <string>


#include <mia/core/filetools.hh>
#include <mia/core/cmdlineparser.hh>

using namespace std;
using namespace mia;

const char *g_description = 
	"This program is used to extract a extract the number pattern for \n"
	"file012312.ext type file names\n"
	"Basic usage:\n"
	"  mia-filenumberpattern [options] \n";

int main( int argc, const char *argv[] )
{
	string in_filename;
	CCmdOptionList options(g_description);
	options.push_back(make_opt( in_filename, "in-file", 'i', "input image example name", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	size_t format_width = get_filename_number_pattern_width(in_filename);
	cout << setw(format_width) << setfill('0') << 0;
	return 0;
}

