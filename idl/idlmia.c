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

#include <stdio.h>
#include <idl_export.h>
#include <string.h>
#include <assert.h>

#include <config.h>
#include <miaconfig.h>

#ifdef WIN32
#include <windows.h>
typedef HINSTANCE ModuleHandle;
#define dlopen(name, flags) LoadLibrary(name)
#define dlclose(handle) FreeLibrary(handle)
#define dlsym(handle, symbol) (void*)GetProcAddress(handle, symbol);
#define dlerror() "unable to load DLL" 
#define MODULE_SUFFIX "dll"
#define EXPORT __declspec(dllexport)
#else
#include <dlfcn.h>
typedef void *ModuleHandle; 
#define MODULE_SUFFIX "so" 
#include <dlfcn.h>
#define EXPORT
#endif


typedef IDL_SYSFUN_DEF2* (*f_get_map)(int *n); 
#ifdef WIN32
    static const char *searchpath ="idlmia_proxy."MODULE_SUFFIX; 
#else
	static const char *searchpath =PLUGIN_SEARCH_PATH"idl/idlmia_proxy."MODULE_SUFFIX; 
#endif

static const char *test_searchpath ="./idlmia_proxy."MODULE_SUFFIX; 

static ModuleHandle do_load_dll()
{
	ModuleHandle handle = NULL; 
	handle = dlopen(searchpath, RTLD_NOW|RTLD_GLOBAL); 

	if (!handle) {
		handle = dlopen(test_searchpath, RTLD_NOW|RTLD_GLOBAL); 
	}
	if (!handle) {
	IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_RET, "unable to load 'idlmia_proxy'"); 
		return NULL; 
	}
	return handle; 
}

static f_get_map do_get_interface(void *handle, const char *name)
{
	f_get_map f = (f_get_map)dlsym(handle, name); 
	if (!f)
		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_RET, name); 
	return f; 
}

static ModuleHandle g_handle = 0; 

EXPORT int IDL_Load(void) 
{
	IDL_SYSFUN_DEF2 *function_addr; 
	IDL_SYSFUN_DEF2 *procedure_addr; 
	int n_p = 0; 
	int n_f = 0; 
	
	f_get_map f_getter = NULL; 
	f_get_map p_getter = NULL; 

	g_handle = do_load_dll();
	
	if (!g_handle)
		return IDL_FALSE; 
	
	f_getter = do_get_interface(g_handle, "get_function_list"); 
	if (!f_getter) 
		return IDL_FALSE; 

	function_addr = f_getter(&n_f); 

	p_getter = do_get_interface(g_handle, "get_procedure_list"); 
	if (!p_getter) 
		return IDL_FALSE; 

	function_addr = f_getter(&n_f); 
	procedure_addr = p_getter(&n_p); 


	return IDL_SysRtnAdd(function_addr, TRUE, n_f) && IDL_SysRtnAdd(procedure_addr, FALSE, n_p); 
}

