/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
	Anymap::const_iterator i = get_iterator(key);
	_M_usage[key] = true;
	cverb << "success\n";
	return i->second;
}

boost::any CDatapool::get_and_remove(const std::string& key)
{
	Anymap::const_iterator i = get_iterator(key);
	boost::any retval = i->second;
	_M_map.erase(key);
	_M_usage.erase(key);
	return retval;
}

void CDatapool::add(const std::string& key, boost::any value)
{
	cvdebug() << "add '" << key << "'\n";
	_M_usage[key] = false;
	_M_map[key] = value;
}

CDatapool& CDatapool::Instance()
{
	static CDatapool pool;
	return pool;
}

bool CDatapool::has_key(const std::string& key) const
{
	return _M_map.find(key) != _M_map.end();
}

bool CDatapool::has_unused_data() const
{
	bool result = false;
	for (Usagemap::const_iterator u = _M_usage.begin();
	     u != _M_usage.end(); ++u) {
		if (!u->second)  {
			result = true;
			cvwarn() << "Datapool has unused parameter '" << u->first << "\n";
		}
	}
	return result;
}

CDatapool::const_iterator CDatapool::get_iterator(const std::string& key) const
{
	cvdebug() << "CDatapool::get: '" << key << "' ... ";
	Anymap::const_iterator i = _M_map.find(key);
	if (i == _M_map.end()) {
		cverb << "fail\n";
		stringstream msg;
		msg << "CDatapool: key '" << key << "' not available";
		throw invalid_argument(msg.str());
	}
	return i;
}

NS_MIA_END
