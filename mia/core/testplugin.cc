/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  ifdef __GNUC__
#    define EXPORT_HANDLER __attribute((visibility("default")))
#  else
#    define EXPORT_HANDLER
#  endif
#endif

#define VSTREAM_DOMAIN "testplugin"

#include <mia/core/testplugin.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN

static const char *test_plugin_type_str = "test";
static const char *test_plugin_data_str = "none";

const char * const test_property = "testprop";

struct test_plugin_type {
	static  const char *type_descr;
};

const char * test_plugin_type::type_descr =  test_plugin_type_str;

struct test_plugin_data {
	static  const char *data_descr;

};

const char * test_plugin_data::data_descr = test_plugin_data_str;


CTestPlugin::CTestPlugin(const char *name):
	TPlugin<test_plugin_data, test_plugin_type>(name)
{
}

CTestPluginHandlerImpl::CTestPluginHandlerImpl()
{
}

CTestPlugin *CTestPluginHandlerImpl::get_plugin(const char *name) const
{
	return plugin(name);
}

template<> 
const char * const TPluginHandler<CTestPlugin>::m_help = "This is a handler for the test plug-ins"; 

template class TPlugin<test_plugin_data, test_plugin_type>; 
template class TPluginHandler<CTestPlugin>;
template class THandlerSingleton<CTestPluginHandlerImpl>;


NS_MIA_END

