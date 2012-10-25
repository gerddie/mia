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

#include <mia/core/handlerbase.hh>

NS_MIA_BEGIN

CPluginHandlerBase::CPluginHandlerBase(const std::string& descriptor):
	m_descriptor(descriptor)
{
}

CPluginHandlerBase::~CPluginHandlerBase() 
{
	
}

void CPluginHandlerBase::print_short_help(std::ostream& os) const
{
	do_print_short_help(os); 
}

void CPluginHandlerBase::print_help(std::ostream& os) const
{
	do_print_help(os); 
}

void CPluginHandlerBase::get_xml_help(xmlpp::Element *root) const
{
	xmlpp::Element* handlerRoot = root->add_child("handler");
	handlerRoot->set_attribute("name", get_descriptor());
	do_get_xml_help(handlerRoot); 
}

const std::string& CPluginHandlerBase::get_descriptor() const
{
	return m_descriptor; 
}

void CPluginHandlerBase::add_dependend_handlers(HandlerHelpMap& handler_map) const
{
	if (handler_map.find(m_descriptor) != handler_map.end()) 
		return; 
	handler_map[m_descriptor] = this; 	
	do_add_dependend_handlers(handler_map);
}

void CPluginHandlerBase::get_string_help_description_xml(std::ostream& os, xmlpp::Element *parent) const
{
	auto type = get_handler_type_string_and_help(os); 
	auto factory = parent->add_child(type);
	factory->set_attribute("name", get_descriptor());
	parent->set_attribute("type", type);
}

std::string CPluginHandlerBase::get_handler_type_string_and_help(std::ostream& os) const
{
	return "base"; 
}

NS_MIA_END
