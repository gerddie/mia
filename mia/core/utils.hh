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

// $Id: miaUtils.hh 887 2006-03-01 12:22:14Z write1 $

/*! \brief Some easy cache, string, and file manipulation tools

\file miaUtils.hh
\author G. Wollny, wollny@cbs.mpg.de, 2004
\author M. Tittgemeyer, tittge@cbs.mpg.de, 2004

*/

#ifndef __MIA_TOOLS_HH
#define __MIA_TOOLS_HH 1

#include <list>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

#include <mia/core/defines.hh>
NS_MIA_BEGIN

/**
   a helper class that stores the current working directory on construction
   and goes back to the it when it is destroyed
*/
class EXPORT_CORE CCWDSaver {
	char *cwd;
public:
	CCWDSaver();
	~CCWDSaver();
};

#ifndef WIN32
/*! a functor to search  for files */
class  FSearchFiles  {
	std::list<std::string>& result;
	const std::string pattern;
public:
	/** constructor of functor
	    \param __result takes an (empty) list to which the found files will be stored
	    \param __pattern holds the search pattern
	*/
	FSearchFiles(std::list<std::string>& __result, const std::string& __pattern);

	/** search the given \a path with the stored search pattern and add the found files to result */
	void operator()(const std::string& path);
};
#endif

NS_MIA_END

#endif
