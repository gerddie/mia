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

#include <sstream>
#include <cstring>
#include <climits>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include <boost/filesystem/fstream.hpp>    // ditto
#include <boost/regex.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/filetools.hh>

#ifdef WIN32
#define snprintf _snprintf
#define strdup _strdup
#endif

NS_MIA_BEGIN

using namespace std;
using namespace mia;


namespace bfs = ::boost::filesystem;

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif


CPathNameArray find_files(const CPathNameArray& searchpath, const std::string& pattern)
{
	boost::regex pat_expr(pattern);
	CPathNameArray result; 

	// search through all the path to find the plugins
	for (auto dir = searchpath.begin(); dir != searchpath.end(); ++dir){

		cvdebug() << "Looking for " << dir->string() << "\n"; 

		if (bfs::exists(*dir) && bfs::is_directory(*dir)) {
			// if we cant save the old directory something is terribly wrong
			bfs::directory_iterator di(*dir); 
			bfs::directory_iterator dend;
			
			cvdebug() << "TPluginHandler<I>::initialise: scan '"<<dir->string() <<"'\n"; 

			while (di != dend) {
				cvdebug() << "    candidate:'" << di->path().string() << "'"; 
				if (boost::regex_match(di->path().string(), pat_expr)) {
					result.push_back(*di); 
					cverb << " add\n";
				}else
					cverb << " discard\n";
				++di; 
			}
		}
	}
	return result; 
}


EXPORT_CORE const std::string get_filename_pattern_and_range(std::string const& in_filename, size_t& start_filenum, size_t& end_filenum, size_t& format_width)
{
	string base_name;
	size_t nwidth = format_width = fname_to_cformat(in_filename.c_str(), base_name, false);
	if (nwidth > 0) { // probably more then one slice
		size_t max_num = 1;
		while (nwidth--)
			max_num *= 10;

		start_filenum = end_filenum = 0;
		cvdebug() << "trying files of pattern " << base_name << " n=" << nwidth << "\n";

		// search for the first existing file
		string filename = create_filename(base_name.c_str(), start_filenum);

		while (!bfs::exists(filename) && start_filenum < max_num) {
			++start_filenum;
			filename = create_filename(base_name.c_str(), start_filenum);
		}

		end_filenum = start_filenum;
		while (bfs::exists(filename) &&  end_filenum < max_num) {
			++end_filenum;
			filename = create_filename(base_name.c_str(), end_filenum);
			if (end_filenum == 0)
				break;
		}
	}
	return base_name;
}


vector<string> get_consecutive_numbered_files(string const& in_filename)
{
	vector<string> src_names;
	string base_name;

	// get all possible file names
	size_t nwidth = fname_to_cformat(in_filename.c_str(), base_name, false);
	if (nwidth > 0) { // probably more then one slice
		// check, how many consecutive files we have
		int n_files = 0;
		size_t start_filenum = 0;
		cvdebug() << "trying files of pattern " << base_name << " n="
			  << nwidth << "\n";

		// search for the first existing file
		string filename = create_filename(base_name.c_str(), start_filenum);
		string first_filename = filename;
		size_t max_num = 1;
		while (nwidth--)
			max_num *= 10;

		while (!bfs::exists(filename) && start_filenum < max_num) {
			++start_filenum;
			filename = create_filename(base_name.c_str(), start_filenum);
		}

		while (bfs::exists(filename) &&  start_filenum < max_num) {
			src_names.push_back(filename);
			++start_filenum;
			++n_files;
			filename = create_filename(base_name.c_str(), start_filenum);
			if (filename == src_names[0])
				break;
		}
		cvdebug() << "Found " << n_files << " input files\n";
	}else{
		src_names.push_back(in_filename);
	}
	return src_names;
}

EXPORT_CORE size_t fname_to_cformat(const char *fname, string& base, bool wildcard)
{
	char *help = strdup(fname);

	char *suffix = strrchr(help, '.');

	if (suffix == help) {
		base.assign(fname);
		free(help);
		return 0;
	}


	char *num = suffix ? suffix : help + strlen(help);
	--num;
	size_t nwidth = 0;

	while (isdigit(*num)) 	{
		++nwidth;

		if (num == help)
			break;
		--num;
	}

	if (nwidth == 0) {
		base.assign(fname);
		free(help);
		return 0;
	}

	stringstream result;
	if (num != help) {
		num[1] = '\0'; // cut of the tail
		result << help;
	}
	if (wildcard) {
		for (size_t i = 0; i < nwidth; ++i)
			result << "?";
	}else {
		result << "%0" << nwidth << "d";
	}
	if (suffix)
		result << suffix;
	base = result.str();
	free(help);
	return nwidth;
}


// ugly
EXPORT_CORE string create_filename(const char *cformat, size_t num)
{
	char buf[FILENAME_MAX];
	snprintf(buf, FILENAME_MAX, cformat, num);
	return string(buf);
}

EXPORT_CORE size_t get_filename_number_pattern_width(std::string const& in_filename)
{
	size_t n = 0;
	int  last_dot = in_filename.rfind('.');

	if (last_dot == 0)
		return n;

	string base = in_filename.substr(0,last_dot);

	string::reverse_iterator rit = base.rbegin();

	while (rit < base.rend() && isdigit(*rit)) {
		++n;
		++rit;
	}
	return n;
}

EXPORT_CORE size_t get_filename_number_pattern(std::string const& in_filename, string& base, string& suffix)
{
	bfs::path p(in_filename);
	suffix = p.extension().string();

	string rest = p.stem().string();

	size_t nwidth = 0;
	string::const_reverse_iterator r = rest.rbegin();
	while (isdigit(*r)) {
		++r;
		++nwidth;
	}
	base = rest.substr(0, rest.size() - nwidth);
	return nwidth;
}

EXPORT_CORE void split_filename_number_pattern(std::string const& in_filename, std::string& base,
					       std::string& suffix, std::string& number)
{
	bfs::path p(in_filename);
	size_t nwidth = get_filename_number_pattern(in_filename, base, suffix);
	string stem = p.stem().string();  
	number = stem.substr(stem.size() - nwidth);
}

NS_MIA_END
