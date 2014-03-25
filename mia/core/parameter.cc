/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#define VSTREAM_DOMAIN "core-parameter"
#include <sstream>
#include <libxml++/libxml++.h>
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

const char *CParameter::type() const
{
	return m_type;
}

void CParameter::get_help_xml(xmlpp::Element& param) const
{
	TRACE_FUNCTION; 
	param.set_attribute("type", m_type); 
	param.set_attribute("required", to_string<bool>(m_is_required)); 
	param.set_attribute("default", get_default_value());
	ostringstream d; 
//	descr(d); 
	param.set_child_text(m_descr);
	do_get_help_xml(param); 
}

void CParameter::do_get_help_xml(xmlpp::Element& /*param*/) const
{
}

void CParameter::post_set()
{

}

std::string CParameter::get_value_as_string() const
{
	return do_get_value_as_string(); 
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
	TRACE_FUNCTION; 
	do_add_dependend_handler(handler_map); 
}

void CParameter::do_add_dependend_handler(HandlerHelpMap& /*handler_map*/)const
{
}

std::string CParameter::get_default_value() const
{
	return do_get_default_value(); 
}

CStringParameter::CStringParameter(std::string& value,CCmdOptionFlags flags , const char *descr, 
				   const CPluginHandlerBase *plugin_hint):
	CParameter(__type_descr<std::string>::value, has_flag(flags, CCmdOptionFlags::required), descr),
	m_value(value), 
	m_default_value(value), 
	m_flags(flags), 
	m_plugin_hint(plugin_hint)
{
}

void CStringParameter::do_reset()
{
	m_value = m_default_value; 
}

bool CStringParameter::do_set(const std::string& str_value)
{
	m_value = str_value; 
	return true; 
}
std::string CStringParameter::do_get_default_value() const
{
	return m_default_value; 
}
std::string CStringParameter::do_get_value_as_string() const
{
	return m_value; 
}


void CStringParameter::do_descr(std::ostream& os) const
{
	TRACE_FUNCTION; 
	if (m_plugin_hint) 
		m_plugin_hint->print_help(os); 
}

void CStringParameter::do_get_help_xml(xmlpp::Element& self) const
{
	TRACE_FUNCTION; 
	if (m_plugin_hint)  {
		auto type = m_plugin_hint->get_handler_type_string(); 
		auto dict = self.add_child(type); 
		dict->set_attribute("name", m_plugin_hint->get_descriptor());
		self.set_attribute("type", type); 
	}
	if (m_flags != CCmdOptionFlags::none) {
		auto flags = self.add_child("flags");
		
		ostringstream ss; 
		if (has_flag(m_flags, CCmdOptionFlags::input))
			ss << "input "; 
		if (has_flag(m_flags, CCmdOptionFlags::output))
			ss << "output "; 
		if (has_flag(m_flags, CCmdOptionFlags::required))
			ss << "required ";
		flags->set_child_text(ss.str());
	}
}

void CStringParameter::do_add_dependend_handler(HandlerHelpMap& handler_map)const
{
	TRACE_FUNCTION; 
	if (m_plugin_hint) 
		m_plugin_hint->add_dependend_handlers(handler_map); 
}

template class TRangeParameter<unsigned short>;
template class TRangeParameter<unsigned int>;
template class TRangeParameter<unsigned long>;
template class TRangeParameter<short>;
template class TRangeParameter<int>;
template class TRangeParameter<long>;
template class TRangeParameter<float>;
template class TRangeParameter<double>; 

template class CTParameter<unsigned short>;
template class CTParameter<unsigned int>;
template class CTParameter<unsigned long>;
template class CTParameter<short>;
template class CTParameter<int>;
template class CTParameter<long>;
template class CTParameter<float>;
template class CTParameter<double>; 

template class CTParameter<string>;
template class CTParameter<bool>;


NS_MIA_END
