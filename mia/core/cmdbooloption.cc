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

#include <mia/core/typedescr.hh>
#include <mia/core/cmdbooloption.hh>

NS_MIA_BEGIN


CCmdBoolOption::CCmdBoolOption(bool& value, char short_opt, const char *long_opt, const char *long_help):
	CCmdOption(short_opt, long_opt, long_help, long_opt, false), 
	m_value(value)
{
	m_value = false; 
}

bool CCmdBoolOption::do_set_value(const char */*str_value*/)
{
	m_value = true; 
	return true; 
}

void CCmdBoolOption::do_write_value(std::ostream& /*os*/) const
{
}									

void CCmdBoolOption::do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& /*handler_map*/) const
{
	do_get_long_help(os);
	parent.set_attribute("type", __type_descr<bool>::value);

}

const std::string CCmdBoolOption::do_get_value_as_string() const
{
	return m_value ? "true": "false"; 
}

size_t CCmdBoolOption::do_get_needed_args() const
{
	return 0; 
}

NS_END 
