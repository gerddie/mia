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

#include <stdexcept>
#include <climits>

#define BOOST_TEST_DYN_LINK
#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/file.hh>


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;


BOOST_AUTO_TEST_CASE( test_file)
{
	try {
		COutputFile f("testfile");
		unlink("testfile");
	}
	catch (...) {
		BOOST_FAIL("unable to create a file for writing in current directory");
	}

	try {
#ifdef WIN32
		CInputFile f("test-core.exe");
#else
		CInputFile f("test-core");
#endif
	}
	catch (...) {
		BOOST_FAIL("unable to open 'test-core' for reading in current directory");
	}


	try {
		CInputFile f("this-file-should-not-exist.11111");
		BOOST_FAIL("'this-file-should-not-exist.11111' was opened for reading");
	}
	catch (runtime_error& x) {
		BOOST_MESSAGE(string("caught an expected exception:") + x.what());
	}

	try {
		CInputFile f("/nnananan5676281/7828/1ttgfthis-file-should-not-exist.11111");
		BOOST_FAIL("'/nnananan5676281/7828/1ttgf/this-file-should-not-be-created.11111' was opened for writing");
	}

	catch (runtime_error& x) {
		BOOST_MESSAGE(string("caught an expected exception:") + x.what());
	}
}


