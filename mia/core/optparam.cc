/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <sstream>
#include <algorithm>

#include <mia/core/optparam.hh>

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
	for_each(m_params.begin(), m_params.end(), [](MapValue p){p.second.reset();}); 
	
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
	// this should be sorted somehow ...
	std::map<std::string, PParameter>::const_iterator i = m_params.begin();
	while ( i != m_params.end() ) {
		os  << "\t" << i->first;
		i->second->descr(os);
		os << "\n";
		++i;
	}
}


NS_MIA_END
