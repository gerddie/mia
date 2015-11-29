/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <ostream>
#include <iostream>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/streamredir.hh>
#include <mia/core/msgstream.hh>

using namespace mia;
using namespace std;
using namespace boost::unit_test;

class Cstreamredir: public streamredir {
public:
	Cstreamredir(ostringstream& output);
	~Cstreamredir();
private:
	virtual void do_put_buffer(const char *begin, const char *end);
	ostringstream& m_output;
};

Cstreamredir::Cstreamredir(ostringstream& output):
	m_output(output)
{
}

Cstreamredir::~Cstreamredir()
{
	sync();
}

void Cstreamredir::do_put_buffer(const char *begin, const char *end)
{
	while (begin != end)
		m_output << *begin++;
}

BOOST_AUTO_TEST_CASE( test_streamredir )
{
	ostringstream test;

	Cstreamredir s(test);
	ostream test_output(&s);

	test_output << "test\n";
	cvdebug() << "test string >>" << test.str() << "<<\n";

	streambuf *os = test_output.rdbuf(0);
	BOOST_CHECK_EQUAL((void*)os, (void *)&s);

	BOOST_CHECK(test.str() == "test");
}

BOOST_AUTO_TEST_CASE( test_streamredir_overflow )
{
	ostringstream test;

	Cstreamredir s(test);
	ostream test_output(&s);

	for (size_t i = 0; i < 2000; ++i) {
		test_output << i;
	}
}
