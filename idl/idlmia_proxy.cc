/* -*- mia-c++ -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#include <sstream>
#include <iostream>
#include <stdexcept>
#include <idl_export.h>

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/3d.hh>

#include <idl/helpers.hh>
#include <idl/reg2d.hh>

using namespace std; 
using namespace mia; 
namespace bfs = ::boost::filesystem; 

class idl_streamredir: public streamredir {
	virtual void do_put_buffer(const char *begin, const char *end); 
}; 

void idl_streamredir::do_put_buffer(const char *begin, const char *end)
{
	string buffer(begin, end); 
	IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_INFO, buffer.c_str());
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

static void set_core_plugin_paths()
{
	list< bfs::path> searchpath; 
	searchpath.push_back( bfs::path("..") /bfs::path("mia") /bfs::path("core") / bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(searchpath); 
}

static void set_2d_plugin_paths()
{
	list< bfs::path> searchpath; 

	bfs::path root_2d(bfs::path("..") /bfs::path("mia") /bfs::path("2d")); 

	searchpath.push_back( root_2d / bfs::path("filter"));
	C2DFilterPluginHandler::set_search_path(searchpath); 
	searchpath.clear(); 

	searchpath.push_back( root_2d / bfs::path("cost"));
	C2DImageCostPluginHandler::set_search_path(searchpath); 
	searchpath.clear(); 


	searchpath.push_back( root_2d / bfs::path("io"));
	C2DImageIOPluginHandler::set_search_path(searchpath); 
	searchpath.clear(); 
	
	searchpath.push_back( root_2d / bfs::path("reg2d"));
	C2DRegModelPluginHandler::set_search_path(searchpath); 
	C2DRegTimeStepPluginHandler::set_search_path(searchpath); 
	searchpath.clear(); 
}

static void set_3d_plugin_paths()
{
	list< bfs::path> searchpath; 
	bfs::path root_3d(bfs::path("..") /bfs::path("mia") /bfs::path("3d")); 

	searchpath.push_back( root_3d / bfs::path("filter"));
	C3DFilterPluginHandler::set_search_path(searchpath); 
	searchpath.clear(); 
}

typedef IDL_VPTR (*idl_function_ext)(int argc, IDL_VPTR *argv); 
typedef void (*idl_procedure_ext)(int argc, IDL_VPTR *argv); 

static IDL_VPTR catch_throw_wrapper(int argc, IDL_VPTR *argv, idl_function_ext f)
{
	try {
		OutputRedirect oredir;
		return f(argc, argv); 
	}

	// we are not using IDL_MSG_RET here, because in IDL it wouldn't return and 
        // most likely "err" would not be cleaned up properly
	catch (invalid_argument& x) {
		stringstream err; 
		err << "MIA - invalid argument:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (runtime_error& x) {
		stringstream err; 
		err << "MIA - runtime error:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (exception& x) {
		stringstream err; 
		err << "MIA - general exception:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (...) {
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, "unknown exception");

	}
	IDL_VPTR retval = (IDL_VPTR) IDL_MemAlloc(sizeof(IDL_VARIABLE), 0, 0); 
	retval->type = IDL_TYP_UNDEF; 
	return retval; 
}

static void  proc_catch_throw_wrapper(int argc, IDL_VPTR *argv, idl_procedure_ext f)
{
	try {
		OutputRedirect oredir;
		f(argc, argv); 
	}

	// we are not using IDL_MSG_RET here, because in IDL it wouldn't return and 
        // most likely "err" would not be cleaned up properly
	catch (invalid_argument& x) {
		stringstream err; 
		err << "MIA - invalid argument:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (runtime_error& x) {
		stringstream err; 
		err << "MIA - runtime error:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (exception& x) {
		stringstream err; 
		err << "MIA - general exception:" << x.what(); 
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, err.str().c_str());
	}
	catch (...) {
		IDL_Message(IDLm_NAMED_GENERIC, IDL_MSG_RET, "unknown exception");

	}
}

void set_test_callback(int argc, IDL_VPTR *argv) 
{
	if (argc != 0) {
		throw invalid_argument("set_test: doesn't take any argument"); 
	}
	set_core_plugin_paths(); 
	set_2d_plugin_paths(); 
	set_3d_plugin_paths(); 
}

IDL_VPTR debug_callback(int argc, IDL_VPTR *argv) 
{
	if (argc != 1) {
		throw invalid_argument("debug: only support exactly one argument"); 
	}
	string old_level(g_verbose_dict.get_name(cverb.get_level())); 
	string level = idl_get_string(argv[0]); 
	cverb.set_verbosity(g_verbose_dict.get_value(level.c_str())); 
	return argv[0]; 
}

void debug_on_callback(int argc, IDL_VPTR *argv) 
{
	if (argc != 0) {
		throw invalid_argument("debug_om: no argument expected"); 
	}
	cverb.set_verbosity(vstream::ml_debug); 
}


IDL_VPTR echo_callback(int argc, IDL_VPTR *argv) 
{
	if (argc != 1) {
		throw invalid_argument("echo: only support exactly one argument"); 
	}
	return argv[0]; 
}

extern IDL_VPTR filter2d_callback(int argc, IDL_VPTR *argv); 
extern IDL_VPTR filter3d_callback(int argc, IDL_VPTR *argv); 
extern IDL_VPTR read2dimage_callback(int argc, IDL_VPTR *argv); 

#define WRAP_CPP_CALL( FUNCTION )					\
	extern "C" EXPORT IDL_VPTR FUNCTION(int argc, IDL_VPTR *argv, char *argk) \
	{						\
		return catch_throw_wrapper(argc, argv, FUNCTION##_callback); \
	}

#define WRAP_CPP_PROC( FUNCTION )					\
	extern "C" EXPORT void FUNCTION(int argc, IDL_VPTR *argv, char *argk) \
	{						\
		proc_catch_throw_wrapper(argc, argv, FUNCTION##_callback); \
	}

		
WRAP_CPP_CALL(echo);  
WRAP_CPP_CALL(filter2d);  
WRAP_CPP_CALL(reg2d);  
WRAP_CPP_CALL(deform2d);  
WRAP_CPP_CALL(filter3d);
WRAP_CPP_CALL(reg3d);  
WRAP_CPP_CALL(deform3d);  
WRAP_CPP_CALL(read2dimage); 

WRAP_CPP_CALL(debug);  
WRAP_CPP_PROC(debug_on);  
WRAP_CPP_PROC(set_test);  

// IDL headers are not const-correct 
#define UNCONST(S) const_cast<char*>(S)

static IDL_SYSFUN_DEF2 function_addr[]  = {
	{{(IDL_SYSRTN_GENERIC)echo}, UNCONST("MIA_ECHO"), 1, 1, 0, 0 }, 
	{{(IDL_SYSRTN_GENERIC)filter2d}, UNCONST("MIA_FILTER2D"), 2, 2, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)filter3d}, UNCONST("MIA_FILTER3D"), 2, 2, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)reg2d}, UNCONST("MIA_NRREG2D"), idx_nrreg_numarg, idx_nrreg_numarg, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)deform2d}, UNCONST("MIA_DEFORM2D"), idx_deform_numarg, idx_deform_numarg, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)reg3d}, UNCONST("MIA_NRREG3D"), idx_nrreg_numarg, idx_nrreg_numarg, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)deform3d}, UNCONST("MIA_DEFORM3D"), idx_deform_numarg, idx_deform_numarg, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)read2dimage}, UNCONST("MIA_READ2DIMAGE"), 1, 1, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)debug}, UNCONST("MIA_DEBUG_LEVEL"), 1, 1, 0, 0 },
};

static IDL_SYSFUN_DEF2 procedure_addr[]  = {

	{{(IDL_SYSRTN_GENERIC)debug_on}, UNCONST("MIA_DEBUG"), 0, 0, 0, 0 },
	{{(IDL_SYSRTN_GENERIC)set_test}, UNCONST("MIA_SET_TEST"), 0, 0, 0, 0 },
}; 

extern "C" EXPORT IDL_SYSFUN_DEF2 * get_function_list(int *n)
{
	*n = sizeof(function_addr) / sizeof(function_addr[0]);
	return function_addr; 
}

extern "C" EXPORT IDL_SYSFUN_DEF2 * get_procedure_list(int *n)
{
	*n = sizeof(procedure_addr) / sizeof(procedure_addr[0]);
	return procedure_addr; 
}
