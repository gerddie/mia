/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <sstream>
#include <stdexcept>

#ifndef WIN32
#include <dlfcn.h>
#define DLL_FLAGS RTLD_LAZY | RTLD_GLOBAL
#else
#define DLL_FLAGS 0
#endif

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  ifdef __GNUC__
#    define EXPORT_HANDLER __attribute((visibility("default")))
#  else
#    define EXPORT_HANDLER
#  endif
#endif


#include <mia/core/msgstream.hh>
#include <mia/core/module.hh>

NS_MIA_BEGIN

CPluginModule::CPluginModule(const char *path):
	_M_loader(path, DLL_FLAGS ),
	m_refs(0),

	m_name(path)

{
	cvdebug() << "loading module '" << path << "'\n";
}

CPluginModule::~CPluginModule()
{
	TRACE("CPluginModule::~CPluginModule()");
	if (m_refs)
		cverr() << "Module _'" << m_name << "' deleted with " << m_refs << " references\n";

}

CPluginBase *CPluginModule::get_interface() const
{
	FPluginInterface f = reinterpret_cast<FPluginInterface>(_M_loader.get_function("get_plugin_interface"));
	if (!f) {
		cvdebug() << "looking up symbol 'get_plugin_interface' in " << _M_loader.get_name() << " failed\n";
		return NULL;
	}
	return f();
}


NS_MIA_END
