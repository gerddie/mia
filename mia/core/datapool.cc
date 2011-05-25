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

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <mia/core/datapool.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN
using namespace std;
CDatapool::CDatapool()
{
}

boost::any CDatapool::get(const std::string& key) const
{
	CScopedLock lock(m_mutex);
	std::cout << "read " << key << "\n"; 
	Anymap::const_iterator i = get_iterator(key);
	m_usage[key] = true;

	return i->second;
}

boost::any CDatapool::get_and_remove(const std::string& key)
{
	CScopedLock lock(m_mutex);
	Anymap::const_iterator i = get_iterator(key);
	boost::any retval = i->second;
	m_map.erase(key);
	m_usage.erase(key);
	return retval;
}

void CDatapool::add(const std::string& key, boost::any value)
{
	CScopedLock lock(m_mutex);
	std::cout << "add '" << key << "'\n";
	m_usage[key] = false;
	m_map[key] = value;
}

CMutex CDatapool::m_mutex; 

CDatapool& CDatapool::Instance()
{
	CScopedLock lock(m_mutex);
	static CDatapool pool;
	return pool;
}

bool CDatapool::has_key(const std::string& key) const
{
	CScopedLock lock(m_mutex);
	return m_map.find(key) != m_map.end();
}

bool CDatapool::has_unused_data() const
{
	CScopedLock lock(m_mutex);
	bool result = false;
	for (Usagemap::const_iterator u = m_usage.begin();
	     u != m_usage.end(); ++u) {
		if (!u->second)  {
			result = true;
			cvinfo() << "Datapool has unused parameter '" << u->first << "\n";
		}
	}
	return result;
}

CDatapool::const_iterator CDatapool::get_iterator(const std::string& key) const
{
	cvdebug() << "CDatapool::get: '" << key << "' ... ";
	Anymap::const_iterator i = m_map.find(key);
	if (i == m_map.end()) {
		cverb << "fail\n";
		stringstream msg;
		msg << "CDatapool: key '" << key << "' not available";
		throw invalid_argument(msg.str());
	}
	return i;
}

NS_MIA_END
