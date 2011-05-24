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

#ifndef ph_handler_hh
#define ph_handler_hh

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <boost/any.hpp>
#include <mia/core/shared_ptr.hh>
#include <boost/filesystem/path.hpp>
#include <boost/utility.hpp>
#include <mia/core/utils.hh>

#include <mia/core/defines.hh>
#include <mia/core/module.hh>
#include <mia/core/plugin_base.hh>

#include <mia/core/import_handler.hh>

#ifdef BOOST_MUTEX
#include <boost/thread/mutex.hpp>
typedef boost::mutex CMutex; 
typedef boost::mutex::scoped_lock CScopedLock; 
#else
#include <tbb/mutex.h>
typedef tbb::mutex CMutex; 
typedef tbb::mutex::scoped_lock CScopedLock; 
#endif


NS_MIA_BEGIN


/** \brief The basic %template of all plugin handlers

    The template TPluginHandler provides the base interface to all plug-in handlers. 
    \tparam I the plugin interface derived from \sa CPluginBase.
*/

template <class I> 
class  EXPORT_HANDLER TPluginHandler: boost::noncopyable {
public: 
	/// typedef for the plug-in interface provided by the class 
	typedef I Interface; 

	/// a map containing the names and theavailabe plug-ins 
	typedef std::map<std::string, Interface*> CPluginMap; 

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

	/**
	   Print out the help to an output stream 
	   @param os
	 */
	void print_help(std::ostream& os) const; 
	
	
	/// \returns an iterator to the plug-ins
	const_iterator begin()const; 
	
	/// \returns the behind-end  iterator to the plug-ins
	const_iterator end()const; 

protected: 
	//! \name Constructors
        //@{

        /*! \brief Initializes the plugin handler based on the build-in search path
	  
	*/

	TPluginHandler(const std::list<boost::filesystem::path>& searchpath); 
        //@}

	/** find a plugin by name. If the plug-in is not available, the method throws an 
            \a invalid_argument exception. 
	    \param plugin name of the plugin 
	    \returns pointer to the plug-in
	*/
	typename TPluginHandler<I>::Interface *plugin(const char *plugin) const;


	/**
	   Add a given plug-in to the handler 
	   @param plugin 
	 */
	void add_plugin(Interface *plugin); 


private: 
	void global_searchpath(list< ::boost::filesystem::path>& searchpath); 
	void initialise(const std::list< ::boost::filesystem::path>& searchpath); 
	
	std::vector<PPluginModule> m_modules;
	CPluginMap m_plugins; 
	
}; 


/**
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
	static void set_search_path(const std::list<boost::filesystem::path>& searchpath);
	
	/// The instance of the plugin handler 
	typedef T Instance;

	/// iterator to iterator over the actual plug-ins 
	typedef typename T::const_iterator const_iterator;

	/// the name,plug-in pair \remark why do I need this 
	typedef typename T::CPluginMap::value_type value_type;
	
	/// \returns a reference to the only instance of the plugin handler 
	static const T& instance(); 
protected:
	/** initialize the handler singleton with a specific plugin search path 
	    (used for running tests) 
	    \param searchpath
	    \remark why not private?  
	*/
	
	THandlerSingleton(const std::list<boost::filesystem::path>& searchpath); 
	THandlerSingleton(); 
	static CMutex m_creation_mutex; 
private: 
	static std::list<boost::filesystem::path> m_searchpath; 
	static bool m_is_created; 
	
}; 


NS_MIA_END

#endif
