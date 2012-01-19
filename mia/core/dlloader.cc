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
		m_name(name) 	{
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
		// For now, the unloading is left to the final clean up when
		// the program ends, this leaves some reachable memory blocks
		//
		// dlclose(m_module);
	}
	void *get_function(const char *name) {
		return dlsym(m_module, name);
	}
	const string& get_name() const {
		return m_name;
	}
private:
	ModuleHandle m_module;
	string m_name;
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

NS_MIA_END

