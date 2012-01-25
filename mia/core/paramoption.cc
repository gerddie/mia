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

#include <mia/core/paramoption.hh>
#include <sstream>

NS_MIA_BEGIN
using std::ostringstream; 

CParamOption::CParamOption(char short_opt, const char *long_opt, CParameter *param):
	CCmdOption(short_opt, long_opt, param->get_descr(), 
		   long_opt, 
		   param->required_set() ? CCmdOption::required : CCmdOption::not_required), 
	m_param(param)
{
	
}

void CParamOption::do_get_long_help(std::ostream& os) const
{
	m_param->descr(os); 
}

bool CParamOption::do_set_value(const char *str_value)
{
	return  m_param->set(str_value);
}

size_t CParamOption::do_get_needed_args() const
{
	return 1; 
}

const std::string CParamOption::do_get_value_as_string() const
{
	return m_param->get_value_as_string();
}

void CParamOption::do_write_value(std::ostream& os) const
{
	m_param->value(os);
}

void CParamOption::post_set()
{
	m_param->post_set();
}

NS_MIA_END
