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

#include <config.h>
#include <cstdlib>

#ifdef WIN32
#include <windows.h>
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

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <mia/core/plugin_base.hh>
#include <miaconfig.h>



NS_MIA_BEGIN
using namespace std;

const std::string plugin_help("help");

CPluginBase::CPluginBase(const char *name):
	m_name(name),
	m_next_interface(NULL)
{
}

void CPluginBase::set_priority(unsigned p)
{
	m_priority = p; 
}

unsigned CPluginBase::get_priority() const
{
	return m_priority; 
}


CPluginBase::~CPluginBase()
{
	TRACE("CPluginBase::~CPluginBase()");
}

void CPluginBase::add_parameter(const std::string& name, CParameter *param)
{
	CParamList::PParameter p(param);
	if ( m_parameters.has_key(name) ) {
		stringstream errmsg;
		errmsg << get_name() << ": Parameter name '" << name << "' already in use'";
		throw invalid_argument(errmsg.str());
	}
	m_parameters[name] = p;
}


void CPluginBase::set_parameters(const CParsedOptions& options)
{
	try {
		m_parameters.set(options);
	}
	catch (invalid_argument& x) {
		stringstream errmsg;
		errmsg << get_name() << ":" << x.what();
		throw invalid_argument(errmsg.str());
	}
}

void CPluginBase::check_parameters()
{
	try {
		m_parameters.check_required();
	}
	catch (invalid_argument& x) {
		stringstream errmsg;
		errmsg << get_name() << ":" << x.what();
		throw invalid_argument(errmsg.str());
	}
}

void CPluginBase::add_dependend_handlers(HandlerHelpMap& handler_map)
{
	TRACE_FUNCTION; 
	cvdebug() << "Add dependend handler for plugin '" << m_name << "'\n"; 
	m_parameters.add_dependend_handlers(handler_map); 
}

const char *CPluginBase::get_name() const
{
	return m_name;
}


const std::string CPluginBase::get_descr() const
{
	return do_get_descr();
}


void CPluginBase::get_short_help(std::ostream& os) const
{
	os << get_name() << ": " << get_descr() << "\n";
	m_parameters.print_help(os);
	os  << "\n";
}

void CPluginBase::get_help(std::ostream& os) const
{
	m_parameters.print_help(os);
}

void CPluginBase::get_help_xml(xmlpp::Element& root) const
{
	cvdebug() << "Get help for " << m_name << "\n"; 
	root.set_child_text(get_descr()); 
	do_get_help_xml(root); 
}

void CPluginBase::do_get_help_xml(xmlpp::Element& root) const
{
	TRACE_FUNCTION; 
	m_parameters.get_help_xml(root);
}

void CPluginBase::append_interface(CPluginBase *plugin)
{
	if (m_next_interface)
		m_next_interface->append_interface(plugin);
	else
		m_next_interface = plugin;
}

CPluginBase *CPluginBase::next_interface()
{
	return m_next_interface;
}

bool CPluginBase::has_property(const char *property) const
{
	return m_properties.has(property);
}

void CPluginBase::add_property(const char *property)
{
	m_properties.add(property);
}

void CPluginBase::set_module(const PPluginModule& module)
{
	m_module = module;
}

PPluginModule CPluginBase::get_module() const
{
	return m_module;
}

const char *g_plugin_root = nullptr; 

PrepareTestPluginPath::PrepareTestPluginPath()
{
	g_plugin_root = MIA_BUILD_ROOT "/plugintest/" PLUGIN_INSTALL_PATH; 
}

PrepareTestPluginPath::~PrepareTestPluginPath()
{
	g_plugin_root = nullptr; 
}

#ifdef WIN32

EXPORT_CORE const string get_plugin_root()
{
	static string result;
	static bool result_valid = false;
	if (!result_valid) {
		LONG retval;
		LONG bsize = 0;
		HKEY hkey;
		string subkey = string("SOFTWARE\\") + string(VENDOR) + string("\\") + string(PACKAGE);
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

const string EXPORT_CORE get_plugin_root()
{
	// this is the override for tests
	if (g_plugin_root) 
		return string(g_plugin_root); 

	// fixme: this should also go into some config file
	char *plugin_root = getenv("MIA_PLUGIN_TESTPATH"); 
	if (plugin_root) 
		return string(plugin_root); 
	return string(PLUGIN_SEARCH_PATH);
}

#endif

NS_MIA_END


