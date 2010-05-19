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


#ifndef __plugin_base_hh
#define __plugin_base_hh

#include <map>
#include <set>
#include <string>
#include <ostream>

#include <mia/core/shared_ptr.hh>
#include <boost/filesystem/path.hpp>

#include <mia/core/defines.hh>
#include <mia/core/module.hh>
#include <mia/core/optparam.hh>

#ifndef EXPORT_HANDLER
#ifdef WIN32
#define EXPORT_HANDLER __declspec(dllimport)
#else
#define EXPORT_HANDLER
#endif
#endif

NS_MIA_BEGIN

EXPORT_CORE extern const char plugin_test_mode[];
EXPORT_CORE extern const std::string plugin_help;

/**
   \class CPluginBase
   The base for all plug-ins.
*/

class EXPORT_CORE CPluginBase {
public:
	/**
	   The constructor  initialises the plug-in with its name and sets
	   its link-up to nil ant initialises its parameter map.
	   \param name
	*/
	CPluginBase(const char *name);

	/** this destructur exists for the sole purpouse to ensure a virtual destructor
	    for all plug-ins.
	*/
	virtual ~CPluginBase();

	/**
	   Add a parameter to the parameter list. If the name of
	   the parameter already exists this function throws an \a
	   invalid_argument exception.
	   \param name Name of the new parameter
	   \param param the actual parameter
	*/
	void add_parameter(const std::string& name, CParameter *param);

	/**
	   Set the parameter according to the given option map. If the
	   parameter name does not exists, the function will throw an
	   \a invalid_argument exception. Depending on the parameter type
	   setting it might also throw an \a invalid_argument exception.
	   \param options the options map
	*/
	void set_parameters(const CParsedOptions& options);

	/**
	   This function checks, whether all requzired parameters have really been set.
	   It throws an \a invalid_argument
	*/
	void check_parameters();

	/// \returns the name of the plug-in
	const char *get_name() const;

	/// \returns the name of the plug-in
	const std::string get_descr() const;

	/** prints out a help text to the given output stream
	    \param os
	*/
	virtual void get_help(std::ostream& os) const;

	/**
	   link up another plug-in in order to be able to put several
	   plug-ins of the same type into the same dynamic library
	*/
	void append_interface(CPluginBase *plugin);

	/// \returns a pointer to the next plug-in, or NULL, if it doesn't exist
	CPluginBase *next_interface();

	/**
	   \param property a named property
	   \returns true if the property is supported, false otherwise
	*/
	bool has_property(const char *property) const;

	bool test(bool uninstalled) const;

	void set_module(const PPluginModule& module);

	PPluginModule get_module() const;
protected:
	/**
	   Add a porperty to the map of supported properties
	   \param property a named property
	*/
	void add_property(const char *property);

private:
	virtual bool do_test() const = 0;
	virtual void prepare_path() const;

	virtual const std::string do_get_descr() const = 0;

	const char *_M_name;

	CPluginBase *_M_next_interface;

	CParamList  _M_parameters;

	std::set<const char *> _M_properties;

	PPluginModule  _M_module;
};


/**
   Templated plugin base class. The classes \a D and \a T must define a
   static  const char *value.  the combination D::value/T::value
   is will be part of the plugin search path.
*/
template <typename D, typename T>
class EXPORT_HANDLER TPlugin: public  CPluginBase {
public:
	typedef D PlugData;
	typedef T PlugType;


	/**
	   The constructor  initialises the plug-in with its name and sets
	   its link-up to nil ant initialises its parameter map.
	   \param name
	*/
	TPlugin(const char *name);

	/**
	   Write a help regarding the plugin to the output stream
	   \param os
	 */
	virtual void get_help(std::ostream& os) const;

	/// \returns the plugin specific part of the plugin search path
	static ::boost::filesystem::path search_path();

	const std::string get_long_name() const;

};


NS_MIA_END

#endif
