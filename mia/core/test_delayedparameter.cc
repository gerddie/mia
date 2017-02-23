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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/delayedparameter.hh>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
using namespace boost;



BOOST_AUTO_TEST_CASE( test_pool_inout )
{
	TDelayedParameter<int> a("code");

	BOOST_CHECK(a.key_is_valid()); 

	BOOST_CHECK(!a.pool_has_key()); 

	CDatapool::instance().add("code", 10);

	BOOST_CHECK(a.pool_has_key()); 

	try {
		BOOST_CHECK_EQUAL(10, a.get());
	}
	catch (std::exception& x) {
		BOOST_FAIL(x.what());
	}
}


BOOST_AUTO_TEST_CASE( test_is_invalid )
{
	TDelayedParameter<int> a;
	BOOST_CHECK(!a.key_is_valid()); 
}


BOOST_AUTO_TEST_CASE( test_pool_type_mismatch )
{
	TDelayedParameter<int> a("code");
	CDatapool::instance().add("code", 10.1);
	
	BOOST_CHECK_THROW(a.get(), std::bad_cast); 
}


