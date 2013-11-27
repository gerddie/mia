/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/internal/autotest.hh>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;

struct CmdlineParserFixture {
	CmdlineParserFixture():m_level(cverb.get_level()) {
	}
	~CmdlineParserFixture() {
		cverb.set_verbosity(m_level);
	}
private:
	vstream::Level m_level;

};

#define IF_THIS(A)  \
	{ if (A) cvdebug() << #A << " is true\n"; }


const SProgramDescription general_help {
	{pdi_group, "Test"}, 
	{pdi_short, "program tests"}, 
	{pdi_description, "This program tests the command line parser."}, 
	{pdi_example_descr, "Example text"}, 
	{pdi_example_code, "Example command"}
};

BOOST_FIXTURE_TEST_CASE( test_set_option, CmdlineParserFixture )
{
	string value ="false";
	set<string> possible_values;
	possible_values.insert("uno");
	possible_values.insert("dos");
	possible_values.insert("tres");

	PCmdOption popt(make_opt(value, possible_values, "string", 's', "a string option"));

	try {
		popt->set_value("uno");
		BOOST_CHECK(value == "uno");
		BOOST_CHECK(popt->get_value_as_string() == "uno");
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	try {
		const char *str_value_err = "cuadro";
		popt->set_value(str_value_err);
		BOOST_FAIL("error value not detected");
	}
	catch (invalid_argument& x) {
		BOOST_MESSAGE(string("Caught:") + string(x.what()));
	}
}

BOOST_FIXTURE_TEST_CASE( test_string_vector_option, CmdlineParserFixture )
{
	const char *str_value = "string,list";
	vector<string> value;
	PCmdOption popt(make_opt(value, "string", 's', "a string option"));
	try {
		popt->set_value(str_value);
		BOOST_REQUIRE(value.size() == 2);
		BOOST_CHECK_EQUAL(value[0], "string");
		BOOST_CHECK_EQUAL(value[1], "list");
		BOOST_CHECK_EQUAL(popt->get_value_as_string(), string(str_value));
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}


BOOST_FIXTURE_TEST_CASE( test_float_vector_option_expect_two, CmdlineParserFixture )
{
	const char *str_value = "1.2,1.4";
	vector<float> value(2);
	PCmdOption popt(make_opt(value, "float", 's', "a float vector option"));
	try {
		popt->set_value(str_value);
		BOOST_REQUIRE(value.size() == 2);
		BOOST_CHECK_CLOSE(value[0], 1.2f, 0.0001f);
		BOOST_CHECK_CLOSE(value[1], 1.4f, 0.0001f);
		BOOST_CHECK_EQUAL(popt->get_value_as_string(), string(str_value));
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}

BOOST_FIXTURE_TEST_CASE( test_float_vector_option_expect_two_failed, CmdlineParserFixture )
{
	const char *str_value = "1.2,1.4,2.3";
	vector<float> value(2);
	PCmdOption popt(make_opt(value, "float", 's', "a float vector option"));
	BOOST_CHECK_THROW(popt->set_value(str_value), invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_string_option, CmdlineParserFixture )
{
	string value = "string with spaces";
	PCmdOption popt(make_opt(value, "string", 's', "a string option"));

	const char *str_value = "let there be spaces";
	try {
		popt->set_value(str_value);
		BOOST_CHECK_EQUAL(value, string(str_value));
		BOOST_CHECK_EQUAL(popt->get_value_as_string(), string(str_value));
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}


BOOST_FIXTURE_TEST_CASE( test_float_option, CmdlineParserFixture )
{
	float value = 10;
	PCmdOption popt(make_opt(value, "float", 'f', "a float option"));
	const char *str_value = "12.2";
	try {
		popt->set_value(str_value);
		BOOST_CHECK(value == 12.2f);
		BOOST_CHECK(popt->get_value_as_string() == "12.2");
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	const char *str_value_err = "12.1s";
	try {
		popt->set_value(str_value_err);
		BOOST_FAIL("error value not detected");
	}
	catch (invalid_argument& x) {
		BOOST_MESSAGE(string("Caught:") + string(x.what()));
	}
}

BOOST_FIXTURE_TEST_CASE( test_ranged_float_option, CmdlineParserFixture )
{
	float value = 10;
	PCmdOption popt(make_opt(value, -10, 12, "float", 'f', "a float option"));
	const char *str_value = "12.2";
	try {
		popt->set_value(str_value);
		BOOST_CHECK(value == 12.0f);
		BOOST_CHECK(popt->get_value_as_string() == "12");
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	BOOST_CHECK_THROW(popt->set_value("12.1s"), invalid_argument); 
}


BOOST_FIXTURE_TEST_CASE( test_int_option, CmdlineParserFixture )
{
	int value = 10;
	PCmdOption popt(make_opt(value, "int", 'i', "a int option"));
	const char *str_value = "12";
	try {
		popt->set_value(str_value);
		BOOST_CHECK(value == 12);

		BOOST_CHECK(popt->get_value_as_string() == "12");
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	const char *str_value_err = "12.1";
	try {
		popt->set_value(str_value_err);
		BOOST_FAIL("error value not detected");
	}
	catch (invalid_argument& x) {
		BOOST_MESSAGE(string("Caught:") + string(x.what()));
	}
}

enum ETestEnums {te_one, te_two, te_three, te_undefined};

BOOST_FIXTURE_TEST_CASE( test_dict_option, CmdlineParserFixture )
{

	TDictMap<ETestEnums>::Table table[] =
		{
			{"one", te_one, "test 1"},
			{"two", te_two, "test 2"},
			{"three", te_three, "test 3"},
			{NULL, te_undefined, ""}
		};
	TDictMap<ETestEnums> map(table);

	ETestEnums value = te_undefined;

	CCmdOptionList olist(general_help);

	olist.add(make_opt(value, map, "dict", 'd', "a dictmap option"));
	vector<const char *> options;

	options.push_back("self");
	options.push_back("-dtwo");
	BOOST_CHECK_EQUAL(olist.parse(options.size(), &options[0]), CCmdOptionList::hr_no); 

	BOOST_CHECK_EQUAL( olist.get_remaining().size(), 0);
	BOOST_CHECK_EQUAL( value, te_two);
}

BOOST_FIXTURE_TEST_CASE( test_flagstring_option, CmdlineParserFixture )
{

	CFlagString::Table table[] =	{
		{'o', te_one},
		{'t', te_two},
		{'h', te_three},
		{0, te_undefined}
	};
	CFlagString map(table);

	int value = te_undefined;

	CCmdOptionList olist(general_help);

	olist.add(make_opt(value, map, "flags", 'f', "a flagstring option", "flags"));
	vector<const char *> options;

	options.push_back("self");
	options.push_back("-f");
	options.push_back("ot");
	BOOST_CHECK_EQUAL(olist.parse(options.size(), &options[0]), CCmdOptionList::hr_no); 

	BOOST_CHECK_EQUAL( olist.get_remaining().size(), 0);
	BOOST_CHECK(value == te_two || te_one);
}


BOOST_FIXTURE_TEST_CASE( test_parser, CmdlineParserFixture )
{
	vector<const char *> options;

	options.push_back("self");
	options.push_back("leftover");

	int int_value1 = 10;
	int int_value2 = 10;
	int int_value3 = 10;
	unsigned short usval = 12;
	float float_value = 1.2f;
	string s("string");
	vector<string> vector_value;

	bool bingo = false;
	bool bingo2 = false;

	options.push_back("-i12");
	options.push_back("-k");
	options.push_back("13");
	options.push_back("--int-3");
	options.push_back("21");
	options.push_back("-f1.3");
	options.push_back("--string");
	options.push_back("string");
	options.push_back("--vector-string");
	options.push_back("string1,string2,string3");
	options.push_back("-b2u16");
	options.push_back("leftover2");


	CCmdOptionList olist(general_help);

	olist.add(make_opt(int_value1,  "int1", 'i',"a int option"));
	olist.add(make_opt(int_value2, "int2",  'k', "another int option"));
	olist.add(make_opt(int_value3,  "int-3", 0, "yet another int option"));
	olist.add(make_opt(float_value, "float", 'f', "a float option"));
	olist.add(make_opt(s,  "string", 's',"a string option"));
	olist.add(make_opt(bingo,  "bingo", 'b', "a bool option"));
	olist.add(make_opt(bingo2,  "bingo2", '2', "another bool option"));
	olist.add(make_opt(usval,  "ushort", 'u', "a short int option"));
	olist.add(make_opt(vector_value,  "vector-string", 'S', "a vector of strings"));

	BOOST_CHECK_EQUAL(olist.parse(options.size(), (const char**)&options[0], "remaining"),  CCmdOptionList::hr_no);

	for(auto i = olist.get_remaining().begin(); i != olist.get_remaining().end(); ++i)
		BOOST_MESSAGE(*i);

	BOOST_CHECK_EQUAL(int_value1,12);
	BOOST_CHECK_EQUAL(int_value2,13);
	BOOST_CHECK_EQUAL(int_value3, 21);
	BOOST_CHECK_EQUAL(float_value, 1.3f);
	BOOST_CHECK_EQUAL(usval, 16);
	BOOST_CHECK_EQUAL(s, string("string"));

	BOOST_REQUIRE(vector_value.size() == 3);
	BOOST_CHECK_EQUAL(vector_value[0], string("string1"));
	BOOST_CHECK_EQUAL(vector_value[1], string("string2"));
	BOOST_CHECK_EQUAL(vector_value[2], string("string3"));

	BOOST_CHECK_EQUAL(olist.get_remaining().size(), 2u);
}

BOOST_FIXTURE_TEST_CASE( test_parser_errors1, CmdlineParserFixture )
{
	vector<const char *> options;
	options.push_back("self1");
	options.push_back("-H2u16");
	bool bool_value = false;

	CCmdOptionList olist(general_help);
	olist.add(make_opt(bool_value, "bool", 'H', "a bool option"));

	BOOST_CHECK_THROW(IF_THIS(olist.parse(options.size(), &options[0]) == CCmdOptionList::hr_no), invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_missing_argument_short, CmdlineParserFixture )
{
	vector<const char *> options = {"self", "-i"};
	string test; 
	
	CCmdOptionList olist(general_help);
	olist.add(make_opt(test, "lala", 'i', "a string option"));
	
	BOOST_CHECK_THROW( IF_THIS(olist.parse(options.size(), &options[0]) == CCmdOptionList::hr_no), invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_missing_string_argument_long, CmdlineParserFixture )
{
	vector<const char *> options = {"self", "--lala"};
	string test; 
	
	CCmdOptionList olist(general_help);
	olist.add(make_opt(test, "lala", 'i', "a string option"));
	
	BOOST_CHECK_THROW( IF_THIS(olist.parse(options.size(), &options[0]) == CCmdOptionList::hr_no), invalid_argument); 
}


BOOST_FIXTURE_TEST_CASE( test_parser_errors2, CmdlineParserFixture )
{
	vector<const char *> options;
	options.push_back("self1");
	options.push_back("-H2u16");
	bool bool_value = false;

	CCmdOptionList olist(general_help);
	olist.add(make_opt(bool_value, "bool", 'H', "a bool option"));

	BOOST_CHECK_THROW(IF_THIS(olist.parse(options.size(), &options[0]) == CCmdOptionList::hr_no), invalid_argument); 
}

const SProgramDescription general_help_test = {
	{pdi_group,"Test"}, 
	{pdi_short,"Short descr"},
	{pdi_description,"This program tests the command line parser."}, 
	{pdi_example_descr, "Example text"}, 
	{pdi_example_code, "Example command"}
}; 

BOOST_FIXTURE_TEST_CASE( test_parser_usage_output, CmdlineParserFixture )
{
	vector<const char *> options;
	options.push_back("test-program");
	options.push_back("-?");

	CCmdOptionList olist(general_help_test);
	stringstream output; 
	olist.set_logstream(output);
	
	BOOST_CHECK_EQUAL(olist.parse(options.size(), &options[0]), CCmdOptionList::hr_usage); 
	
	const string test("Usage:\n  test-program -V verbose -h help -? usage \n"); 

	BOOST_CHECK_EQUAL(output.str(), test); 
}


BOOST_FIXTURE_TEST_CASE( test_parser_help_output, CmdlineParserFixture )
{
	vector<const char *> options;
	options.push_back("test-program");
	options.push_back("-h");

	CCmdOptionList olist(general_help_test);
	ostringstream output; 
	olist.set_logstream(output);
	
	BOOST_CHECK_EQUAL(olist.parse(options.size(), &options[0]), CCmdOptionList::hr_help); 
	
	const string test("\nProgram group:  Test\n"
			  "    \n    This program tests the command line parser.\n\n"
			  "Basic usage:\n    test-program [options]\n\n"
			  "The program supports the following command line options:\n\n"
			  "Help & Info              \n"
			  "  -V --verbose=warning (dict) \n"  
			  "                        verbosity of output, print messages of given \n"
			  "                        level and higher priorities. Supported \n"
			  "                        priorities starting at lowest level are: \n"
#ifndef NDEBUG 
			  "                          trace: Function call trace\n"
			  "                          debug: Debug output\n"
#endif 
			  "                          info: Low level messages\n"
			  "                          message: Normal messages\n"
			  "                          warning: Warnings\n"
			  "                          fail: Report test failures\n"
			  "                          error: Report errors\n"
			  "                          fatal: Report only fatal errors\n"
			  "     --copyright        print copyright information\n"
			  "  -h --help             print this help\n"
			  "     --help-xml         print help formatted as XML\n"
			  "  -? --usage            print a short help\n"
			  "     --version          print the version number and exit\n\n"
			  "Processing               \n"
#if defined(__PPC__) && ( TBB_INTERFACE_VERSION  < 6101 )
			  "     --threads=1 (int)  Maxiumum number of threads to use for \n"
#else
			  "     --threads=-1 (int) \n"
			  "                        Maxiumum number of threads to use for \n"
#endif

			  "                        processing,This number should be lower or \n"
			  "                        equal to the number of logical processor \n"
			  "                        cores in the machine. (-1: automatic \n"
			  "                        estimation). \n\n"
			  "Example usage:\n  Example text\n"
			  "    \n    test-program Example command\n\n"
			  "Copyright:\n"
			  "  This software is Copyright (c) Gert Wollny 1999-2013 Leipzig, \n"
			  "  Germany and Madrid, Spain. It comes with ABSOLUTELY NO WARRANTY and\n"
			  "  you may redistribute it under the terms of the GNU GENERAL PUBLIC \n"
			  "  LICENSE Version 3 (or later). For more information run the program \n"
			  "  with the option '--copyright'.\n  \n"); 
  

	BOOST_CHECK_EQUAL(output.str().size(), test.size()); 
	BOOST_CHECK_EQUAL(output.str(), test); 

	if (output.str() != test) {
		const auto ostr = output.str(); 
		auto io = ostr.begin(); 
		auto it = test.begin(); 
		while (io != ostr.end() && it != test.end()) {
			if (*io == *it) 
				cerr << *io; 
			else {
				cerr << "\nERROR:'"<< *io << "' versus '" << *it <<"'\n"; 
				break; 
			}
			++io; 
			++it; 
		}
	}
}

