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

#ifndef mia_core_test_helpers_hh
#define mia_core_test_helpers_hh

#ifndef mia_internal_autotest_hh
#include <mia/internal/autotest.hh>
#endif

#include <set>
#include <string>

/**
   \ingroup  test
   Test help template to check whether a plug-in handler loads a certain set of expected
   plug-ins. The function uses boost::unittest to handele the testing.

   \tparam the plug-in handler type to be tested
   \param expected the set of expected plug-ins given by their name
*/
template <class PluginHandler>
void test_plugin_names_and_count(const std::set<std::string>& expected)
{
       auto plugins = PluginHandler::instance().get_set();
       BOOST_CHECK_EQUAL(plugins.size(), expected.size());

       for (auto p = plugins.begin(); p != plugins.end(); ++p) {
              BOOST_CHECK_MESSAGE(expected.find(*p) != expected.end(), "unexpected plugin '" << *p << "' found");
       }

       for (auto p = expected.begin(); p != expected.end(); ++p)
              BOOST_CHECK_MESSAGE(plugins.find(*p) != plugins.end(), "expected plugin '" << *p << "' not found");
}


#endif
