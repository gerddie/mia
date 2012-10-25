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

#ifndef ph_ioplugin_hh
#error include ioplugin.hh first
#endif

#include <iostream>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN


template <typename D> 
TIOPlugin<D>::TIOPlugin(const char *name):
	TPlugin<D, io_plugin_type>(name)
{
}


template <typename D> 
void TIOPlugin<D>::add_suffixes(std::multimap<std::string, std::string>& map) const
{
	do_add_suffixes(map); 
}

template <typename D> 
typename TIOPlugin<D>::PData TIOPlugin<D>::load(const std::string& fname) const
{
	typename TIOPlugin<D>::PData retval = do_load(fname);
	if (retval)
		retval->set_source_format(this->get_name()); 
	return retval; 
}

template <typename D> 
bool TIOPlugin<D>::save(const std::string& fname, const Data& data) const
{
	return do_save(fname, data);
}

template <typename D> 
void TIOPlugin<D>::do_get_help_xml(xmlpp::Element& root) const
{
	//flag that docu should not print help about non-existent parameters 
	root.add_child("noparam"); 
}

template <typename D> 
const typename TIOPlugin<D>::PixelTypeSet& TIOPlugin<D>::supported_pixel_types() const
{
	return m_typeset; 
}



template <typename D> 
void TIOPlugin<D>::add_supported_type(EPixelType t)
{
	m_typeset.insert(t); 
}

template <typename D> 
std::string TIOPlugin<D>::get_preferred_suffix() const
{
	return 	this->do_get_preferred_suffix(); 
}

template <typename D> 
std::string TIOPlugin<D>::do_get_preferred_suffix() const
{
	return this->get_name(); 
}

NS_MIA_END
