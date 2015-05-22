/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
void TIOPlugin<D>::add_suffixes(std::multimap<std::string, std::string>& map)const 
{
	for (auto s = m_suffixes.begin(); s != m_suffixes.end(); ++s) 
		map.insert(std::pair<std::string, std::string>(*s, this->get_name())); 

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

template <typename I, typename T>
std::string get_values_as_string(I begin, I end, T translate)
{
	assert(begin != end); 
	std::stringstream str; 
	str << translate(*begin++); 
	while (begin != end) 
		str << ", " << translate(*begin++); 
	return str.str(); 
}

template <typename D> 
void TIOPlugin<D>::do_get_help_xml(CXMLElement& root) const
{
	//flag that docu should not print help about non-existent parameters 
	root.add_child("noparam"); 

	if (!m_suffixes.empty()) {
		auto suffix_docu = root.add_child("suffixes");
		auto s = get_values_as_string(m_suffixes.begin(), m_suffixes.end(),
					      [](const std::string& s){return s;}); 
		suffix_docu->set_child_text(s); 
	}
	if (!m_typeset.empty()) {
		auto type_docu = root.add_child("datatypes");
		auto s = get_values_as_string(m_typeset.begin(), m_typeset.end(),
					      [](EPixelType pt){return CPixelTypeDict.get_help(pt);}); 
		type_docu->set_child_text(s); 
	}
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
void TIOPlugin<D>::add_suffix(const std::string& suffix)
{
	m_suffixes.insert(suffix); 
}

template <typename D> 
const typename TIOPlugin<D>::SuffixSet& TIOPlugin<D>::get_suffixes() const
{
	return m_suffixes; 
}

template <typename D> 
const std::string TIOPlugin<D>::get_preferred_suffix() const
{
	return 	this->do_get_preferred_suffix(); 
}

template <typename D> 
const std::string TIOPlugin<D>::do_get_preferred_suffix() const
{
	return this->get_name(); 
}

NS_MIA_END
