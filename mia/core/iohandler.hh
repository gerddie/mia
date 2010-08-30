/* -*- mona-c++  -*-
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

#ifndef ph_IOhandler_hh
#define ph_IOhandler_hh
#include <set>

#include <mia/core/handler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/delayedparameter.hh>

NS_MIA_BEGIN

/**
   Input/Output plugin handler base class, derived privately from the 
   standart plug-in handler to hide its interface.
   Class \a I, the interface class needs to be derived from \a TIOPlugin.  
*/

template <class I> 
class EXPORT_HANDLER TIOPluginHandler: public TPluginHandler<I> {
public:

	typedef typename I::Data Data; 
	typedef typename std::shared_ptr<Data > PData; 
	typedef typename TPluginHandler<I>::Interface Interface; 
	typedef typename TPluginHandler<I>::const_iterator const_iterator; 
	typedef std::multimap<std::string, std::string> CSuffixmap; 
	
	typedef TDelayedParameter<PData> DataKey; 
	
	/**
	   Load data from a file. The suffix of \a fname is used to derive a preffered plug-in, 
	   which is then tried first. If this fails, all other plug-ins are tried as well. 
	   \param fname the file name 
	   \returns a shared pointer to the loaded data or an empty shared pointer 
	   if no data could be loaded. 
	 */
	PData load(const std::string& fname) const; 

	/**
	   Load data from a file into the data pool, the key is the filename. 
	   The suffix of \a fname is used to derive a preffered plug-in, 
	   which is then tried first. If this fails, all other plug-ins are tried as well. 
	   \param fname the file name 
	   throws a std::runtime_error if loading fails 
	 */

	DataKey load_to_pool(const std::string& fname) const; 

	/**
	   Save the data to a file. If type is empty, then the output plugin is derived from 
           the fname suffix. if this fails, saving fails. If type is provided, the output plugin 
           is selected accordingly. 
	   \param type output plugin name - leave empty to derive from fname suffix
	   \param fname output file name 
	   \param data the data to be saved
	   \returns true if saving is successfull, false otherwise. 
	 */
	bool save(const std::string& type, const std::string& fname, const Data& data) const;

	/** 
	    \param fname a file name 
	    \returns the plug-in that is preferred for the suffix of \a fname 
	*/
	const Interface *prefered_plugin_ptr(const std::string& fname) const; 

	const Interface& prefered_plugin(const std::string& fname) const; 

	const CSuffixmap& get_supported_filetype_map() const; 

protected: 
	/**
	   constructor that is provided with a list of plugin search path. 
	 */
	TIOPluginHandler(const std::list< ::boost::filesystem::path>& searchpath); 

private: 	
	// a map of plugins 

	CSuffixmap _M_suffixmap; 

	// list of supported compressd file suffixes
	std::set<std::string> _M_compress_sfx; 

	/**
	   Private plugin to handle the virtual data pool IO  
	 */
	class EXPORT_HANDLER CDatapoolPlugin : public Interface {
	public: 
		CDatapoolPlugin(); 
	private: 
		void do_add_suffixes(typename TIOPluginHandler<I>::CSuffixmap& map) const; 
		PData do_load(const std::string& fname) const;
		bool do_save(const std::string& fname, 
			     const typename Interface::Data& data) const; 
		const std::string do_get_descr() const;
	}; 
	CDatapoolPlugin *_M_pool_plugin; 
}; 

template <typename T>
class EXPORT_HANDLER TIOHandlerSingleton : public THandlerSingleton<T> {
public: 
	typedef typename T::CSuffixmap CSuffixmap; 
	TIOHandlerSingleton(const std::list<boost::filesystem::path>& searchpath):
		THandlerSingleton<T>(searchpath)
		{
		}

	TIOHandlerSingleton()
		{
		}

	/// \returns a reference to the only instance of the plugin handler 
	static const T& instance(); 

}; 

NS_MIA_END
#endif
