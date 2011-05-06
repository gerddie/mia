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

#ifndef ph_testplugin_hh
#define ph_testplugin_hh

#include <mia/core/plugin_base.hh>
#include <mia/core/handler.hh>

NS_MIA_BEGIN

struct test_plugin_type;
struct test_plugin_data;

/// constant defining a test property 
EXPORT_CORE extern const char *const test_property;

/**
   \brief Class to test plugin handling. 
*/

class EXPORT_CORE CTestPlugin: public TPlugin<test_plugin_data, test_plugin_type>  {
public:
	/// Construct the test plugin with the given name 
	CTestPlugin(const char *name);

	/// \returns the plugin search path for testing 
	static ::boost::filesystem::path search_path();
private:
	virtual bool do_test() const = 0;
};


/**
   \brief Class to test plugin handling. 
*/
class EXPORT_CORE CTestPluginHandlerImpl : public  TPluginHandler<CTestPlugin> {
protected:
	CTestPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath);
public:
	CTestPlugin *get_plugin(const char *name) const;
};


/// Test plugin handler only used internally for theting the plugin handler 
typedef THandlerSingleton<CTestPluginHandlerImpl> CTestPluginHandler;
NS_MIA_END


#endif
