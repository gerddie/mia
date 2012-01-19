/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef mia_core_test_helpers_hh
#define mia_core_test_helpers_hh

#ifndef mia_internal_autotest_hh
#include <mia/internal/autotest.hh>
#endif 

#include <set>
#include <string>

/**
   Test help template to check whether a plug-in handler loads a certain set of expected 
   plug-ins. The function uses boost::unittest to handele the testing. 

   \tparam the plug-in handler type to be tested 
   \param expected the set of expected plug-ins given by their name 
*/
template <PluginHandler> 
void test_plugin_names_and_count(const std::set<std::string>& expected) const
{
	auto plugins = PluginHandler::instance().get_set(); 
	BOOST_CHECK_EQUAL(plugins.size(), test_data.size()); 
	
	for (auto p = plugins.begin(); p != plugins.end(); ++p) {
		BOOST_CHECK_MESSAGE(test_data.find(*p) != test_data.end(), "unexpected plugin '" << *p << "' found"); 
	}
	
	for (auto p = test_data.begin(); p != test_data.end(); ++p)
		BOOST_CHECK_MESSAGE(plugins.find(*p) != plugins.end(), "expected plugin '" << *p << "' not found"); 

}


#endif
