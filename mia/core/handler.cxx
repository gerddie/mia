/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iterator>
#include <climits> 

#include <boost/filesystem/operations.hpp>

#include <mia/core/module.hh>
#include <mia/core/plugin_base.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/xmlinterface.hh>




#include <config.h>


NS_MIA_BEGIN

namespace bfs = ::boost::filesystem; 

EXPORT_CORE const std::string get_plugin_root(); 

template <typename I> 
TPluginHandler<I>::TPluginHandler():
	CPluginHandlerBase(I::PlugData::data_descr, I::PlugType::type_descr)
{
	TRACE_FUNCTION; 
}

template <typename I>
void TPluginHandler<I>::initialise(const CPluginSearchpath& searchpath)
{
	TRACE_FUNCTION; 

	m_modules = searchpath.find_modules(I::get_data_path_part(), I::get_type_path_part()); 
	

	// now try to load the interfaces and put them in the map
	for (auto i = m_modules.begin(); i != m_modules.end(); ++i) {
		try {
			CPluginBase *pp = (*i)->get_interface();
			if (!pp) 
				cverr() << "Module '" << (*i)->get_name() << "' doesn't provide an interface\n"; 
			
			while (pp) {
				cvdebug() << "Got type '" << typeid(*pp).name() 
					  << "', expect '"<< typeid(Interface).name() << "'\n"; 
				Interface *p = dynamic_cast<Interface*>(pp); 
				CPluginBase *pold = pp; 
				
				pp = pp->next_interface(); 
				
				if (p) {
					cvdebug() << "add plugin '" << p->get_name() << "'\n"; 

					auto loaded_plugin_i = m_plugins.find(p->get_name()); 
					if ( loaded_plugin_i ==  m_plugins.end()) {
						p->set_module(*i);
						add_plugin_internal(PInterface(p)); 

						// since this module will be used 
						// keep its module till the final cleanup 
						(*i)->set_keep_library(); 
					} else {
						auto loaded_plugin = loaded_plugin_i->second; 
						if (loaded_plugin->get_priority() < p->get_priority()) {
							cvwarn() << "Plugin with name '" << p->get_name() 
								 << "' and priority '" <<  p->get_priority() 
								 << "' overrides already loaded plugin\n"; 
							p->set_module(*i);
							
							// since this module will be used 
							// keep its module till the final cleanup 
							(*i)->set_keep_library(); 
								
							*i = loaded_plugin->get_module(); 
							add_plugin_internal(PInterface(p)); 
						}else{
							cvwarn() << "Plugin with name '" << p->get_name() 
								 << "' already loaded, and new one has no higher priority.\n"; 
							delete p;
						}
						
						// since this module will not be used at all 
						// unload the according library 
						(*i)->set_unload_library(); 
					}


					

					
				}else {
					cvdebug() << "discard '" << pold->get_name() << "'\n"; 
					delete pold; 
				}
			}
		}
		catch(std::invalid_argument& x) {
			cvdebug() << "Module '" << (*i)->get_name() 
				  << "' was not loaded because '" 
				  << x.what() << "'\n"; 
		}
	}
	do_initialise(); 
}

template <typename I>
bool TPluginHandler<I>::add_plugin(PInterface p)
{
	bool result = true; 
	auto loaded_plugin_i = m_plugins.find(p->get_name()); 
	if ( loaded_plugin_i ==  m_plugins.end()) {
		add_plugin_internal(p); 
	}else{
		auto loaded_plugin = loaded_plugin_i->second; 
		if (loaded_plugin->get_priority() < p->get_priority()) {
			cvinfo() << "Plugin with name '" << p->get_name() 
				 << "' and priority '" <<  p->get_priority() 
				 << "' overrides already loaded plugin with priority '"
				 << loaded_plugin->get_priority() << "'\n"; 
			add_plugin_internal(p);

			// if the plug-in was inported from a module, mark this module for removal
			auto old_module = loaded_plugin->get_module(); 
			if (old_module)
				old_module->set_unload_library(); 
			
		}else{
			cvinfo() << "Plugin with name '" << p->get_name() 
				 << "' has lower or equal priority '" <<  p->get_priority() 
				 << "' compared to already loaded plugin with priority '"
				 << loaded_plugin->get_priority() << "' ... discarding\n"; 
			result = false; 
		}
	}
	return result; 
}


template <typename I>
void TPluginHandler<I>::add_plugin_internal(PInterface p)
{
	
	m_plugins[p->get_name()] = p;	
}

template <typename I>
void TPluginHandler<I>::do_add_dependend_handlers(HandlerHelpMap& handler_map)const 
{
	for (auto p = begin(); p != end(); ++p)
		p->second->add_dependend_handlers(handler_map); 
}

template <typename I>
typename TPluginHandler<I>::const_iterator TPluginHandler<I>::begin() const
{
	return m_plugins.begin(); 
}

template <typename I>
typename TPluginHandler<I>::const_iterator TPluginHandler<I>::end() const
{
	return m_plugins.end(); 
}

