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

#ifndef mia_core_handlerbase_hh
#define mia_core_handlerbase_hh

#include <ostream>
#include <fstream>
#include <map>
#include <mia/core/xmlinterface.hh>

#include <mia/core/defines.hh>

NS_MIA_BEGIN




class CPluginHandlerBase; 

/**
   \ingroup plugin
   \brief A map that is used to collect the plug-in handlers used in a program 
   
   This map is used to collect that plug-in handlers that are used by the 
   TFactoryParameter in order to obtain a list of plug.in dependecies for the 
   program help. 
*/
typedef std::map<std::string, const CPluginHandlerBase*> HandlerHelpMap; 

/** 
   \ingroup plugin
   
   \brief The base class for all plugin handlers
   
   This clas provides some basic funcionallity that is common to all plugin handlers. 
   In addition, the copy constructor and assigment operator are deleted in order to forbit copying 
   of instances of this class. 
*/
class EXPORT_CORE CPluginHandlerBase  {
 public: 

	/**
	   Constructor 
	   \param descriptor plugin search  path descriptor, used to identify the plugin type for users
	*/
	CPluginHandlerBase(const std::string& descriptor); 

	/**
	   forbid copying - doxygen should drop these from the documentation 
	*/
	CPluginHandlerBase(const CPluginHandlerBase& other) = delete; 

	/**
	   forbid copying - doxygen should drop these from the documentation 
	*/
	CPluginHandlerBase& operator  = (const CPluginHandlerBase& other) = delete;

	virtual ~CPluginHandlerBase();

	/**
	   Print out the short help to an output stream 
	   @param os
	*/
	void print_short_help(std::ostream& os) const; 

	/**
	   Print out the help to an output stream 
	   @param os
	 */
	void print_help(std::ostream& os) const; 

	/**
	   Add the help for all plug-ins to the xml tree 
	   \param root toot element the documentation is added to 
	 */
	void get_xml_help(CXMLElement *root) const; 

	/// \returns the plug-in handler descriptor
	const std::string& get_descriptor() const; 

	/** add all plug-in handler that may be called by this plug-in handler
	    \param [in,out] handler_map plug in handler map to add to 
	 */
	void add_dependend_handlers(HandlerHelpMap& handler_map) const;  

	/**
	   write the XML description of this handler to an XML node 
	   @param os stream to write additional descriptions to. 
	   @param root the parent node the information is added to 
	*/
	void get_string_help_description_xml(std::ostream& os, CXMLElement *root) const; 
	
	
	/**
	   @returns the type name of the plug-in handler 
	*/
	std::string get_handler_type_string() const; 
	
	
	/**	  
	   Validate the given string to see whether anything useful can be done 
	   with it. The actual way how this is validated depends on the plug-in
           type.  
	 */
	bool validate_parameter_string(const std::string& s) const; 

private: 
	
	virtual void do_print_short_help(std::ostream& os) const = 0; 
	virtual void do_print_help(std::ostream& os) const = 0; 
	virtual void do_get_xml_help(CXMLElement *root) const = 0; 
	virtual void do_add_dependend_handlers(HandlerHelpMap& handler_map) const = 0;  
	virtual std::string get_handler_type_string_and_help(std::ostream& os) const; 
	virtual std::string do_get_handler_type_string() const; 
	virtual bool do_validate_parameter_string(const std::string& s) const; 

	std::string m_descriptor; 
}; 




NS_MIA_END


#endif
