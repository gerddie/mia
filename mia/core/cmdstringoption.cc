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


#include <mia/core/cmdstringoption.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/typedescr.hh>

NS_MIA_BEGIN

using std::string; 

CCmdStringOption::CCmdStringOption(std::string& value, char short_opt, const char *long_opt, 
				   const char *long_help, bool required, const CPluginHandlerBase *plugin_hint):
	CCmdOption(short_opt, long_opt, long_help, long_opt, required), 
	m_value(value), 
	m_plugin_hint(plugin_hint)
{
	if (!value.empty() && required) 
		cvwarn() << "CCmdStringOption:option '" << long_opt << "' has a default value '"
			 << value
			 <<"' but is also marked as required. This doesn't make mzúch sense."; 
}

bool CCmdStringOption::do_set_value(const char *str_value)
{
	m_value.assign(str_value); 
	return true; 
}

const string CCmdStringOption::do_get_value_as_string() const
{
	return m_value; 
}

void CCmdStringOption::do_write_value(std::ostream& os) const
{
	if (m_value.empty()) 
		if (required)
			os << "[required] "; 
		else
			os << "=NULL ";
	else 
		os << "=" << m_value;
}

void CCmdStringOption::do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& handler_map) const
{
	if (m_plugin_hint) {
		m_plugin_hint->add_dependend_handlers(handler_map); 
		m_plugin_hint->get_string_help_description_xml(os, &parent); 
	}else
		parent.set_attribute("type", __type_descr<string>::value);
}

void CCmdStringOption::do_get_long_help(std::ostream& os) const
{
	if (m_plugin_hint)
		m_plugin_hint->print_help(os); 
}


NS_MIA_END

