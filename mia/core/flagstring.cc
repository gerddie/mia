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

#define VSTREAM_DOMAIN "FlagString"

#include <algorithm>
#include <mia/core/flagstring.hh>

NS_MIA_BEGIN
using namespace std;

CFlagString::CFlagString(const Table table[])
{
	const Table *t = table;
	while (t->id != 0) {
		m_map[t->id] = t->flag;
		m_backmap[t->flag] = t->id;
		++t;
	}
}

int CFlagString::get(const string& flags)const
{
	int result = 0;
	for (auto i = flags.begin(); i != flags.end(); ++i) {
		auto f = m_map.find(*i);
		if (f == m_map.end())
			throw create_exception<invalid_argument>("CFlagString::get: flag '", *i, "' unknown");
		result |= f->second;
	}
	return result;
}

const string CFlagString::get(int flags)const
{
	string result;

	// first try to find the flag directly
	auto f = m_backmap.find(flags);
	if (f != m_backmap.end()) {
		result.push_back(f->second);
		return result;
	}
	// combined flag
	int acc = 0;
	for(auto f = m_backmap.begin(); flags != acc && f != m_backmap.end(); ++f) {
		if ((flags & f->first) == f->first) {
			result.push_back(f->second);
			acc |= f->first;
		}
	}
	return result;
}

const string CFlagString::get_flagnames()const
{
	string result;
	for(auto f = m_map.begin(); f != m_map.end(); ++f)
		result.push_back(f->first);
	sort(result.begin(), result.end());
	return result;
}

NS_MIA_END
