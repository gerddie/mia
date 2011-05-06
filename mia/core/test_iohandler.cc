/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/dummyhandler.hh>

NS_MIA_USE
using namespace std;
using namespace boost;
namespace bfs = ::boost::filesystem;

struct DummyPluginFixture {
	DummyPluginFixture();
};

DummyPluginFixture::DummyPluginFixture()
{
	std::list<bfs::path> searchpath;
	searchpath.push_back(bfs::path("testplug"));
	CTestIOPluginHandler::set_search_path(searchpath);
}


BOOST_FIXTURE_TEST_CASE(  test_dummy_io, DummyPluginFixture )
{
	std::list<bfs::path> searchpath;

	searchpath.push_back(bfs::path("testplug"));

	CTestIOPluginHandler::set_search_path(searchpath);

	const CTestIOPluginHandler::Instance&  handler = CTestIOPluginHandler::instance();

	BOOST_CHECK_EQUAL(handler.size(), 3u);

	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "datapool la lo ");

	CTestIOPlugin::PData data_lala = handler.load("test.la");
	BOOST_CHECK_EQUAL(*data_lala, "lala");

	cvdebug() << "data_lala " << data_lala->get_source_format() << "\n";
	BOOST_CHECK_EQUAL(data_lala->get_source_format(), "la");

	CTestIOPlugin::PData data_lolo  = handler.load("test.lo");
	BOOST_CHECK_EQUAL(*data_lolo,  "lolo");

	cvdebug() << "data_lolo " << data_lolo->get_source_format() << "\n";
	BOOST_CHECK_EQUAL(data_lolo->get_source_format(), "lo");

	string src_fmt = data_lala->get_source_format();
	cvdebug() << "src_fmt = " << src_fmt << "\n";
	BOOST_CHECK(handler.prefered_plugin_ptr(src_fmt));

}

BOOST_AUTO_TEST_CASE(  test_datapool_io )
{
	const CTestIOPluginHandler::Instance&  handler = CTestIOPluginHandler::instance();

	CDummyType data("testdata");

	BOOST_CHECK(handler.save("datapool", "testdummy.datapool", data));
	try {
		std::shared_ptr<CDummyType > value = handler.load("testdummy.datapool");
	}
	catch (std::exception& x) {
		BOOST_FAIL(x.what());
	}
}

