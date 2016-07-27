/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef ph_handler_hh
#define ph_handler_hh

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <atomic>
#include <boost/any.hpp>

#include <mia/core/utils.hh>
#include <mia/core/filetools.hh>

#include <mia/core/defines.hh>
#include <mia/core/parallel.hh>
#include <mia/core/module.hh>
#include <mia/core/plugin_base.hh>
#include <mia/core/handlerbase.hh>
#include <mia/core/searchpath.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN


/** 
    \ingroup plugin
    
    \brief The basic %template of all plugin handlers

    The template TPluginHandler provides the base interface to all plug-in handlers. 
    \tparam I the plugin interface derived from \sa CPluginBase.
*/

template <typename I> 
class  EXPORT_HANDLER TPluginHandler: public CPluginHandlerBase {
public: 
	/// typedef for the plug-in interface provided by the class 
	typedef I Interface; 

	typedef std::shared_ptr<I> PInterface; 

	/// a map containing the names and the available plug-ins 
	typedef std::map<std::string, PInterface> CPluginMap; 

	/// the iterator to walk over the available plug-ins 
	typedef typename CPluginMap::const_iterator const_iterator; 
	

	/**
	   The destructor frees the modules and plug-ins 
	*/
	virtual ~TPluginHandler(); 

	/// \returns the number of available plug-ins
	size_t size() const; 

	/** \returns the names of the plug-ins as a space delimited string, 
	    ordered case sensitive and alphabetically. 
	*/
	const std::string get_plugin_names() const; 

	/** \returns the names of the plug-ins as a set */
	const std::set<std::string> get_set() const; 

	
	/// \returns an iterator to the plug-ins
	const_iterator begin()const; 
	
	/// \returns the behind-end  iterator to the plug-ins
	const_iterator end()const; 

	/**
	   Add a given plug-in to the handler. 
	   @param plugin 
	   @returns true if the plug-in was added, false if a plug-in with a higher or equal 
	   priority and the same name already existed. 
	*/
	bool add_plugin(PInterface plugin); 

protected: 


	/**
	   Add a given plug-in to the handler. The pointer must not be freed in client code. 
	   @param plugin 
	*/
	void add_plugin_internal(PInterface plugin); 


	//! \name Constructors
        //@{

        /*! \brief Initializes the plugin handler based on the build-in search path
	  
	*/

	TPluginHandler(); 
        //@}

	/** find a plugin by name. If the plug-in is not available, the method throws an 
            \a invalid_argument exception. 
	    \param plugin name of the plugin 
	    \returns pointer to the plug-in
	*/
	typename TPluginHandler<I>::Interface *plugin(const char *plugin) const;


	void initialise(const CPluginSearchpath& searchpath); 

private: 
	virtual void do_initialise(); 

	void do_add_dependend_handlers(HandlerHelpMap& handler_map) const; 	
	
	std::vector<PPluginModule> m_modules;
	CPluginMap m_plugins; 

	virtual void do_print_short_help(std::ostream& os) const; 
	virtual void do_print_help(std::ostream& os) const; 
	virtual void do_get_xml_help(CXMLElement& root) const; 	

	static const char * const m_help; 

}; 


/**
   \ingroup plugin
   
   \brief the singleton that a plug-in handler really is 
   
   Class to make a singleton out of a plugin handler by deriving from it.
*/

template <typename T>
class EXPORT_HANDLER THandlerSingleton : public T {
public:

	/**
	   Set the plugin search path for the plug-in - throws "runtime_error" if the 
	   plugin handler is already instanciated. 
	 */
	static void set_search_path(const CPluginSearchpath& searchpath);
	
	/// The instance of the plugin handler 
	typedef T Instance;

	/// iterator to iterator over the actual plug-ins 
	typedef typename T::const_iterator const_iterator;

	/// the name,plug-in pair \remark why do I need this 
	typedef typename T::CPluginMap::value_type value_type;
	
	/**
	   \returns a reference to the only instance of the plugin handler 
	*/
	static const T& instance(); 

	static bool add_plugin(typename T::PInterface p); 

	/**
	   \returns a pointer to the only instance of the plugin handler, it is possible that 
	   this instance is not yet initialized.   
	*/
	static const T* pointer(); 
protected:

	/** This mutex ensures that each Singleton is indeed only created once and 
	    no race condition happens within a multi-threaded environmnet */ 
	static CMutex m_creation_mutex; 

private: 
	// the constructor is private because you must not derive the singleton
	// derive the handler if you need specific funcionality, and then 
	// template the singleton with the derived handler. 
	THandlerSingleton(); 

	static T& do_instance(bool require_initialization); 

	static CPluginSearchpath m_searchpath; 
	static bool m_is_created; 
	static CMutex m_initialization_mutex;
	static std::atomic<bool> m_is_initialized; 
	
}; 


NS_MIA_END

#endif