/**
   The destructor frees the modules and plug-ins 
*/
template <typename I>
TPluginHandler<I>::~TPluginHandler()
{
	
}


template <typename I>
size_t TPluginHandler<I>::size() const
{
	return m_plugins.size(); 
}

template <typename I>
const std::string TPluginHandler<I>::get_plugin_names() const
{
	std::vector<std::string> names;  

	for (auto i = m_plugins.begin(); i != m_plugins.end(); ++i)
		names.push_back(i->first);

	sort(names.begin(), names.end()); 
	std::stringstream outstr; 

	std::copy(names.begin(), names.end(), std::ostream_iterator<const std::string>(outstr, " "));
	
	return outstr.str(); 
}

template <typename I>
const std::set<std::string> TPluginHandler<I>::get_set() const
{
	std::set<std::string> r; 
	for (auto i = m_plugins.begin(); i != m_plugins.end(); ++i)
		r.insert(i->first);
	return r; 
}

template <typename I>
typename TPluginHandler<I>::Interface *TPluginHandler<I>::plugin(const char *plugin) const 
{
	auto p = m_plugins.find(plugin); 
	if (p == m_plugins.end()) {
		std::stringstream msg; 
		cvdebug() << "Plugin '" << plugin << "' not found in '" 
		    <<  I::PlugData::data_descr << "/" << I::PlugType::type_descr << "'\n"
		    << " With search path\n"
		    << "    '" << get_plugin_root(); 
		return nullptr; 
	}
	return p->second.get(); 
}

template <typename I>
void TPluginHandler<I>::do_print_short_help(std::ostream& os) const
{
	for (auto i = begin(); i != end(); ++i) {
		i->second->get_short_help(os);
	}
}

template <typename I>
void TPluginHandler<I>::do_print_help(std::ostream& os) const
{
	for (auto i = begin(); i != end(); ++i) {
		i->second->get_help(os);
	}
}

template <typename I>
void TPluginHandler<I>::do_get_xml_help(CXMLElement& handlerRoot) const
{
	handlerRoot.set_child_text(m_help);
	for (auto i = begin(); i != end(); ++i) {
		auto pluginRoot = handlerRoot.add_child("plugin");
		pluginRoot->set_attribute("name", i->first);
		i->second->get_help_xml(*pluginRoot); 
	}
}

template <typename I>
void TPluginHandler<I>::do_initialise()
{
}

template <typename T> 
THandlerSingleton<T>::THandlerSingleton()
{
	TRACE_FUNCTION; 
	assert(!m_is_created); 
	m_is_created = true; 
}

template <typename T> 
const T& THandlerSingleton<T>::instance()
{
	return do_instance(true);
}

template <typename T> 
const T* THandlerSingleton<T>::pointer()
{
	return &do_instance(false);
}

template <typename T> 
bool THandlerSingleton<T>::add_plugin(typename T::PInterface p)
{
	return do_instance(true).add_plugin(p); 
} 

template <typename T> 
T& THandlerSingleton<T>::do_instance(bool require_initialization)
{
	TRACE_FUNCTION; 
	CScopedLock lock(m_creation_mutex); 
	static THandlerSingleton<T> me;
	cvdebug() << "m_is_initialized = " << m_is_initialized << "\n";
	
	if (require_initialization) {
		if (!m_is_initialized) {
			TRACE("Uninitialized state"); 
			CScopedLock lock_init(m_initialization_mutex);
			if (!m_is_initialized) {
				TRACE("Enter locked uninitialized state"); 
				lock.release(); 
				cvdebug() << "not yet initialized: second check passed\n"; 
				me.initialise(m_searchpath);
				m_is_initialized = true; 
				
			}
		}
	}
	return me; 
}

template <typename T>
void THandlerSingleton<T>::set_search_path(const CPluginSearchpath& searchpath)
{
	TRACE_FUNCTION; 

	CScopedLock lock(m_creation_mutex); 
	cvdebug() << "Set path: "
		  << T::Interface::get_data_path_part() << "/"
		  << T::Interface::get_type_path_part() << '\n'; 

	if (m_is_created) {
		cvinfo() << "THandlerSingleton<" 
			 << T::Interface::get_data_path_part() << "/"
			 << T::Interface::get_type_path_part()
			 << ">::set_search_path: handler was already created\n"; 
		
	}
//	cvdebug() << "searchpath=" << searchpath << "\n"; 
	m_searchpath = searchpath; 

}


template <typename T>
CPluginSearchpath THandlerSingleton<T>::m_searchpath;

template <typename T>
bool THandlerSingleton<T>::m_is_created = false; 

template <typename T>
std::atomic<bool> THandlerSingleton<T>::m_is_initialized(false); 

template <typename T>
CMutex THandlerSingleton<T>::m_creation_mutex; 

template <typename T>
CMutex THandlerSingleton<T>::m_initialization_mutex; 


NS_MIA_END
