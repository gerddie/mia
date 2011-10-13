/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iterator>
#include <climits> 

#include <boost/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <mia/core/module.hh>
#include <mia/core/plugin_base.hh>
#include <mia/core/msgstream.hh>


#include <config.h>


NS_MIA_BEGIN

namespace bfs = ::boost::filesystem; 

using namespace std; 

EXPORT_CORE const std::string get_plugin_root(); 

template <typename I> 
TPluginHandler<I>::TPluginHandler(const std::list<bfs::path>& searchpath)
{
	if (!searchpath.empty())
		initialise(searchpath); 
	else{
		std::list<bfs::path> gsearchpath; 
		global_searchpath(gsearchpath); 
		initialise(gsearchpath); 
	}
}

template <typename I>
void TPluginHandler<I>::global_searchpath(list<bfs::path>& searchpath)
{
	bfs::path type_path = TPlugin<typename I::PlugData,typename I::PlugType>::search_path();
	
	cvdebug() << "Add plugin searchpath\n"; 
	cvdebug() << "   " << get_plugin_root() << "/" << type_path.string() << "\n"; 

	searchpath.push_back( bfs::path(get_plugin_root()) / type_path); 


#ifdef PLUGIN_HOME_SEARCH_PATH 
	char *c_home = getenv("HOME"); 
	if (c_home) {

		cvdebug() << "   " << c_home << "/" << PLUGIN_HOME_SEARCH_PATH 
			  << "/" << type_path.native_file_string() << "\n"; 

		searchpath.push_back( bfs::path(c_home, bfs::native) / bfs::path(PLUGIN_HOME_SEARCH_PATH) 
				      / type_path); 
	}
#endif

	char *c_user = getenv("MIA_PLUGIN_PATH");
	if (c_user) {
		cvdebug() << "   " << c_user << "/" << type_path.string() << "\n"; 
		bfs::path lala(c_user); 
		bfs::path subdir = lala / type_path; 
		
		searchpath.push_back( subdir ); 
	}	
}

