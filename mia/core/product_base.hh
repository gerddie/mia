/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#ifndef __mia_core_productbase_hh
#define __mia_core_productbase_hh

#include <mia/core/module.hh>
#include <mia/core/property_flags.hh>

NS_MIA_BEGIN

/**
   \ingroup infrastructure 
   \brief The base class for all plug-in created object. 
   
   Base of all objects that are created from within plug-ins using a factory method. 
   It also holds the creator string for the instance of the created object.
   \remark The pointer to the modules should take care that the module is valid until the 
   object is destroyed. 
 */

class EXPORT_CORE CProductBase: public CPropertyFlagHolder {
public:
	/**
	   Set the string used to create this object using its factory. 
	   \param init init string 
	 */
	void set_init_string(const char *init); 

	/**
	   \returns the init string used to create this instance 
	 */
	const char *get_init_string() const; 
	
	/**
	   Set the plugin module which holds the code for this object 
	   \param module
	   \remark In a perfect world, the module could be unloaded after 
	   all instances of the class using its code are destroyed. 
	   Unfortunately, the destructor still required the code after 
	   dereferencing the module. 
	*/
	void set_module(PPluginModule module);
private:
	PPluginModule m_module;
	std::string m_init_string; 
};
NS_MIA_END


#endif
