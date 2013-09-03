/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_flagstring_hh
#define mia_core_flagstring_hh

#include <map>

#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

class  EXPORT_CORE CFlagString {
public:
	typedef struct {
		char id;
		int  flag;
	} Table;

	CFlagString(const Table table[]);

	int get(const std::string& flags)const;

	const std::string get(int flags)const;

	const std::string get_flagnames()const;

private:
	typedef std::map<char, int> TMap;
	typedef std::map<int, char> TBackMap;

	TMap m_map;
	TBackMap m_backmap;

};

NS_MIA_END
#endif