template <typename I>
void TPluginHandler<I>::initialise(const list<bfs::path>& searchpath)
{
	// create the pattern match
	stringstream pattern; 

	pattern << ".*\\."<< MIA_MODULE_SUFFIX << "$"; 

	cvdebug() << "TPluginHandler<I>::initialise: '"<<
		TPlugin<typename I::PlugData,typename I::PlugType>::search_path() <<
		"' using search pattern'" << pattern.str() << "'\n"; 
	boost::regex pat_expr(pattern.str());	
	vector<bfs::path> candidates; 

	// search through all the path to find the plugins
	for (list<bfs::path>::const_iterator dir = searchpath.begin(); 
	     dir != searchpath.end(); ++dir){
		cvdebug() << "Looking for " << dir->string() << "\n"; 
		if (bfs::exists(*dir) && bfs::is_directory(*dir)) {
			// if we cant save the old directory something is terribly wrong
			bfs::directory_iterator di(*dir); 
			bfs::directory_iterator dend;
			
			cvdebug() << "TPluginHandler<I>::initialise: scan '"<<dir->string() <<"'\n"; 

			while (di != dend) {
				cvdebug() << "    candidate:'" << di->path().string() << "'"; 
				if (boost::regex_match(di->path().string(), pat_expr)) {
					candidates.push_back(*di); 
					cverb << " add\n";
				}else
					cverb << " discard\n";
				++di; 
			}
		}
	}
	
	// candidates contains all the names of the possible plug-ins
	// now load the modules and put them in a list
	for (vector<bfs::path>::const_iterator i = candidates.begin(); 
	     i != candidates.end(); ++i) {
		try {
			cvdebug()<< " Load '" <<i->string()<<"'\n"; 
			m_modules.push_back(PPluginModule(new CPluginModule(i->string().c_str())));
		}
		catch (invalid_argument& ex) {
			cverr() << ex.what() << "\n"; 
		}
		catch (std::exception& ex) {
			cverr() << ex.what() << "\n"; 
		}
		catch (...) {
			cverr() << "Loading module " << i->string() << "failed for unknown reasons\n"; 
		}
	}

	// now try to load the interfaces and put them in the map
	for (vector<PPluginModule>::const_iterator i = m_modules.begin(); 
	     i != m_modules.end(); ++i) {
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
					if (m_plugins.find(p->get_name()) ==  m_plugins.end()) {
						p->set_module(*i);
						add_plugin(p); 
					} else {
						cvwarn() << "Plugin with name '" << p->get_name() 
							 << "' already loaded, ignoring new one.\n"; 
						delete p; 
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
}

template <typename I>
void TPluginHandler<I>::add_plugin(Interface *p)
{
	m_plugins[p->get_name()] = p;	
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
	for (typename CPluginMap::iterator i = m_plugins.begin(); 
	     i != m_plugins.end(); ++i) {
		delete i->second; 
	}
}


template <typename I>
size_t TPluginHandler<I>::size() const
{
	return m_plugins.size(); 
}

template <typename I>
const string TPluginHandler<I>::get_plugin_names() const
{
	vector<string> names;  

	for (typename map<string, Interface*>::const_iterator i = m_plugins.begin(); 
	     i != m_plugins.end(); ++i)
		names.push_back(i->first);

	sort(names.begin(), names.end()); 
	stringstream outstr; 

	copy(names.begin(), names.end(), ostream_iterator<const string>(outstr, " "));
	
	return outstr.str(); 
}

template <typename I>
const std::set<std::string> TPluginHandler<I>::get_set() const
{
	std::set<std::string> r; 
	for (typename map<string, Interface*>::const_iterator i = m_plugins.begin(); 
	     i != m_plugins.end(); ++i)
		r.insert(i->first);
	return r; 
}

template <typename I>
typename TPluginHandler<I>::Interface *TPluginHandler<I>::plugin(const char *plugin) const 
{
	typename map<string, Interface*>::const_iterator p = m_plugins.find(plugin); 
	if (p == m_plugins.end()) {
		stringstream msg; 
		msg << "Plugin '" << plugin << "' not found in '" 
		    << I::PlugType::type_descr << "/" <<  I::PlugData::data_descr << "'\n"
		    << " With search path\n"
		    << "    '" << get_plugin_root(); 
		throw invalid_argument(msg.str()); 
	}
	return p->second; 
}

template <typename I>
void TPluginHandler<I>::do_print_short_help(std::ostream& os) const
{
	for (const_iterator i = begin(); i != end(); ++i) {
		i->second->get_short_help(os);
	}
}

template <typename I>
void TPluginHandler<I>::do_print_help(std::ostream& os) const
{
	for (const_iterator i = begin(); i != end(); ++i) {
		i->second->get_help(os);
	}
}


template <typename T> 
THandlerSingleton<T>::THandlerSingleton(const std::list<boost::filesystem::path>& searchpath):
	T(searchpath) 
{
	TRACE_FUNCTION; 
	assert(!m_is_created); 
	m_is_created = true; 
}

template <typename T> 
THandlerSingleton<T>::THandlerSingleton():
	T(m_searchpath)
{
	TRACE_FUNCTION; 
	assert(!m_is_created); 
	m_is_created = true; 
}

	
template <typename T> 
const T& THandlerSingleton<T>::instance()
{
	CScopedLock lock(m_creation_mutex); 
	TRACE_FUNCTION; 
	static THandlerSingleton me; 
	return me; 
}

template <typename T>
void THandlerSingleton<T>::set_search_path(const std::list<boost::filesystem::path>& searchpath)
{
	CScopedLock lock(m_creation_mutex); 
	typedef typename  T::Interface IF; 
	TRACE("THandlerSingleton<T>::set_search_path"); 
	cvdebug() << "Set path: "  << TPlugin<typename IF::PlugData,typename IF::PlugType>::search_path() << '\n'; 

	if (m_is_created) {
		bfs::path type_path = TPlugin<typename IF::PlugData,typename IF::PlugType>::search_path();
		cvinfo() << "THandlerSingleton<" << 
			type_path.string() <<
			">::set_search_path: handler was already created\n"; 
			
	}
#if 0 // work around the appearent icc bug, where the static member 
      //variable is not initialised
	::new (&m_searchpath) std::list<boost::filesystem::path>; 
#endif
	m_searchpath = searchpath; 

}


template <typename T>
std::list<boost::filesystem::path> THandlerSingleton<T>::m_searchpath;
template <typename T>
 bool THandlerSingleton<T>::m_is_created = false; 

template <typename T>
CMutex THandlerSingleton<T>::m_creation_mutex; 


NS_MIA_END
