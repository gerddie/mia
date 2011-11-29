/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#include <sstream>
#include <mia/core/parameter.hh>
#include <mia/core/parameter.cxx>


NS_MIA_BEGIN
using namespace std;

CParameter::CParameter(const char type[], bool required, const char *descr):
    m_required(required),
    m_is_required(required),
    m_type(type),
    m_descr(descr)
{
}

CParameter::~CParameter()
{
}

void CParameter::reset()
{
	m_required = m_is_required; 
	do_reset(); 
}

void CParameter::do_reset()
{	
}

const char *CParameter::type() const
{
	return m_type;
}

void CParameter::get_help_xml(xmlpp::Element& param) const
{
	param.set_attribute("type", m_type); 
	param.set_attribute("required", to_string<bool>(m_is_required)); 
	param.set_attribute("default", get_default_value());
	ostringstream d; 
	descr(d); 
	param.set_child_text(d.str());
	do_get_help_xml(param); 
}

void CParameter::value(std::ostream& os) const
{
	os << "=" << (m_required ? "[required]" : get_default_value()) 
	   <<  " ("<< m_type << ")"; 
}

void CParameter::descr(ostream& os) const
{
	os << m_descr << " ";
	do_descr(os);
}

const char *CParameter::get_descr() const
{
	return m_descr;
}

const std::string CParameter::errmsg(const std::string& err_value) const
{
	std::stringstream msg;
	msg <<  "Parameter<"<< type() << ">::set(" << get_descr() <<
		"): value '" << err_value << "' not allowed";
	return msg.str();
}

bool CParameter::required_set() const
{
	return m_required;
}

bool CParameter::set(const string& str_value)
{
	m_required = false;
	return do_set(str_value);
}

void CParameter::add_dependend_handler(HandlerHelpMap& handler_map)const
{
	do_add_dependend_handler(handler_map); 
}

void CParameter::do_add_dependend_handler(HandlerHelpMap& /*handler_map*/)const
{
}

std::string CParameter::get_default_value() const
{
	return do_get_default_value(); 
}

const char  type_str_uint[5] = "uint";
const char  type_str_int[4] = "int";
const char  type_str_float[6] = "float";
const char  type_str_double[7] = "double";
const char  type_str_string[7] = "string";
const char  type_str_bool[5] = "bool";

template class TRangeParameter<unsigned int, type_str_uint>;
template class TRangeParameter<int, type_str_int>;
template class TRangeParameter<float, type_str_float>;
template class CTParameter<string,type_str_string>;
template class CTParameter<bool, type_str_bool>;
template class TRangeParameter<double, type_str_double>; 

NS_MIA_END
