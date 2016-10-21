/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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


#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/fixedwidthoutput.hh>

NS_MIA_USE
using namespace std;


BOOST_AUTO_TEST_CASE ( test_fixedwidth_writing ) 
{
	stringstream out; 

	CFixedWidthOutput ts(out, 50); 

	ts.write("This is the first line"); 
	ts.push_offset(5); 
	ts.newline(); 
	ts.write("Some more text that is really long and doesn't fit on a line");
	ts.push_offset(5); 
	ts.newline(); 
	ts.write("yet some more text that is really long and doesn't fit on a line\n");
	ts.write("123456789 123456789 123456789 123456789 Let's see.");
	ts.pop_offset();
	ts.write("\nAnd another line that shouldn't fit on a line since it is really long");
	ts.pop_offset();
	ts.write("\nThe final line\n");

	const string test_text("This is the first line\n"
			       "     Some more text that is really long and \n"
			       "     doesn't fit on a line\n"
			       "          yet some more text that is really long \n"
			       "          and doesn't fit on a line\n"
			       "          123456789 123456789 123456789 123456789\n"
			       "          Let's see.\n"
			       "     And another line that shouldn't fit on a \n"
			       "     line since it is really long\n" 
			       "The final line\n"); 

	cvdebug()  << out.str(); 
	cvdebug()  << test_text; 
	
	BOOST_CHECK_EQUAL(out.str(), test_text); 

}
