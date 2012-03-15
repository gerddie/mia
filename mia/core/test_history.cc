/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <sys/types.h>

#ifndef WIN32
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <config.h>
#include <stdexcept>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/iodata.hh>

using namespace std;
NS_MIA_USE

static const char *revision = "$Revision: 937 $:";


const SProgramDescription g_general_help = {
	"Test", 
	"Short help", 
	"This program tests the history entries.", 
	"Example text", 
	"Example"
}; 




typedef TDictMap<int> CDict;
const CDict::Table dict[] = {
	{"testa", 1, "test A"},
	{"testb", 2, "test B"},
	{"testc", 3, "test C"},
	{NULL, 0, ""}
};

CDict the_dict(dict);

namespace options {

	// required
	string some_single_string;
	float  some_float = -1.0f;
	// optional
	int some_int = -1;
	bool   true_bool = false;
	bool internal = false;
	int dict_val = 1;
#ifdef OPT_CAN_VECTOR
	vector<string> series_of_strings;
	vector<string> other_series_of_strings;
	vector<int> series_of_int;
	vector<float> series_of_float;
#endif
}

const char *float_str = "2.1";
const char *int_str = "1231";

#ifdef OPT_CAN_VECTOR
const char *vstring = "string1,string2,string3";
const char *ovstring = "ostring1,ostring2,ostring3";
const char *vint = "10,20,-10";
const char *vfloat = "10.1,20.2,-10.4";
#endif
const char *some_string = "some_string";
const char *dict_val = "testb";


bool fork_and_run_check(const char *me)
{
	pid_t pid = fork();
	if (!pid) { // child process
		execlp(me, me,
		       "-s",some_string,
		       "-f", float_str,
		       "-i", int_str,
		       "--tbool",
		       "-d", dict_val,
#ifdef OPT_CAN_VECTOR
		       "-v", vstring,
		       "-o",ovstring,
		       "-u", vint,
		       "-a", vfloat,
#endif
		       "-V", "debug",
		       "--internal",
		       NULL);
		cvfail() << "unable to start myself" << endl;
		return -1;
	}else {
		int result = -1;
		return wait(&result) == pid;
	}
}

bool check_entry(const string& key, const string& val, const CHistoryRecord& hr)
{
	CHistoryRecord::const_iterator i = hr.find(key);
	if (i == hr.end()) {
		cvfail() << "key "<< key << " not found" << endl;
		return false;
	}
	bool success = val == i->second;
	if (!success)
		cvfail() << val << " vs. " << i->second << endl;
	return success;
}

int main(int argc, const char *argv[])
{

	CCmdOptionList options(g_general_help);
	options.add(make_opt( options::some_single_string, "string", 's', "some string option",  CCmdOption::required ));
	options.add(make_opt( options::some_float, "float", 'f', "some float option",CCmdOption::required ));
	options.add(make_opt( options::some_int, "int", 'i', "some int option", CCmdOption::required));
	options.add(make_opt( options::true_bool, "tbool", 't', "true_bool"));
	options.add(make_opt( options::internal, "internal", 'l', "internal test option" ));
	options.add(make_opt( options::dict_val, the_dict, "dict", 'd', "other vector of strings"));
#ifdef OPT_CAN_VECTOR
	options.add(make_opt( options::series_of_strings, "vstring", 'v', "a vector of strings", 
				    CCmdOption::required));
	options.add(make_opt( options::other_series_of_strings, "ovstring", 'o', "other vector of strings",
				    CCmdOption::required));
	options.add(make_opt( options::series_of_int, "vint", 'u', "a vector of int", 
				    CCmdOption::required));
	options.add(make_opt( options::series_of_float, "vfloat", 'a', "a vector of float",
				    CCmdOption::required));
#endif

	try {
		if (options.parse(argc, argv)) 
			return EXIT_SUCCESS; 
	}
	catch (invalid_argument& x) {
		assert(!options::internal);
	}

	CHistory& history = CHistory::instance();

	history.append(argv[0], revision, options);

	if (options::internal) {
		// the program was run from the inside, so start the option check
		if (options::some_single_string != string("some_string")) {
			cvfail() << "string failed" << endl;
			return -1;
		}
		if (options::some_float != 2.1f) {
			cvfail() << "float failed" << endl;
			return -1;
		}
		if (options::some_int  != 1231) {
			cvfail() << "int failed" << endl;
			return -1;
		}
		if (!options::true_bool) {
			cvfail() << "bool failed" << endl;
			return -1;
		}

		if (options::dict_val != 2) {
			cvfail() << "dict_val = " <<  dict_val << " -> "<< options::dict_val <<endl;
			cvfail() << "dict failed" << endl;
			return -1;
		}
#ifdef OPT_CAN_VECTOR
		if ((options::series_of_strings[0] != "string1")||
		    (options::series_of_strings[1] != "string2")||
		    (options::series_of_strings[2] != "string3")) {
			cvfail() << "vstring failed" << endl;
			return -1;
		}

		if ((options::other_series_of_strings[0] != "ostring1")||
		    (options::other_series_of_strings[1] != "ostring2")||
		    (options::other_series_of_strings[2] != "ostring3")) {
			cvfail() << "vstring failed" << endl;
			return -1;
		}

		if ((options::series_of_int.size() != 3) ||
		    (options::series_of_int[0] != 10)    ||
		    (options::series_of_int[1] != 20)    ||
		    (options::series_of_int[2] !=-10)) {
			cvfail() << "vint failed" << endl;
			return -1;
		}

		if ((options::series_of_float.size() != 3) ||
		    (options::series_of_float[0] != 10.1f)    ||
		    (options::series_of_float[1] != 20.2f)    ||
		    (options::series_of_float[2] !=-10.4f)) {
			cvfail() << " vfloat failed" << endl;
			return -1;
		}
#endif

		/** now check the history */
		CHistory::const_iterator i = history.begin();
		if (i == history.end()) {
			cvfail() << "no history record available" << endl;
			return -1;
		}
		if (!check_entry("+PROGRAM", argv[0], i->second))
		    return -1;
		if (!check_entry("+LIBMIA_VERSION", PACKAGE_VERSION, i->second))
			return -1;
		if (!check_entry("+USER", getenv("USER"), i->second))
			return -1;
		if (!check_entry("+VERSION", revision, i->second))
			return -1;
		if (!check_entry("float", float_str, i->second))
			return -1;
		if (!check_entry("int", int_str, i->second))
			return -1;
#ifdef OPT_CAN_VECTOR
		if (!check_entry("vstring", vstring, i->second))
			return -1;
		if (!check_entry("vint", vint, i->second))
			return -1;
		if (!check_entry("vfloat", vfloat, i->second))
			return -1;
		if (!check_entry("ovstring", ovstring, i->second))
			return -1;
#endif
		if (!check_entry("string", some_string, i->second))
			return -1;

		if (!check_entry("dict", dict_val, i->second)){
			return -1;
		}

		return EXIT_SUCCESS;

	} else {


		if (!fork_and_run_check(argv[0]))
			cvfail() << "fork_and_run_check\n";


		return EXIT_SUCCESS;

	}
}

