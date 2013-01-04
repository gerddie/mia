/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <stdexcept>
#include <cassert>
#include <cstring>

#include <mia/core/optionparser.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN
using namespace std;

char CComplexOptionParser::m_parts_seperator      = '+';
char CComplexOptionParser::m_part_options         = ':';
char CComplexOptionParser::m_option_seperator     = ',';
char CComplexOptionParser::m_assignment_seperator = '=';


CComplexOptionParser::CComplexOptionParser(const string& param)
{
	cvdebug() << "CComplexOptionParser: constr '" << param << "'\n";
	typedef list<string> string_list;
	string_list parts;
	if (split(param, m_parts_seperator, parts)== 0)
		return;

	auto i = parts.begin();
	auto e = parts.end();

	while (i != e) {
		auto  part = split_pair(*i,m_part_options);
		CParsedOptions options;

		cvdebug() << "CComplexOptionParser: '" << part.first << "' : '" << part.second 
			  << "' len="<< part.second.size() << "\n";

		string help = part.second;

		while (!help.empty()) {
			cvdebug() << "split '"<< help << "'\n";
			auto next_option = split_pair(help, m_assignment_seperator);

			if (next_option.second.empty()) {
				help = next_option.second;
			}else {
				auto tail = split_pair_b(next_option.second, m_option_seperator);
				next_option.second = tail.first;
				help = tail.second;
			}
			cvdebug() << "got option: '"<< next_option.first << "=[" << next_option.second << "]'\n";
			options.insert(next_option);
		}
		cvdebug() << "CComplexOptionParser: add '" << part.first << "'\n";
		m_Parts.insert(CParts::value_type(part.first, options));

		++i;
	}
	cvdebug() << "CComplexOptionParser: constr - got " << m_Parts.size() << " option(s)\n";
}

CComplexOptionParser::const_iterator
CComplexOptionParser::begin()const
{
	return m_Parts.begin();
}



CComplexOptionParser::const_iterator
CComplexOptionParser::end()const
{
	return m_Parts.end();
}


CComplexOptionParser::CParts::size_type CComplexOptionParser::size() const
{
	return m_Parts.size();
}


void CComplexOptionParser::set_seperators(SSeperators s)
{
	assert(strlen(s) >= 4);
	m_parts_seperator      = s[0];
	m_part_options         = s[1];
	m_option_seperator     = s[2];
	m_assignment_seperator = s[3];
}

int CComplexOptionParser::split(const string& s, char c, list<string>& result)const
{
	string::size_type i = 0;
	
	
	while (i < s.size()) {
		string::size_type start = i;
		int level = 0; 
		while (i < s.size() && ( s[i] != c || level > 0 )) {
			if (s[i] == '[') 
				++level; 
			if (s[i] == ']') {
				if (level > 0)
					--level; 
				else
					throw invalid_argument("CComplexOptionParser: encountered ']' without leading '['"); 
			}
			++i; 
		}
		if (level > 0) 
			throw invalid_argument("CComplexOptionParser: miss closing ']' ");
		
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
		int pdepth = 0; 
		size_t i = 1; 
		while ( ( pdepth || (s[i] != ']')) && (i < s.size())) {
			if (s[i] == '[') 
				++pdepth; 
			else if (s[i] == ']') 
				--pdepth;
			++i; 
		}
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
