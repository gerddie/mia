/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <config.h>

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
	_M_name(name),
	_M_next_interface(NULL)
{
}

CPluginBase::~CPluginBase()
{
	TRACE("CPluginBase::~CPluginBase()");
}

void CPluginBase::add_parameter(const std::string& name, CParameter *param)
{
	CParamList::PParameter p(param);
	if ( _M_parameters.has_key(name) ) {
		stringstream errmsg;
		errmsg << get_name() << ": Parameter name '" << name << "' already in use'";
		throw invalid_argument(errmsg.str());
	}
	_M_parameters[name] = p;
}


void CPluginBase::set_parameters(const CParsedOptions& options)
{
	try {
		_M_parameters.set(options);
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
		_M_parameters.check_required();
	}
	catch (invalid_argument& x) {
		stringstream errmsg;
		errmsg << get_name() << ":" << x.what();
		throw invalid_argument(errmsg.str());
	}
}


const char *CPluginBase::get_name() const
{
	return _M_name;
}


const std::string CPluginBase::get_descr() const
{
	return do_get_descr();
}


void CPluginBase::get_help(std::ostream& os) const
{
	_M_parameters.print_help(os);
}


void CPluginBase::append_interface(CPluginBase *plugin)
{
	if (_M_next_interface)
		_M_next_interface->append_interface(plugin);
	else
		_M_next_interface = plugin;
}

bool CPluginBase::test(bool uninstalled) const
{
	TRACE("CPluginBase::test");
	if (uninstalled)
		prepare_path();
	return do_test();
}

void CPluginBase::prepare_path() const
{
	// normally empty
}

CPluginBase *CPluginBase::next_interface()
{
	return _M_next_interface;
}

bool CPluginBase::has_property(const char *property) const
{
	return _M_properties.has(property);
}

void CPluginBase::add_property(const char *property)
{
	_M_properties.add(property);
}

void CPluginBase::set_module(const PPluginModule& module)
{
	_M_module = module;
}

PPluginModule CPluginBase::get_module() const
{
	return _M_module;
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
	// fixme: this should also go into some config file
	return string(PLUGIN_SEARCH_PATH);
}
#endif

NS_MIA_END


