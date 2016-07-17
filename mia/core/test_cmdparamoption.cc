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


#include <mia/internal/autotest.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdparamoption.hh>

using namespace mia;
using namespace std;

const SProgramDescription general_help {
	{pdi_group, "Test"}, 
	{pdi_short, "program tests"}, 
	{pdi_description, "This program tests the command line parser."}, 
	{pdi_example_descr, "Example text"}, 
	{pdi_example_code, "Example command"}
};

struct CmdlineParserFixture {
	CmdlineParserFixture():m_level(cverb.get_level()) {
	}
	~CmdlineParserFixture() {
		cverb.set_verbosity(m_level);
	}
private:
	vstream::Level m_level;

};

BOOST_FIXTURE_TEST_CASE( test_string_repeatable_option, CmdlineParserFixture )
{
        const char *v1 = "string";
        const char *v2 = "list"; 
        
        vector<string> value;
	PCmdOption popt(make_repeatable_opt(value, "string", 's', "a string option"));

        popt->set_value(v1);
        
        BOOST_CHECK_EQUAL(value.size(), 1u);
        BOOST_REQUIRE(value.size() == 1);
        BOOST_CHECK_EQUAL(value[0], v1);
        
        popt->set_value(v2);
        BOOST_CHECK_EQUAL(value.size(), 2u);
        BOOST_REQUIRE(value.size() == 2);
        BOOST_CHECK_EQUAL(value[0], v1);
        BOOST_CHECK_EQUAL(value[1], v2);
        
        //BOOST_CHECK_EQUAL(popt->get_value_as_string(), string(str_value));
	
}
