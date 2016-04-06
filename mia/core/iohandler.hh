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

#ifndef ph_IOhandler_hh
#define ph_IOhandler_hh
#include <set>

#include <mia/core/handler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/delayedparameter.hh>

NS_MIA_BEGIN

/**
   \ingroup plugin

   \brief Template for all plug-in handlers that are responsible for data IO 
   
   Input/Output plugin handler base class, derived privately from the 
   standart plug-in handler to hide its interface.
   All IO plug-in handlers all proved a CDatapool as temporal storage to pass 
   data around without disk-io.
   \tparam I the interface class that needs to be derived from \a TIOPlugin.  
*/

template <class I> 
class EXPORT_HANDLER TIOPluginHandler: public TPluginHandler<I> {
public:

	/// Data type handled by this plug-in 
	typedef typename I::Data Data; 
	
        /// Shared pointer to the data hadnled by this plug-in 
	typedef typename std::shared_ptr<Data > PData; 
	
	/// the IO interface provided by this handler 
	typedef typename TPluginHandler<I>::Interface Interface; 
	
	/// an iterator over the availabe plug-ins 
	typedef typename TPluginHandler<I>::const_iterator const_iterator; 

	/// The map that maps file name suffixes to IO plug-ins 
	typedef std::multimap<std::string, std::string> CSuffixmap; 
	
	/// The type of the key used for the CDatapool access 
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
	   \param fname output file name 
	   \param data the data to be saved
	   \returns true if saving is successfull, false otherwise. 
	 */
	bool save(const std::string& fname, const Data& data) const;

	/** Tolerant search for the plug-in corresponding to the file name suffix
	    \param fname a file name 
	    \returns the plug-in that is preferred for the suffix of \a fname, or NULL 
	    if none was found. 
	*/
	const Interface *preferred_plugin_ptr(const std::string& fname) const; 

	/** 
	    Search for the plug-in corresponding to the file name suffix, if the 
	    search fails, an \a std::invalid_argument exception is thrown. 
	    \param fname a file name 
	    \returns the plug-in that is preferred for the suffix of \a fname 
	*/

	const Interface& preferred_plugin(const std::string& fname) const; 

	/// @returns the a mapping of the supported file suffixes to their plugins 
	const CSuffixmap& get_supported_filetype_map() const; 

	/// @ returns a string containing the supported file type suffixes
	const std::string get_supported_suffixes() const; 

	/// @ returns a saet containing the supported file type suffixes
	const std::set<std::string> get_supported_suffix_set() const; 

	/**
	   Translate the file type decriptor to the file suffix. 
	   \param type type descriptor (plugin name);
	   \returns preferred suffix of the file type 
	   \remark if the type is onkown it will throw and std::invalid_argument exception 
	 */
	std::string get_preferred_suffix(const std::string& type) const; 

protected: 
	/**
	   constructor that is provided with a list of plugin search path. 
	 */
	TIOPluginHandler(); 

private: 	

	void do_initialise(); 
	// a map of plugins 

	CSuffixmap m_suffixmap; 

	// list of supported compressd file suffixes
	std::set<std::string> m_compress_sfx; 


	// print out info about the available plug-ins 
	void do_print_help(std::ostream& os) const;

	std::string get_handler_type_string_and_help(std::ostream& os) const; 
	std::string do_get_handler_type_string() const; 
        bool do_validate_parameter_string(const std::string& s) const;

	/**
	   Private plugin to handle the virtual data pool IO  
	 */
	class EXPORT_HANDLER CDatapoolPlugin : public Interface {
	public: 
		CDatapoolPlugin(); 
	private: 
		PData do_load(const std::string& fname) const;
		bool do_save(const std::string& fname, 
			     const typename Interface::Data& data) const; 
		const std::string do_get_descr() const;
		const std::string do_get_preferred_suffix() const; 
			
	}; 
	std::shared_ptr<Interface> m_pool_plugin; 
}; 

/**
   \brief Explicitely instanciate all that is needed for an IO plugin 
   \param IOTYPE data type that is handled by this io handler 
 */
#define EXPLICITE_INSTANCEIATE_IO_HANDLER(IOTYPE)			\
	template class TIOPlugin<IOTYPE>;				\
	template class THandlerSingleton<TIOPluginHandler<TIOPlugin<IOTYPE>>>;	\
	template class TIOPluginHandler<TIOPlugin<IOTYPE>>;		\
	template class TPluginHandler<TIOPlugin<IOTYPE>>		\



template <typename Data> 
struct IOHandler_of {
	typedef THandlerSingleton<TIOPluginHandler<TIOPlugin<Data>>> type;
}; 

/**
   This fakes some load image function 
   \remark what is this for? 
*/
template <typename T>
T load_image(const std::string& MIA_PARAM_UNUSED(filename))
{
	static_assert(sizeof(T) == 0, "The call to load_image must be resolved to a template specialization"); 

}

NS_MIA_END
#endif
