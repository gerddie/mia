/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
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

#include <stdexcept>
#include <boost/filesystem/convenience.hpp>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

namespace bfs = ::boost::filesystem; 

template <class I> 
TIOPluginHandler<I>::TIOPluginHandler(const std::list<bfs::path>& searchpath):
	TPluginHandler<I>(searchpath), 
	_M_pool_plugin(new CDatapoolPlugin())
{
	for (const_iterator i = this->begin(); i != this->end(); ++i) 
		i->second->add_suffixes(_M_suffixmap);

	_M_compress_sfx.insert(".Z"); 
	_M_compress_sfx.insert(".gz"); 
	_M_compress_sfx.insert(".bz2"); 

	add_plugin(_M_pool_plugin); 
}

template <class I> 
const typename TIOPluginHandler<I>::Interface *
TIOPluginHandler<I>::prefered_plugin_ptr(const std::string& fname) const
{
	// get the suffix - if there is a Z, gz, or bz2, include it in the suffix
	bfs::path fpath(fname);
	std::string fsuffix = fpath.extension(); 
	if (!fsuffix.empty()) {
		if (_M_compress_sfx.find(fsuffix) != _M_compress_sfx.end()) {
			// remove the last extension and get the one before
			bfs::path help(fpath.stem()); 
			fsuffix = help.extension();
		}
	}else 
		fsuffix = fname; 
	
	if (fsuffix[0] != '.') 
		fsuffix = string(".") + fsuffix; 
	
	cvdebug() << "looking up plugin for '" << fsuffix << "'\n"; 

	if (fsuffix == ".datapool") 
		return _M_pool_plugin; 

	CSuffixmap::const_iterator p = _M_suffixmap.find(fsuffix);
	if (p != _M_suffixmap.end())
		return this->plugin(p->second.c_str());
	return 0; 
}

template <class I> 
const typename TIOPluginHandler<I>::Interface&
TIOPluginHandler<I>::prefered_plugin(const std::string& fname) const
{
	// get the suffix - if there is a Z, gz, or bz2, include it in the suffix
	bfs::path fpath(fname);
	std::string fsuffix = fpath.extension(); 
	if (_M_compress_sfx.find(fsuffix) != _M_compress_sfx.end()) {
		bfs::path  help(fpath.stem()); 
		fsuffix = help.extension();
	}
	
	CSuffixmap::const_iterator p = _M_suffixmap.find(fsuffix);
	if (p != _M_suffixmap.end())
		return *this->plugin(p->second.c_str());
	throw invalid_argument(string("no plugin corresponds to '") + fsuffix + "'"); 
}

template <class I> 
const typename TIOPluginHandler<I>::CSuffixmap& 
TIOPluginHandler<I>::get_supported_filetype_map() const
{
	return _M_suffixmap; 
}


template <class I> 
typename TIOPluginHandler<I>::PData
TIOPluginHandler<I>::load(const std::string& fname) const
{
	const Interface *pp = prefered_plugin_ptr(fname); 
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
	const Interface *pp = prefered_plugin_ptr(fname); 
	if (pp->has_property("pool-placeholder"))
		return TDelayedParameter<PData>(fname); 

	if (pp) {
		PData retval = pp->load(fname); 
		if (retval.get()) {
			CDatapool::Instance().add(fname, retval); 
			return TDelayedParameter<PData>(fname); 
		}
	}

	for (const_iterator i = this->begin(); i != this->end(); ++i) {
		PData retval = 	i->second->load(fname); 
		if (retval.get()) {
			CDatapool::Instance().add(fname, retval); 
			return TDelayedParameter<PData>(fname); 
		}
	}
	throw std::runtime_error(std::string("unable to load from ") + fname);
}

/// \returns a reference to the only instance of the plugin handler 
template <typename T>
const T& TIOHandlerSingleton<T>::instance()
{
	TRACE_FUNCTION; 
	static TIOHandlerSingleton<T> me; 
	return me; 
}


template <class I> 
bool TIOPluginHandler<I>::save(const std::string& type, 
			       const std::string& fname, const Data& data) const
{
	const I* p = NULL; 

	if (type.empty()) {
		p = prefered_plugin_ptr(fname); 
		if (!p) 
			p = this->plugin(data.get_source_format().c_str()); 
	}else 
		p = this->plugin(type.c_str());

	if (!p) {
		stringstream errmsg; 
		errmsg << "Unable to find an appropriate plugin from type = '" << type
		       << "' filename = '" << fname 
		       << "' and source format = '" << data.get_source_format() << "'"; 
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
	map.insert(pair<string,string>(".datapool", this->get_name())); 
}

template <class I> 
typename TIOPluginHandler<I>::PData 
TIOPluginHandler<I>::CDatapoolPlugin::do_load(const std::string& fname) const
{
	if (CDatapool::Instance().has_key(fname)) {
		boost::any value = CDatapool::Instance().get(fname); 
		return boost::any_cast<typename Interface::PData>(value);
	}else
		return typename TIOPluginHandler<I>::PData(); 
}

template <class I> 
bool TIOPluginHandler<I>::CDatapoolPlugin::do_save(const std::string& fname, 
						   const typename Interface::Data& data) const
{
	typename Interface::PData value = 
		typename Interface::PData(data.clone()); 
	CDatapool::Instance().add(fname, value);
	return true; 
}

template <class I> 
const std::string TIOPluginHandler<I>::CDatapoolPlugin::do_get_descr() const
{
	return "virtual IO to and from data pool"; 
}



NS_MIA_END

