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

#include <mia/core/cmdlineparser.hh>
NS_MIA_USE

int main(int argc, const char **args)
{
	if (argc > 1) {
		CCmdOptionList olist; 
		float value = 10; 
		std::string s("fun"); 
		olist.push_back(make_opt(value, "float",'f',  "a really long help string to test the line break of the output", "float")); 
		olist.push_back(make_opt(s, "string",'s',  "a string option", "string")); 

		olist.parse(argc, args); 
	}
	return 0; 
}
