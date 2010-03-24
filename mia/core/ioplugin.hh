/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#ifndef ph_ioplugin_hh
#define ph_ioplugin_hh

#include <set>
#include <mia/core/plugin_base.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/iodata.hh>
#include <mia/core/shared_ptr.hh>

NS_MIA_BEGIN

struct EXPORT_CORE io_plugin_type {
	static  const char *value;

};

/**
   \class TIOPlugin template class 
   The basis of all io plug-ins. The type \a D must provied a typedef \a type the referes to the
   data type to be handled and it must provied a string \a value that describes the type of
   the io data and also corresponds to the last part of the directory name, where the plug-in
   will be located after installation of the package.
   The type D must be derived from \a CIOData, to supply functions to store and acquire the
   file format used to load the data.
*/

template <typename D>
class EXPORT_HANDLER TIOPlugin: public TPlugin<D, io_plugin_type> {

public:
	typedef  typename D::type Data;
	typedef  typename SHARED_PTR(Data) PData;
	typedef  std::set<EPixelType> PixelTypeSet;

	/**
	   Constructor of the Plugin
	   \param name the name of the plug-in, should correnspond to the file type
	*/
	TIOPlugin(const char *name);

	/**
	   Adds file suffixes supported by this plug-in to a map to improve loading of files.
	   \param map the map, the suffixes will be added to.
	*/
	void add_suffixes(std::multimap<std::string, std::string>& map) const;

	/**
	   the load function, the actual implementation of the loading is implemented in do_load,
	   which has to be implemented in the according plug-in
	   \param fname the file name to be loaded
	   \returns a shared pointer to the loaded data, or an empty shared pointer,
	   if no data could be loaded.
	*/
	PData load(const std::string& fname) const;

	/**
	   The save function. Saves the \a data to file \a fname. The actual save function is implemented in do_save
	   and needs to be provied in th eaccording IO plug-in.
	   \param fname file name
	   \param data data to be stored.
	   \returns true if the saving was succesfull, false otherwise
	*/
	bool save(const std::string& fname, const typename D::type& data) const;

	/// \returns a set of thepixel types, supported by this IO plug-in
	const PixelTypeSet& supported_pixel_types() const;

protected:

	/// add pixel type t to the list of supported types
	void add_supported_type(EPixelType t);
private:
	PixelTypeSet _M_typeset;

	/**
	   Interface function that needs to be impemented ina a derived class:
	   Set the suffixes handled by this plug-in
	   \param map suffix:plugin map to add to
	*/
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const = 0;

	/**
	   Interface function that needs to be impemented ina a derived class:
	   \param fname
	 */
	virtual PData do_load(const std::string& fname) const = 0;

	/**
	   Interface function that needs to be impemented ina a derived class:
	   \param fname
	   \param data the data to be saved
	   \returns true if saveing was successfull
	 */
	virtual bool do_save(const std::string& fname, const typename D::type& data) const = 0;

	virtual bool do_test() const;
};

EXPORT_CORE extern const char * const io_plugin_property_multi_record;
EXPORT_CORE extern const char * const io_plugin_property_history_split;
EXPORT_CORE extern const char * const io_plugin_property_has_attributes;

NS_MIA_END

#endif
