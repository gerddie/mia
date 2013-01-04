/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <config.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <sstream>

#define __STDC_UTF_16__ 1

#include "mex.h"

namespace mia {
	const std::string get_plugin_root(); 
}

#ifdef WIN32
#include <windows.h>
typedef HINSTANCE ModuleHandle;
#define dlopen(name, flags) LoadLibrary(name)
#define dlclose(handle) FreeLibrary(handle)
#define dlsym(handle, symbol) (void*)GetProcAddress(handle, symbol);
#define dlerror() "unable to load DLL" 
#define MODULE_SUFFIX "dll"
#else
#include <dlfcn.h>
typedef void *ModuleHandle; 
#define MODULE_SUFFIX "so" 
#include <dlfcn.h>
#endif

using namespace std; 
typedef void (*mexcall)(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
typedef void (*fpreptest)(void); 


#ifdef WIN32
static const char *searchpath = "\\matlab\\miamex_proxy."MODULE_SUFFIX; 
static const char *test_searchpath = "miamex_proxy."MODULE_SUFFIX;
#define snprintf _snprintf
#else
static const char *searchpath = "/matlab/miamex_proxy."MODULE_SUFFIX; 
static const char *test_searchpath = "./miamex_proxy."MODULE_SUFFIX;
#endif
static int is_test = 0; 


#ifdef WIN32
static string get_plugin_root()
{
	static string result; 
	static bool result_valid = false; 
	if (!result_valid) {
		LONG retval; 
		LONG bsize = 0; 
		HKEY hkey;
		string subkey = "SOFTWARE\\"VENDOR"\\"PACKAGE; 
		retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_READ, &hkey); 
		if (retval != ERROR_SUCCESS) {
				stringstream err; 
				err << "Unknown registry key 'HKEY_LOCAL_MACHINE\\" << subkey << "'"; 
				throw runtime_error(err.str()); 
		}
		
		retval = RegQueryValue(hkey, NULL, NULL, &bsize); 
		if (retval == ERROR_SUCCESS) {
			char *keybuffer = new char[bsize]; 
			retval = RegQueryValue(hkey, NULL, keybuffer, &bsize);
			if (retval != ERROR_SUCCESS) {
					stringstream err; 
					err << "Unknown registry key '" << subkey << "'"; 
					throw runtime_error(err.str()); 
			}
			result = string(keybuffer) + string("\\") + string(PLUGIN_INSTALL_PATH); 
			result_valid = true; 
			delete[] keybuffer; 
		}else{
			stringstream err; 
			err << "Unknown registry key '" << subkey << "'"; 
			throw runtime_error(err.str()); 
		}
		RegCloseKey(hkey); 
	}
	return result; 
}
#else
const string get_plugin_root() 
{
	return string(::mia::get_plugin_root());
}
#endif


static ModuleHandle do_load_dll()
{
	ModuleHandle handle = NULL; 
	const string sp(is_test ? test_searchpath : get_plugin_root() + searchpath); 
	
	handle = dlopen(sp.c_str(), RTLD_NOW|RTLD_GLOBAL); 
	if (!handle) {
		char errmsg[1024]; 
		snprintf(errmsg, 1023, "unable to load '%s' dynamic library", sp.c_str()); 
		mexErrMsgTxt(errmsg); 
		return NULL; 
	}
	return handle; 
}

static mexcall do_get_interface(ModuleHandle handle)
{
	mexcall f = (mexcall)dlsym(handle, "miamex"); 
	if (!f)
		mexErrMsgTxt("Symbol 'miamex' not found in miamex_proxy"); 
	return f; 
}

static void do_close_dll(ModuleHandle handle) 
{
	dlclose(handle);
}

static char *get_string(const mxArray *prhs)
{
	int buflen = (mxGetM(prhs) * mxGetN(prhs)) + 1;
	char *s = (char *)mxCalloc(buflen, sizeof(char));
	int status = mxGetString(prhs, s, buflen);
	if(status != 0) 
		mexWarnMsgTxt("Not enough space. String is truncated.");
	return s; 
}

extern "C" void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	static ModuleHandle handle = 0; 
	static mexcall f = 0; 

	if (nrhs == 1) {
		if ( mxIsChar(prhs[0]) == 1 && mxGetM(prhs[0]) == 1) {
			char *command = get_string(prhs[0]); 
			
			if (!strcmp("set_test", command))
				is_test = 1;
		}
	}

	if (!handle) {
		handle = do_load_dll();
		if (!handle) {
			mexErrMsgTxt("miamex_proxy: Library not found"); 
			return; 
		}
		f = do_get_interface(handle); 
	}
	if (!f) {
		mexErrMsgTxt("miamex: Symbol not found"); 
		return; 
	}
	
	f(nlhs, plhs, nrhs, prhs);
}

