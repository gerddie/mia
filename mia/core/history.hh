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

#ifndef HISTORY_HH
#define HISTORY_HH

#include <list>
#include <string>
#include <map>
#include <vector>

#include <mia/core/cmdlineparser.hh>

NS_MIA_BEGIN


/**
   \ingroup basic
   \brief This class holds the complete history list of a data set.

   \todo Use it.
*/


class EXPORT_CORE CHistory: public std::list<CHistoryEntry>
{
public:
       /** append a new history record
           \param program name of the calling program
           \param version version of the program run
           \param options the options used in this call
       */
       void append(const std::string& program, const std::string& version, const CCmdOptionList& options);

       /**
           \returns the complete history as a string.
       */
       std::string as_string()const;

       /** returns the reference to the singelton history object */
       static CHistory& instance();
private:
       CHistory();
};
NS_MIA_END

#endif
