/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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


#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  ifdef __GNUC__
#    define EXPORT_HANDLER __attribute((visibility("default")))
#  else
#    define EXPORT_HANDLER
#  endif
#endif

#include <mia/core/testplugin.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN

static const char *test_plugin_type_str = "test";
static const char *test_plugin_data_str = "none";

const char * const test_property = "testprop";

struct test_plugin_type {
	static  const char *value;
};

const char * test_plugin_type::value =  test_plugin_type_str;

struct test_plugin_data {
	static  const char *type_descr;

};

const char * test_plugin_data::type_descr = test_plugin_data_str;


CTestPlugin::CTestPlugin(const char *name):
	TPlugin<test_plugin_data, test_plugin_type>(name)
{
}

CTestPluginHandlerImpl::CTestPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath):
	TPluginHandler<CTestPlugin>(searchpath)
{
}

CTestPlugin *CTestPluginHandlerImpl::get_plugin(const char *name) const
{
	return plugin(name);
}

template class TPluginHandler<CTestPlugin>;
template class THandlerSingleton<CTestPluginHandlerImpl>;


NS_MIA_END

