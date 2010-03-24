/* -*- mia-c++ -*-
 * Copyright (c) 2007 Gert Wollny <gert at die.upm.es>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <config.h>
#include <miaconfig.h>
#include <idl_export.h>

#ifdef WIN32
#define MODULE_SUFFIX "dll"
#define DYNAMIC_EXPORT  __declspec(dllexport)
#else
#define MODULE_SUFFIX "so" 
#define DYNAMIC_EXPORT  
#include <dlfcn.h>
#endif

typedef int (*idlfilter)(int argc, void *argv []); 
typedef void (*fpreptest)(void); 

static const char *searchpath =PLUGIN_SEARCH_PATH"idl/idlmiafilter_proxy."MODULE_SUFFIX; 
static const char *test_searchpath ="./idlmiafilter_proxy."MODULE_SUFFIX; 
static int is_test = 0; 

static void *do_load_dll()
{
	void *handle = NULL; 
	const char *sp = is_test ? test_searchpath : searchpath; 
	
	/*IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_INFO, sp); */
	handle = dlopen(sp, RTLD_NOW|RTLD_GLOBAL); 
	if (!handle) {
		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_RET, "unable to load"); 
		return NULL; 
	}
	return handle; 
}

static idlfilter do_get_interface(void *handle)
{
	idlfilter f = (idlfilter)dlsym(handle, "mia2"); 
	if (!f)
		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_RET, "mia2"); 
	return f; 
}

static void do_close_dll(void *handle) 
{
	dlclose(handle);
}


DYNAMIC_EXPORT int mia2(int argc, void *argv[]) 
{
	static void *handle = 0; 
	static idlfilter f; 

	if (argc == 1) {
		IDL_STRING *function = (IDL_STRING *)argv[0];
		if (!strncmp("set_test", function->s, function->slen))
			is_test = 1; 
	}

	if (!handle) {
		handle = do_load_dll(); 
		if (!handle)
			return -2; 
		f = (idlfilter)do_get_interface(handle); 
	}
	
	if (!f) {
		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_RET, "mia2: Symbol not found"); 
		return -1; 
	}
	
	
	return f(argc, argv); 
}
