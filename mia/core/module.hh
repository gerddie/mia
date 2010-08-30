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

#ifndef ph_modules_hh
#define ph_modules_hh

#include <string>
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>
#include <mia/core/dlloader.hh>

NS_MIA_BEGIN
using namespace std;

/// definition of the Plugin interface loading function

class CPluginBase;
typedef  CPluginBase* (*FPluginInterface)(void);

/** \class CPluginModule
    The plugin module loading class
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

private:
	CDLLoader _M_loader;
	size_t m_refs;
	std::string m_name;
};

typedef std::shared_ptr<CPluginModule > PPluginModule;

NS_MIA_END

#endif
