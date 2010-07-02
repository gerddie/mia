/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef ph_optionparser_hh
#define ph_optionparser_hh

#include <list>
#include <map>
#include <string>

#include <mia/core/defines.hh>

NS_MIA_BEGIN


/// a map of parsed options as pairs key:value pairs
typedef std::map<std::string, std::string> CParsedOptions;


/** \class CComplexOptionParser
    \brief Parser for complex command line options
        This is a little parser to parse more complex command line options.
	The syntax of the command line is
	"ssd:src=img1.v,ref=img2.v+lm:src=lm1.lmx,ref=lm2.lmx,f=[tsp:r=3]"
	with one level of brackets allowed

	Above options string will be split off like:
	< ssd, { < src, img1.v  >, < ref, img2.v  > } >
	<   lm, ( < src, lm1.lmx >, < ref, lm2.lmx >, < f, tsp:r=3 > } >
*/

class EXPORT_CORE CComplexOptionParser {

public:

	typedef char SSeperators[];
	typedef std::multimap<std::string, CParsedOptions> CParts;
	typedef CParts::const_iterator const_iterator;

	/**
	   \name constructor
	   Parse the input string and split it in a nested list of name-value pairs
	   \param param the option string
	*/
	CComplexOptionParser(const std::string& param);

	///\returns the begin iterator to the list of option parts that are (normally) delimited by '+'
	const_iterator begin()const;

	///\returns the end iterator to the list of option parts that are (normally) delimited by '+'
	const_iterator end() const;

	/// \returns the number of the parts in the option string parsed
	CParts::size_type size()const;

	/** replace the standart set of seperators
	    \param sep the new seperators
	*/
	static void set_seperators(SSeperators sep);

private:

	int split(const std::string& s, char c, std::list<std::string>& result)const;
	CParsedOptions::value_type split_pair_b(const std::string& s, char c)const;
	CParsedOptions::value_type split_pair(const std::string& s, char c)const;

	CParts _M_Parts;

	static char _M_parts_seperator;
	static char _M_part_options;
	static char _M_option_seperator;
	static char _M_assignment_seperator;

};

NS_MIA_END

#endif
