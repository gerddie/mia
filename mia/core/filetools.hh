/* -*- mia-c++  -*-
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef MIA_FILETOOLS_HH
#define MIA_FILETOOLS_HH

#include <string>
#include <vector>

#include <mia/core/defines.hh>

NS_MIA_BEGIN


/**
   Based on an input pattern get a vector of all files that follow the same numbering pattern and
   are all numberd consecutive. Only the part of a numbering scheme is taken into account that
   is directly followed by the last "." in the filename.
   \param in_filename a file name pattern, e.g. file0000.png
   \returns a vector of filenames that follow above pattern
*/
EXPORT_CORE std::vector<std::string> get_consecutive_numbered_files(std::string const& in_filename);

/**
   Based on an input pattern get a vector of all files that follow the same numbering pattern and
   are all numberd consecutive. Also obtain the begin and end numbers as well as the width of the
   number pattern
   \param in_filename a file name pattern, e.g. file0000.png
   \param[out] min minimum file number
   \param[out] max maximum file number
   \param[out] format_width with of the numbering part (here 4)
   \returns a vector of filenames that follow above pattern
*/
EXPORT_CORE const std::string get_filename_pattern_and_range(std::string const& in_filename, size_t& min, size_t& max, size_t& format_width);


/**
   Based on an input pattern obtain a vector of file names that follow the same numbering pattern and
   are all numberd consecutive and are within the given range.
   \param in_filename a file name pattern, e.g. file0000.png
   \param start minimum file number
   \param end maximum file number
   \returns a vector of filenames that follow above pattern
*/
EXPORT_CORE std::vector<std::string> get_consecutive_numbered_files_from_pattern(std::string const& in_filename, int start, int end);

/**
   split a filename with a number part into a c-format string or a wildcard string
   \param fname input file name
   \param[out] base the resulting format string
   \param wildcard true to generate a wildcard string, false to generate a c-format string
   \returns the width of the numbering pattern
   example fname = file000.png -> wildcard ? file???.png : file%03d.png
*/
EXPORT_CORE size_t fname_to_cformat(const char *fname, std::string& base, bool wildcard);

/**
   split a file name into directoyr and file
   \remark obsolate - use BOOST functions
*/

EXPORT_CORE void split_dir_fname(const char *in_name, std::string& dir, std::string& sname);


/**
   create a file name from a pattern and a number
   \param cformat format pattern string
   \param num number
   \returns an according string
*/
EXPORT_CORE std::string create_filename(const char *cformat, size_t num);

/**
   \param in_filename some filename of the pattern "bla/bla/nameXXXXX.bla" where X are digits
   \returns the number of digits in the filename
 */

EXPORT_CORE size_t get_filename_number_pattern_width(std::string const& in_filename);

/**
   Split the file name numbering pattern for names (e.g. "lala0000.bla") and get the number 
     of digits 
   \param in_filename the input file name 
   \param[out} base the base of the filename (would be "lala") 
   \param[out] suffix the file suffix (would be ".bla") 
   \returns the number of digits (would be 4) 
 */
EXPORT_CORE size_t get_filename_number_pattern(std::string const& in_filename, std::string& base, std::string& suffix);

/**
   Split the file name numbering pattern for names (e.g. "lala00210.bla") 
   \param in_filename the input file name 
   \param[out} base the base of the filename (would be "lala") 
   \param[out] suffix the file suffix (would be ".bla") 
   \param[out] number the file suffix (would be "00210") 
 */

EXPORT_CORE void split_filename_number_pattern(std::string const& in_filename, std::string& base,
					       std::string& suffix, std::string& number);

NS_MIA_END

#endif
