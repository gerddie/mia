/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <cassert>
#include <mia/core/typedescr.hh>
#include <mia/core/cmdbooloption.hh>
#include <mia/core/xmlinterface.hh>

NS_MIA_BEGIN


CCmdBoolOption::CCmdBoolOption(bool& value, char short_opt, const char *long_opt, const char *long_help, 
			       CCmdOptionFlags flags):
	CCmdOption(short_opt, long_opt, long_help, long_opt, flags), 
	m_value(value)
{
	m_value = false; 

	assert(!has_flag(CCmdOptionFlags::required) && "A boolean flag option must not have the flag CCmdOptionFlags::required"); 
}

bool CCmdBoolOption::do_set_value(const char */*str_value*/)
{
	m_value = true; 
	return true; 
}

void CCmdBoolOption::do_write_value(std::ostream& /*os*/) const
{
}									

void CCmdBoolOption::do_get_long_help_xml(std::ostream& os, CXMLElement& parent, HandlerHelpMap& /*handler_map*/) const
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
