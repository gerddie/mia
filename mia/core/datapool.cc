/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <sstream>
#include <stdexcept>
#include <mia/core/datapool.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

typedef tbb::recursive_mutex::scoped_lock CRecursiveScopedLock; 
using namespace std;
CDatapool::CDatapool()
{
}

boost::any CDatapool::get(const std::string& key) const
{
	CRecursiveScopedLock lock(m_mutex);
	Anymap::const_iterator i = get_iterator(key);
	m_usage[key] = true;

	return i->second;
}

boost::any CDatapool::get_and_remove(const std::string& key)
{
	CRecursiveScopedLock lock(m_mutex);
	Anymap::const_iterator i = get_iterator(key);
	boost::any retval = i->second;
	m_map.erase(key);
	m_usage.erase(key);
	return retval;
}

void CDatapool::add(const std::string& key, boost::any value)
{
	CRecursiveScopedLock lock(m_mutex);
	m_usage[key] = false;
	m_map[key] = value;
}

tbb::recursive_mutex CDatapool::m_mutex; 

CDatapool& CDatapool::instance()
{
	CRecursiveScopedLock lock(m_mutex);
	static CDatapool pool;
	return pool;
}

bool CDatapool::has_key(const std::string& key) const
{
	CRecursiveScopedLock lock(m_mutex);
	return m_map.find(key) != m_map.end();
}

bool CDatapool::has_unused_data() const
{
	CRecursiveScopedLock lock(m_mutex);
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


void CDatapool::clear()
{
	CRecursiveScopedLock lock(m_mutex);
	if (has_unused_data()) 
		cvmsg() << "CDatapool: The data pool holds data that was never used\n"; 

	m_map.clear(); 
	m_usage.clear(); 
}

NS_MIA_END
