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

#include <cstring>
#include <sstream>
#include <iostream>

#ifndef WIN32
#include <dlfcn.h>
#endif

#include <idl_export.h>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/3d.hh>

using namespace std; 
using namespace mia; 
namespace bfs = boost::filesystem; 

#ifdef WIN32
#define DYNAMIC_EXPORT  __declspec(dllexport)
#else
#define DYNAMIC_EXPORT  
#endif

#define IDL_Message_call(x,y,z)  IDL_Message(x,y,z)

typedef int (*mia_idl_call)(int argc, void *args[]);
typedef struct { 
	const char *name;  
	mia_idl_call callback;
} SCallbackMap; 

extern int filter2d_internal(int argc, void *argv []); 
extern int filter3d_internal(int argc, void *argv []); 
static int echo_internal(int argc, void *argv []); 
static int prepare_test(int argc, void *argv []); 
static int set_debug_level(int argc, void *argv []); 

const SCallbackMap callback_map[] = {
	{"echo", echo_internal},
	{"set_test", prepare_test}, 
	{"filter2d", filter2d_internal},
	{"filter3d", filter3d_internal},
	{"set_debug_level", set_debug_level}, 
	{0, 0}
};

class idl_streamredir: public streamredir {
	virtual void do_put_buffer(const char *begin, const char *end); 
}; 

void idl_streamredir::do_put_buffer(const char *begin, const char *end)
{
	string buffer(begin, end); 
	IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_INFO, buffer.c_str());
}

class OutputRedirect : public ostream {
public:
	OutputRedirect(): ostream(new idl_streamredir())
	{
		vstream::instance().set_stream(*this);
	}
	~OutputRedirect()
	{
		vstream::instance().set_stream(std::cerr);
	}
};

static void prepare_3dfilter_plugin_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back(bfs::path("3d") / bfs::path("filter")); 
	searchpath.push_back(bfs::path("..")/ bfs::path("mia") / bfs::path("3d") / bfs::path("filter")); 
	
	C3DFilterPluginHandler::set_search_path(searchpath); 	
}

static void prepare_2dfilter_plugin_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back(bfs::path("2d") / bfs::path("filter")); 
	searchpath.push_back(bfs::path("..")/ bfs::path("mia") / bfs::path("2d") / bfs::path("filter")); 
	
	C2DFilterPluginHandler::set_search_path(searchpath); 	
}

static void prepare_spacial_kernel_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back(bfs::path("core")/bfs::path("spacialkernel"));
	searchpath.push_back(bfs::path("..")/ bfs::path("mia") / bfs::path("core")/bfs::path("spacialkernel"));

	C1DSpacialKernelPluginHandler::set_search_path(searchpath); 
}

struct STestPrep {
	STestPrep(){
		prepare_3dfilter_plugin_path();
		prepare_2dfilter_plugin_path(); 
		prepare_spacial_kernel_path(); 
	}
};

static int prepare_test(int argc, void *argv [])
{
	static STestPrep test_prep;
	return 0; 
}

static int set_debug_level(int argc, void *argv [])
{
	if (argc != 1)
		throw invalid_argument("set_debug_level: wrong number of arguments"); 
	IDL_STRING *level = (IDL_STRING *) argv[0]; 
	string debug_level(level->s, level->slen); 
	cverb.set_verbosity(g_verbose_dict.get_value(debug_level.c_str())); 
	return 0; 
}

static int echo_internal(int argc, void *argv [])
{
	if (argc < 1)
		return -2; 


	IDL_STRING *command = (IDL_STRING *)argv[0]; 
	if (!strncmp(command->s, "test", command->slen)) {
		IDL_INT *in = (IDL_INT *)argv[1];  
		IDL_INT *out = (IDL_INT *)argv[2];  
		*out = *in; 
		return 0; 
	}
	throw invalid_argument("echo_internal: fail");	
	return -1; 
}

extern "C" int DYNAMIC_EXPORT mia2(int argc, void *argv [])
{
	try {
		OutputRedirect oredir; 
		if (argc < 1) {
			IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_RET, "no function given");
			return -2; 
		}
		
		const IDL_STRING *function = (IDL_STRING *)argv[0]; 
		const SCallbackMap *cm  = callback_map; 
		
		while (cm->name && strncmp(function->s, cm->name, function->slen))
			++cm; 
		
		if (!cm->callback) {
			stringstream err; 
			err << "Function '"; 
			for (int i = 0; i < function->slen; ++i)
				err << function->s[i]; 
			err << "' not supported"; 
			throw invalid_argument(err.str());
		}
		return cm->callback(argc - 1, &argv[1]); 
	}

	// we are not using IDL_MSG_RET here, because in IDL it wouldn't return and 
        // most likely "err" would not be cleaned up properly
	catch (invalid_argument& x) {
		stringstream err; 
		err << "MIA - invalid argument:" << x.what(); 
		IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_INFO, err.str().c_str());
		return -1; 
	}
	catch (runtime_error& x) {
		stringstream err; 
		err << "MIA - runtime error:" << x.what(); 
		IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_INFO, err.str().c_str());
		return -3; 
	}
	catch (exception& x) {
		stringstream err; 
		err << "MIA - general exception:" << x.what(); 
		IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_INFO, err.str().c_str());
		return -4; 
	}
	catch (...) {
		IDL_Message_call(IDLm_NAMED_GENERIC, IDL_MSG_INFO, "unknown exception");
		return -5; 
	}
}

