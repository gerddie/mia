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

#include <mia/core/datapool.hh>


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
using namespace boost;


BOOST_AUTO_TEST_CASE( test_pool_inout )
{

	CDatapool::Instance().add("param1", 10);
	CDatapool::Instance().add("param2", string("fun"));

	any p1 = CDatapool::Instance().get("param1");

	BOOST_CHECK(p1.type() == typeid(int));
	BOOST_CHECK_EQUAL(any_cast<int>(p1), 10);

	any p2 = CDatapool::Instance().get("param2");
	BOOST_CHECK(p2.type() == typeid(string));
	BOOST_CHECK_EQUAL(any_cast<string>(p2), string("fun"));

	BOOST_CHECK(!CDatapool::Instance().has_unused_data());
}

BOOST_AUTO_TEST_CASE( test_pool_out_noexists )
{
	BOOST_CHECK_THROW(CDatapool::Instance().get("param3"), invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_pool_get_and_remove )
{
	CDatapool::Instance().add("param1", 10);
	any p1 = CDatapool::Instance().get_and_remove("param1");
	BOOST_CHECK(!CDatapool::Instance().has_key("param1"));
}

BOOST_AUTO_TEST_CASE( test_pool_has_unused )
{
	CDatapool::Instance().add("param1", 10);
	BOOST_CHECK(CDatapool::Instance().has_unused_data());
}

BOOST_AUTO_TEST_CASE( test_pool_has_key )
{
	CDatapool::Instance().add("param1", 10);
	BOOST_CHECK(CDatapool::Instance().has_key("param1"));
	BOOST_CHECK(!CDatapool::Instance().has_key("unknown"));
}
