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

#include <stdexcept>
#include <cassert>
#include <cstring>

#include <mia/core/optionparser.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN
using namespace std;

char CComplexOptionParser::_M_parts_seperator      = '+';
char CComplexOptionParser::_M_part_options         = ':';
char CComplexOptionParser::_M_option_seperator     = ',';
char CComplexOptionParser::_M_assignment_seperator = '=';


CComplexOptionParser::CComplexOptionParser(const string& param)
{
	cvdebug() << "CComplexOptionParser: constr '" << param << "'\n";
	typedef list<string> string_list;
	string_list parts;
	if (split(param, _M_parts_seperator, parts)== 0)
		return;

	string_list::const_iterator i = parts.begin();
	string_list::const_iterator e = parts.end();

	while (i != e) {
		pair<string, string>  part = split_pair(*i,_M_part_options);
		CParsedOptions options;

		cvdebug() << "CComplexOptionParser: '" << part.first << "' : '" << part.second << "' len="<< part.second.size() << "\n";

		string help = part.second;

		while (!help.empty()) {
			cvdebug() << "split '"<< help << "'\n";
			pair<string, string> next_option = split_pair(help, _M_assignment_seperator);

			if (next_option.second.empty()) {
				help = next_option.second;
			}else {
				pair<string, string> tail = split_pair_b(next_option.second, _M_option_seperator);
				next_option.second = tail.first;
				help = tail.second;
			}
			cvdebug() << "got option: '"<< next_option.first << "=[" << next_option.second << "]'\n";
			options.insert(next_option);
		}
		cvdebug() << "CComplexOptionParser: add '" << part.first << "'\n";
		_M_Parts.insert(CParts::value_type(part.first, options));

		++i;
	}
	cvdebug() << "CComplexOptionParser: constr - got " << _M_Parts.size() << " option(s)\n";
}

CComplexOptionParser::const_iterator
CComplexOptionParser::begin()const
{
	return _M_Parts.begin();
}



CComplexOptionParser::const_iterator
CComplexOptionParser::end()const
{
	return _M_Parts.end();
}


CComplexOptionParser::CParts::size_type CComplexOptionParser::size() const
{
	return _M_Parts.size();
}


void CComplexOptionParser::set_seperators(SSeperators s)
{
	assert(strlen(s) >= 4);
	_M_parts_seperator      = s[0];
	_M_part_options         = s[1];
	_M_option_seperator     = s[2];
	_M_assignment_seperator = s[3];
}

int CComplexOptionParser::split(const string& s, char c, list<string>& result)const
{
	string::size_type i = 0;

	while (i < s.size()) {
		string::size_type start = i;
		i = s.find(c, start);

		if (i < s.size()) {
			result.push_back(s.substr(start,i-start));
			++i;
		}else {
			result.push_back(s.substr(start, s.size() - start));
		}
	}
	return result.size();
}

CParsedOptions::value_type
CComplexOptionParser::split_pair(const string& s, char c)const
{
	size_t i = s.find(c);

	if (i < s.size()) {
		return CParsedOptions::value_type(s.substr(0,i), s.substr(i+1,s.size()-i-1));
	}else
		return CParsedOptions::value_type(s,string(""));
}

CParsedOptions::value_type
CComplexOptionParser::split_pair_b(const string& s, char c)const
{
	bool has_protector = (s[0] == '[');

	if (has_protector) {

		size_t i = s.find(']');
		if (i >= s.size())
			throw runtime_error(string("Parsing option ") + s + string(" failed"));

		if (i < s.size() - 1) {
			return CParsedOptions::value_type(s.substr(1,i-1), s.substr(i+2,s.size()-i-1));
		}else
			return CParsedOptions::value_type(s.substr(1,i-1), string(""));
	}else {
		size_t i = s.find(c);

		if (i < s.size()) {
			return CParsedOptions::value_type(s.substr(0,i), s.substr(i+1,s.size()-i-1));
		}else
			return CParsedOptions::value_type(s,string(""));
	}
}


NS_MIA_END
