/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef mia_core_dictmap_hh
#define mia_core_dictmap_hh

#include <string>
#include <set>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <cassert>


#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

/**
   A mapper from emums to string values. - usefull for names flags 
*/
template <typename T>
class TDictMap {
public: 
	/**
	   The initialisation table. The last entry must have the name pointer pointing to 0. 
	*/
	typedef struct { 
		const char *name; 
		const T value; 
	} Table; 
	
	/** Create the map by providing an initialisation map
	    \param table the initialisation table, needs to be terminated by a {NULL, some_value} pair. 
	    \param last_is_default \a true if the value provided with the NULL name is the default return value 
	    if unknown names are given. If false, an unknown name in get_value() will throw 
	    \a std::invalid_argument exception. 
	    
	 */
	TDictMap(const Table *table, bool last_is_default = false); 
	
	/**
	   \param name 
	   \returns corresponding flag 
	   \remark throws std::invalid_argument if the name is unknown 
	*/
	T get_value(const char *name) const; 
	
	/**
	   \param name 
	   \returns corresponding flag 
	   \remark throws std::invalid_argument if the value is unknown 
	*/
	const char *get_name(T value) const; 
	
	/// \returns a set of all available names
	const std::set<std::string> get_name_set() const; 
	
private: 
	typedef std::map<std::string, T> TMap; 
	typedef std::map<T, std::string> TBackMap; 

	bool      _M_last_is_default; 
	TMap      _M_table;
        TBackMap  _M_back_table;
	T         _M_default; 

	struct Insert {
		Insert(	std::set<std::string>& result ):_M_result(result) {
		}
		void operator() (const typename TMap::value_type& v) {
			_M_result.insert(v.first); 
		}
	private: 
		std::set<std::string>& _M_result; 
	};
}; 


template <typename T>
TDictMap<T>::TDictMap(const Table *table, bool last_is_default): 
	_M_last_is_default(last_is_default)
{
	assert(table); 
	const Table *t = table; 
	while (t->name){
		if (!_M_table.insert(typename TMap::value_type(t->name, t->value)).second)
			throw std::invalid_argument(std::string("TDictMap<T>::TDictMap:'") + 
						    std::string(t->name) + 
						    std::string("' already present")); 
		_M_back_table.insert(typename TBackMap::value_type(t->value, t->name)); 
		++t; 
	}
	_M_default = t->value; 
}

template <typename T>
T TDictMap<T>::get_value(const char *name) const
{
	typename TMap::const_iterator i = _M_table.find(name); 
	if (i == _M_table.end()) {
		if (!_M_last_is_default) 
			throw std::invalid_argument(std::string("TDictMap<T>::get_value: unknown key '")+
						    std::string(name) + std::string("' provided")); 
		else 
			return _M_default; 
	}
	return i->second; 
}

template <typename T>
const char *TDictMap<T>::get_name(T value) const
{
	typename TBackMap::const_iterator i = _M_back_table.find(value); 
	
	if (i == _M_back_table.end()) {
		if (!_M_last_is_default || (_M_default != value)) 
			THROW(std::invalid_argument, "TDictMap<T>::get_name: unknown value"<< value << " provided"); 
		else
			return "(default)"; 
	}
	return i->second.c_str();
}

template <typename T>
const std::set<std::string> TDictMap<T>::get_name_set() const
{
	std::set<std::string> result; 
	std::for_each(_M_table.begin(),_M_table.end(), Insert(result)); 
	return result; 
}


NS_MIA_END
#endif
