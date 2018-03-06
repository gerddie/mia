/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
       const char *user = getenv("USER");
       record["+USER"] = string(user ? user : "unknown");
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

              for (auto i = options.get_remaining().begin();   i != options.get_remaining().end(); ++i)
                     extra_str << *i << " ";

              record["EXTRA"] = extra_str.str();
       }

       push_back(CHistoryEntry(time_str, record));
}


string CHistory::as_string()const
{
       CHistory::const_iterator e = end();
       stringstream result;

       for (CHistory::const_iterator i = begin(); i != e; ++i) {
              result << i->first << ":";
              CHistoryRecord::const_iterator ke = i->second.end();

              for (CHistoryRecord::const_iterator k = i->second.begin(); k != ke; ++k) {
                     result << "  " << k->first << " : " << k->second << endl;
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
