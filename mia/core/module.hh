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

#ifndef ph_modules_hh
#define ph_modules_hh

#include <memory>
#include <string>
#include <mia/core/defines.hh>
#include <mia/core/dlloader.hh>

NS_MIA_BEGIN

/// definition of the Plugin interface loading function
class CPluginBase;

/**   
      \ingroup plugin
      C function type that is provided by the plug-ins as entry point 
*/
typedef  CPluginBase* (*FPluginInterface)(void);

/** 
   \ingroup plugin
   
   \brief  The plugin module loading class. 
   
   This class handles the loading of shared object modules and hides all the 
   ugliness of platform dependency. 
 */
class EXPORT_CORE CPluginModule {
public:
        /**
	   Constructor that takes the path to the module, tries to open it, and will throw
	   an \a invalid_argument exception, if the module can not be loaded
	*/
	CPluginModule(const char *path);
	~CPluginModule();

	/**
	   \returns the plug-in object implemented in the module or NULL, if the module does not
	   proide such plug-in
	*/
	CPluginBase *get_interface() const;

	///\returns the file name of the module
	const std::string& get_name() const; 

	/** tell the DLL loader that it should unload this library
	    when the modules is destroyed */ 
	void set_unload_library(); 

	void set_keep_library(); 
private:
	CDLLoader m_loader;
	std::string m_name;
};

/** 
    \ingroup plugin
    Shared pointer type of the CPluginModule 
*/
typedef std::shared_ptr<CPluginModule > PPluginModule;

NS_MIA_END

#endif
