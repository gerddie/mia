/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <sstream>
#include <string>
#include <stdexcept>

#include <mia/core/dlloader.hh>

#ifdef WIN32
#include <windows.h>
typedef HINSTANCE ModuleHandle;
#define dlopen(name, flags) LoadLibrary(name)
#define dlclose(handle) FreeLibrary(handle)
#define dlsym(handle, symbol) (void*)GetProcAddress(handle, symbol);
#define dlerror() "unable to load DLL"
#else
#include <dlfcn.h>
typedef void *ModuleHandle;
#endif

NS_MIA_BEGIN
using namespace std;

struct CDLLoaderData {

	CDLLoaderData(const char *name, int flags):
		// valgrind will report reachable memory blocks on this call, 
		// see destructor for a justification 
	        m_module(dlopen(name, flags)),
		m_name(name), 
		m_unload (true){
		if (!m_module) {
			stringstream s;
			s << name << ":'" << dlerror() << "'";
			throw runtime_error(s.str());
		}
	}
	~CDLLoaderData() {
		//
		// normally, a call to dlclose would be apropriated,
		// however, using reference counting doesn't work properly, since
		// the plugin-destructor is executing code within the module,
		// after releasing it. Hence the module might be unloaded even though,
		// the code is still needed.
		// For now, the unloading is normally left to the final clean up when
		// the program ends, this leaves some reachable memory blocks
		// if the called knows that unloading is save, then 
		// unloading can be explicitely specified.
		//
		if (m_unload) 
			dlclose(m_module);
	}
	
	void *get_function(const char *name) {
		return dlsym(m_module, name);
	}
	
	const string& get_name() const {
		return m_name;
	}
	
	void set_unload_library(bool value) {
		m_unload = value; 
	}


private:
	ModuleHandle m_module;
	string m_name;
	bool m_unload; 
};

CDLLoader::CDLLoader(const char *name, int flags):
	impl(new CDLLoaderData(name, flags))
{
}

CDLLoader::~CDLLoader()
{
	delete impl;
}

void *CDLLoader::get_function(const char *name) const
{
	return impl->get_function(name);
}

const string& CDLLoader::get_name() const
{
	return impl->get_name();
}

void CDLLoader::set_unload_library()
{
	impl->set_unload_library(true); 
}

void CDLLoader::set_keep_library() 
{
	impl->set_unload_library(false); 
}


NS_MIA_END

