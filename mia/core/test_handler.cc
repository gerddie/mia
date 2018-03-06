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

#define BOOST_TEST_DYN_LINK

#include <iostream>
#include <climits>

#include <mia/internal/autotest.hh>

#include <mia/core/handler.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/testplugin.hh>
#include <boost/filesystem/path.hpp>

NS_MIA_USE
using namespace std;
namespace bfs = ::boost::filesystem;


/*
   It is not quite clean whether BOOST test is thread save, therefore
   the threaded function only counts the fails and uses MIA to report,
   and then in the serial code part the number of fails is tested via BOOST
*/
BOOST_AUTO_TEST_CASE( test_dummy_plugin_handler_parallel )
{
       CPluginSearchpath sp(true);
       sp.add("testplug");
       CTestPluginHandler::set_search_path(sp);
       auto callback = [](const C1DParallelRange & range, int init) {
              CThreadMsgStream thread_stream;
              TRACE_FUNCTION;
              int fails = init;

              for (auto i = range.begin(); i != range.end(); ++i) {
                     const CTestPluginHandler::Instance& handler = CTestPluginHandler::instance();

                     if (handler.size() != 3) {
                            cvfail() << "handler.size() == 3\n";
                            ++fails;
                     }

                     if (handler.get_plugin_names() != "dummy1 dummy2 dummy3 ") {
                            cvfail() << "handler.get_plugin_names() == 'dummy1 dummy2 dummy3 '\n";
                            ++fails;
                     }

                     if (!handler.get_plugin("dummy3")->has_property(test_property)) {
                            cvfail() << "handler.get_plugin(\"dummy3\")->has_property(test_property)\n";
                            ++fails;
                     }

                     if (handler.get_plugin("dummy1")->has_property(test_property)) {
                            cvfail() << "!handler.get_plugin(\"dummy1\")->has_property(test_property)";
                            ++fails;
                     }
              }

              return fails;
       };
       int fails = preduce( C1DParallelRange(0, 4, 1), 0, callback, [](int x, int y) {
              return x + y;
       });
       BOOST_CHECK_EQUAL(fails, 0);
}


BOOST_AUTO_TEST_CASE( test_dummy_plugin_handler )
{
       CPluginSearchpath sp(true);
       sp.add("testplug");
       CTestPluginHandler::set_search_path(sp);
       const CTestPluginHandler::Instance& handler = CTestPluginHandler::instance();
       BOOST_CHECK(handler.size() == 3);
       BOOST_CHECK(handler.get_plugin_names() == "dummy1 dummy2 dummy3 ");
       BOOST_CHECK(handler.get_plugin("dummy3")->has_property(test_property));
       BOOST_CHECK(!handler.get_plugin("dummy1")->has_property(test_property));
       cvdebug() << handler.get_plugin("dummy3")->get_descr()  << "\n";
       cvdebug() << handler.get_plugin("dummy1")->get_descr()  << "\n";
       BOOST_CHECK_EQUAL(handler.get_plugin("dummy1")->get_descr(),
                         string("test_dummy_symbol from dummy1"));
       BOOST_CHECK_EQUAL(handler.get_plugin("dummy3")->get_descr(),
                         std::string("test_dummy_symbol from dummy3"));
}


class CDummy1Override : public CTestPlugin
{
public:
       CDummy1Override();
private:
       virtual const string do_get_descr() const;
};

CDummy1Override::CDummy1Override():
       CTestPlugin("dummy1")
{
       set_priority(1);
       add_property(test_property);
}

const std::string test_dummy_symbol()
{
       return "test_dummy_symbol from main application";
}

const string CDummy1Override::do_get_descr() const
{
       return test_dummy_symbol();
}

BOOST_AUTO_TEST_CASE( test_dummy_plugin_handler_override_plugin )
{
       CPluginSearchpath sp(true);
       sp.add("testplug");
       CTestPluginHandler::set_search_path(sp);
       auto& handler = CTestPluginHandler::instance();
       shared_ptr<CDummy1Override> override(new CDummy1Override);
       BOOST_CHECK(CTestPluginHandler::add_plugin(override));
       BOOST_CHECK(handler.size() == 3);
       BOOST_CHECK(handler.get_plugin_names() == "dummy1 dummy2 dummy3 ");
       BOOST_CHECK(handler.get_plugin("dummy3")->has_property(test_property));
       BOOST_CHECK(handler.get_plugin("dummy1")->has_property(test_property));
       cvdebug() << handler.get_plugin("dummy3")->get_descr()  << "\n";
       cvdebug() << handler.get_plugin("dummy1")->get_descr()  << "\n";
       BOOST_CHECK_EQUAL(handler.get_plugin("dummy1")->get_descr(),
                         string("test_dummy_symbol from main application"));
       BOOST_CHECK_EQUAL(handler.get_plugin("dummy3")->get_descr(),
                         std::string("test_dummy_symbol from dummy3"));
}


BOOST_AUTO_TEST_CASE( test_windows_interface )
{
       bfs::path test_path = bfs::path("c:") / bfs::path("windows");
}


