/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: miaHistory.cc 937 2006-07-11 11:56:52Z write1 $


#include <ctime>
#include <sstream>

#include <config.h>

#include <mia/core/history.hh>
#include <mia/core/cmdlineparser.hh>


using namespace std;

NS_MIA_BEGIN

char const *get_revision();

void CHistory::append(const string& me, const string& version, const CCmdOptionList& options)
{
	CHistoryRecord record = options.get_values();
	record["+PROGRAM"] = me;
	record["+VERSION"] = version;
	record["+USER"] = string(getenv("USER"));
	record["+LIBMIA_VERSION"] = PACKAGE_VERSION;
	record["+LIBMIA_REVISION"] = get_revision();
	time_t t = time(NULL);
	char *time_str = ctime(&t);
	char *help = time_str;

	while (*help) {
		if (*help == ':')
			*help = '.';
		if (*help == ' ')
			*help = '_';
		if (*help == '\n')
			*help = '_';
		++help;
	}

	if (!options.get_remaining().empty()) {
		stringstream extra_str;
		for (vector<const char *>::const_iterator i = options.get_remaining().begin();
		     i != options.get_remaining().end(); ++i)
			extra_str << *i << " ";
		record["EXTRA"] = extra_str.str();
	}

	push_back(CHistoryEntry(time_str, record));
}


string CHistory::as_string()const
{
	CHistory::const_iterator e = end();
	stringstream result;

	for (CHistory::const_iterator i = begin(); i != e; ++i){
		result << i->first << ":";
		CHistoryRecord::const_iterator ke = i->second.end();
		for (CHistoryRecord::const_iterator k = i->second.begin(); k != ke; ++k) {
			result <<"  "<< k->first << " : " << k->second << endl;
		}
	}
	return result.str();
}

CHistory::CHistory()
{
}

CHistory& CHistory::instance()
{
	static CHistory object;
	return object;
}

NS_MIA_END
