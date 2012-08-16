/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <stdexcept>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/core/handler.cxx>


NS_MIA_BEGIN

namespace bfs = ::boost::filesystem; 

template <class I> 
TIOPluginHandler<I>::TIOPluginHandler(const std::list<bfs::path>& searchpath):
	TPluginHandler<I>(searchpath), 
	m_pool_plugin(new CDatapoolPlugin())
{
	this->add_plugin(m_pool_plugin); 
	for (const_iterator i = this->begin(); i != this->end(); ++i) 
		i->second->add_suffixes(m_suffixmap);

	m_compress_sfx.insert(".Z"); 
	m_compress_sfx.insert(".gz"); 
	m_compress_sfx.insert(".bz2"); 
}

template <class I> 
const typename TIOPluginHandler<I>::Interface *
TIOPluginHandler<I>::preferred_plugin_ptr(const std::string& fname) const
{
	// get the suffix - if there is a Z, gz, or bz2, include it in the suffix
	bfs::path fpath(fname);
	std::string fsuffix = __bfs_get_extension(fpath); 
	if (!fsuffix.empty()) {
		if (m_compress_sfx.find(fsuffix) != m_compress_sfx.end()) {
			// remove the last extension and get the one before
			bfs::path help(fpath.stem()); 
			fsuffix = __bfs_get_extension(fpath); 
		}
	}else 
		fsuffix = fname; 
	
	if (fsuffix[0] != '.') 
		fsuffix = string(".") + fsuffix; 
	
	cvdebug() << "looking up plugin for '" << fsuffix << "'\n"; 

	if (fsuffix == ".datapool") 
		return m_pool_plugin; 

	CSuffixmap::const_iterator p = m_suffixmap.find(fsuffix);
	if (p != m_suffixmap.end())
		return this->plugin(p->second.c_str());
	return 0; 
}

template <class I> 
std::string TIOPluginHandler<I>::get_preferred_suffix(const std::string& type) const
{
	auto plugin = this->plugin(type.c_str());
	if ( !plugin ) {
		THROW(invalid_argument, "Plug-in '" << type << "' not available"); 
	}
	return plugin->get_preferred_suffix(); 
}

template <class I> 
const typename TIOPluginHandler<I>::Interface&
TIOPluginHandler<I>::preferred_plugin(const std::string& fname) const
{
	// get the suffix - if there is a Z, gz, or bz2, include it in the suffix
	bfs::path fpath(fname);
	auto fsuffix = __bfs_get_extension(fpath); 
	if (m_compress_sfx.find(fsuffix) != m_compress_sfx.end()) {
		bfs::path  help(fpath.stem()); 
		fsuffix = __bfs_get_extension(fpath); 
	}
	cvdebug() << "Got suffix '" << fsuffix << "'\n"; 
	auto p = m_suffixmap.find(fsuffix);
	if (p != m_suffixmap.end())
		return *this->plugin(p->second.c_str());
	THROW(invalid_argument, "No plugin from '"
	      << I::PlugType::type_descr << "/" <<  I::PlugData::data_descr
	      << "' corresponds to suffix '" <<  fsuffix << "' supported suffixes:" 
	      << get_supported_suffixes()); 
}

template <class I> 
const typename TIOPluginHandler<I>::CSuffixmap& 
TIOPluginHandler<I>::get_supported_filetype_map() const
{
	return m_suffixmap; 
}

template <class I> 
const std::string TIOPluginHandler<I>::get_supported_suffixes() const
{
	std::stringstream result; 
	for (auto i = m_suffixmap.begin(); i != m_suffixmap.end(); ++i)
		result << i->first << ' '; 
	return result.str(); 
}

template <class I> 
typename TIOPluginHandler<I>::PData
TIOPluginHandler<I>::load(const std::string& fname) const
{
	const Interface *pp = preferred_plugin_ptr(fname); 
	if (pp) {
		PData retval = pp->load(fname); 
		if (retval.get()) 
			return retval; 
	}
	
	for (const_iterator i = this->begin(); i != this->end(); ++i) {
		PData retval = 	i->second->load(fname); 
		if (retval.get())
			return retval; 
	}
	throw std::runtime_error(std::string("unable to load from ") + fname);
}

