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

#include <mia/core/paramtranslator.hh>

NS_MIA_BEGIN
using std::invalid_argument; 


CParamTranslator::CParamTranslator(const char *name):
        m_name(name)
{
}

CParamTranslator::~CParamTranslator()
{
}
        
void CParamTranslator::add_parameter(const std::string& name, CParameter *param)
{
        CParamList::PParameter p(param);
	if ( m_parameters.has_key(name) ) {
                throw create_exception<invalid_argument>("CParamTranslator: Parameter name '", 
                                                         name, "' already in use'");
	}
	m_parameters[name] = p;
}

void CParamTranslator::set_parameters(const CParsedOptions& options)
{
        try {
		m_parameters.set(options);
	}
	catch (invalid_argument& x) {
                throw create_exception<invalid_argument>("CParamTranslator:", x.what());
	}
}

void CParamTranslator::check_parameters()
{
	try {
		m_parameters.check_required();
	}
	catch (invalid_argument& x) {
                throw create_exception<invalid_argument>("CParamTranslator:", x.what());
        }
}

void CParamTranslator::get_short_help(std::ostream& os) const
{
	os << get_name() << ": " << get_descr() << "\n";
	m_parameters.print_help(os);
	os  << "\n";
}

const char *CParamTranslator::get_name() const
{
        return m_name; 
}

const std::string CParamTranslator::get_descr() const
{
	return do_get_descr();
}


void CParamTranslator::get_help(std::ostream& os) const
{
	m_parameters.print_help(os);
}

void CParamTranslator::get_help_xml(xmlpp::Element& root) const
{
	cvdebug() << "Get help for " << m_name << "\n"; 
	root.set_child_text(get_descr()); 
	do_get_help_xml(root); 
}

void CParamTranslator::do_get_help_xml(xmlpp::Element& root) const
{
	TRACE_FUNCTION; 
	m_parameters.get_help_xml(root);
}

CParamList& CParamTranslator::get_parameters()
{
        return m_parameters; 
}

NS_MIA_END
