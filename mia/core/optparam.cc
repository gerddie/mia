/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <stdexcept>
#include <sstream>
#include <algorithm>

#include <mia/core/optparam.hh>
#include <mia/core/tools.hh>

NS_MIA_BEGIN

using namespace std;



bool CParamList::has_key(const std::string& key) const
{
	return m_params.find(key) != m_params.end();
}

CParamList::PParameter& CParamList::operator [] (const std::string& key)
{
	return m_params[key];
}

void CParamList::set(const CParsedOptions& options)
{
	
	typedef std::map<std::string, PParameter>::iterator::value_type MapValue; 
	for_each(m_params.begin(), m_params.end(), [](MapValue p){p.second->reset();}); 
	
	for (auto i = options.begin();
	     i != options.end(); ++i) {
		auto p = m_params.find(i->first);
		if (p == m_params.end()) {
			stringstream msg;
			msg << "unknown parameter '" << i->first << "'";
			throw invalid_argument(msg.str());
		}
		
		if (!p->second->set(i->second))  {
			stringstream msg;
			msg << "Parameter '" << i->first << "' unable to interpret '" << i->second;
			throw invalid_argument(msg.str());
		}
	}
	for_each(m_params.begin(), m_params.end(), [](MapValue p){p.second->post_set();}); 
}

void CParamList::check_required() const
{
	for (auto i = m_params.begin(); i != m_params.end(); ++i) {
		if (i->second->required_set()) {
			stringstream msg;
			msg << "parameter '" << i->first << "' required ";
			throw invalid_argument(msg.str());
		}
	}
}

void CParamList::print_help(std::ostream& os) const
{
	auto  i = m_params.begin();
	while ( i != m_params.end() ) {
		os  << "\n\t" << i->first;
		i->second->value(os);
		os << "\n\t\t";
		i->second->descr(os);
		++i;
	}
	os << "\n"; 
}

void CParamList::get_help_xml(CXMLElement& root)const
{
	TRACE_FUNCTION; 
	for( auto i = m_params.begin();i != m_params.end(); ++i ) {
		cvdebug()<< "   param '" << i->first << "'\n"; 
		auto p = root.add_child("param"); 
		p->set_attribute("name", i->first); 
		i->second->get_help_xml(*p); 
	}
}

void CParamList::add_dependend_handlers(HandlerHelpMap& handler_map) const
{
	TRACE_FUNCTION; 
	for (auto p = m_params.begin(); p != m_params.end(); ++p) 
		p->second->add_dependend_handler(handler_map); 
}

NS_MIA_END