template <class I> 
typename TIOPluginHandler<I>::DataKey
TIOPluginHandler<I>::load_to_pool(const std::string& fname) const
{
	TDelayedParameter<PData> result(fname); 
	
	// load to pool reuses available copies. 
	if (CDatapool::instance().has_key(fname)) 
		return result; 

	const Interface *pp = preferred_plugin_ptr(fname); 
	if (pp) {
		// if the IO plugin signals, that the data will be loaded later
		// just return the key 
		if (pp->has_property("pool-placeholder"))
			return result; 
		
		// try the preferred plugin to load the data 
		// on sucess put it into the pool and return the key 
		PData retval = pp->load(fname); 
		if (retval.get()) {
			CDatapool::instance().add(fname, retval); 
			return result; 
		}
	}

	// try all plugins if one fits the data on file 
	for (const_iterator i = this->begin(); i != this->end(); ++i) {
		PData retval = 	i->second->load(fname); 
		if (retval.get()) {
			CDatapool::instance().add(fname, retval); 
			return result; 
		}
	}
	throw std::runtime_error(std::string("unable to load from ") + fname);
}



/// \returns a reference to the only instance of the plugin handler 
template <typename T>
const T& TIOHandlerSingleton<T>::instance()
{
	CScopedLock lock(THandlerSingleton<T>::m_creation_mutex); 
	TRACE_FUNCTION; 
	static TIOHandlerSingleton<T> me; 
	return me; 
}


template <class I> 
bool TIOPluginHandler<I>::save(const std::string& fname, const Data& data) const
{
	const I* p = NULL; 

	p = preferred_plugin_ptr(fname); 
	// okay, file name didn't help, let's see if the data knows about its input type 
	if (!p) {
		if (!data.get_source_format().empty())
			p = this->plugin(data.get_source_format().c_str()); 
	}
	
	// todo: no file type given, one should try the best now, like take the 
	// first file format that supports the data 
	
	// bail out with an error
	if (!p) {
		stringstream errmsg; 
		errmsg << "Unable to find an appropriate plugin to save to file "
		       << " '" << fname << "' (based on its extension) "; 
 		if (!data.get_source_format().empty())
			errmsg << " or to format = '" << data.get_source_format() << "'";
		throw invalid_argument(errmsg.str()); 
	}
	return p->save(fname, data); 
}

template <class I> 
TIOPluginHandler<I>::CDatapoolPlugin::CDatapoolPlugin():
	Interface("datapool")
{
	this->add_property("pool-placeholder"); 
}

template <class I> 
void TIOPluginHandler<I>::CDatapoolPlugin::do_add_suffixes(
        typename TIOPluginHandler<I>::CSuffixmap& map) const
{
	cvdebug() << "Add pair" << "'.@' -> '" << this->get_name() << "'\n"; 
	map.insert(pair<string,string>(".@", this->get_name())); 
}

template <class I> 
typename TIOPluginHandler<I>::PData 
TIOPluginHandler<I>::CDatapoolPlugin::do_load(const std::string& fname) const
{
	if (CDatapool::instance().has_key(fname)) {
		boost::any value = CDatapool::instance().get(fname); 
		return boost::any_cast<typename Interface::PData>(value);
	}else
		return typename TIOPluginHandler<I>::PData(); 
}

template <class I> 
bool TIOPluginHandler<I>::CDatapoolPlugin::do_save(const std::string& fname, 
						   const typename Interface::Data& data) const
{
	// why do I need to clone this? Being a shared pointer should be sufficient
	typename Interface::PData value = 
		typename Interface::PData(data.clone()); 
	CDatapool::instance().add(fname, value);
	return true; 
}

template <class I> 
const std::string TIOPluginHandler<I>::CDatapoolPlugin::do_get_descr() const
{
	return "virtual IO to and from data pool"; 
}



NS_MIA_END

