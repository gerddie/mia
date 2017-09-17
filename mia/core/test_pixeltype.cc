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

#include <climits>

#define BOOST_TEST_DYN_LINK
#include <boost/any.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core/pixeltype.hh>
#include <mia/core/msgstream.hh>
NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_pixeltype_translation )
{

	BOOST_CHECK(CPixelTypeDict.get_name_set().size() == 12);
	BOOST_CHECK(it_none == 11);

	BOOST_CHECK(CPixelTypeDict.get_value("bit")    == it_bit);
	BOOST_CHECK(CPixelTypeDict.get_value("ubyte")  == it_ubyte);
	BOOST_CHECK(CPixelTypeDict.get_value("sbyte")  == it_sbyte);
	BOOST_CHECK(CPixelTypeDict.get_value("ushort") == it_ushort);
	BOOST_CHECK(CPixelTypeDict.get_value("sshort") == it_sshort);
	BOOST_CHECK(CPixelTypeDict.get_value("uint")   == it_uint);
	BOOST_CHECK(CPixelTypeDict.get_value("sint")   == it_sint);
	BOOST_CHECK(CPixelTypeDict.get_value("ulong")  == it_ulong);
	BOOST_CHECK(CPixelTypeDict.get_value("slong")  == it_slong);
	BOOST_CHECK(CPixelTypeDict.get_value("float")  == it_float);
	BOOST_CHECK(CPixelTypeDict.get_value("double") == it_double);
	BOOST_CHECK(CPixelTypeDict.get_value("none") == it_none);
	try {
		CPixelTypeDict.get_value("something");
		BOOST_FAIL("pixel type name 'something' should throw");
	}
	catch (std::invalid_argument& x) {
		cvdebug() << "caught expected exception '" << x.what() << "'\n";
	}
}


