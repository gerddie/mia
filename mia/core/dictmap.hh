
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
   \ingroup cmdline
   
   \brief A mapper from emums to string values. - usefull for names flags 

   \todo extend it to also contain help strings for each flag. 
*/
template <typename T>
class TDictMap {
public: 
	/**
	   This map is used to reference the help strings 
	 */
	typedef std::map<T, std::pair<std::string, std::string> > THelpMap; 
	/**
	   The initialisation table. The last entry must have the name pointer pointing to 0. 
	*/
	typedef struct { 
		/// parameter name 
		const char * const name; 
		/// parameter value 
		const T value; 
		/// help text 
		const char * const help; 
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
	   \param value
	   \returns corresponding flag 
	   \remark throws std::invalid_argument if the value is unknown 
	*/
	const char *get_name(T value) const; 
	

	/**
	   \param value
	   \returns corresponding flag 
	   \remark throws std::invalid_argument if the value is unknown 
	*/
	const char *get_help(T value) const; 

	/// \returns a set of all available names
	const std::set<std::string> get_name_set() const; 

	/**
	   \returns start of help map
	*/
	typename THelpMap::const_iterator get_help_begin() const; 

	/**
	   \returns end of help map
	*/
	typename THelpMap::const_iterator get_help_end() const; 
	
private: 

	typedef std::map<std::string, T> TMap; 
	typedef std::map<T, std::string> TBackMap; 

	bool      m_last_is_default; 
	TMap      m_table;
        TBackMap  m_back_table;
	THelpMap  m_help; 
	T         m_default; 

	struct Insert {
		Insert(	std::set<std::string>& result ):m_result(result) {
		}
		void operator() (const typename TMap::value_type& v) {
			m_result.insert(v.first); 
		}
	private: 
		std::set<std::string>& m_result; 
	};
}; 


template <typename T>
TDictMap<T>::TDictMap(const Table *table, bool last_is_default): 
	m_last_is_default(last_is_default)
{
	assert(table); 
	const Table *t = table; 
	while (t->name){
		if (!m_table.insert(typename TMap::value_type(t->name, t->value)).second)
			throw std::invalid_argument(std::string("TDictMap<T>::TDictMap:'") + 
						    std::string(t->name) + 
						    std::string("' already present")); 
		m_back_table.insert(typename TBackMap::value_type(t->value, t->name)); 
		m_help.insert(typename  THelpMap::value_type(t->value, 
							     std::pair<std::string, std::string>(t->name, t->help ? t->help : ""))); 
		++t; 
	}
	m_default = t->value; 
}

template <typename T>
T TDictMap<T>::get_value(const char *name) const
{
	typename TMap::const_iterator i = m_table.find(name); 
	if (i == m_table.end()) {
		if (!m_last_is_default) 
			throw std::invalid_argument(std::string("TDictMap<T>::get_value: unknown key '")+
						    std::string(name) + std::string("' provided")); 
		else 
			return m_default; 
	}
	return i->second; 
}

template <typename T>
const char *TDictMap<T>::get_name(T value) const
{
	auto i = m_back_table.find(value); 
	
	if (i == m_back_table.end()) {
		if (!m_last_is_default || (m_default != value)) 
			throw create_exception<std::invalid_argument>("TDictMap<T>::get_name: unknown value ", value, " provided"); 
		else
			return "(default)"; 
	}
	return i->second.c_str();
}

template <typename T>
const char *TDictMap<T>::get_help(T value) const
{
	auto i = m_help.find(value); 
	if (i == m_help.end()) 
		throw create_exception<std::invalid_argument>("TDictMap<T>::get_help: unknown value ", value, " provided"); 
	return i->second.second.c_str();
}

template <typename T>
const std::set<std::string> TDictMap<T>::get_name_set() const
{
	std::set<std::string> result; 
	std::for_each(m_table.begin(),m_table.end(), Insert(result)); 
	return result; 
}

template <typename T>
typename TDictMap<T>::THelpMap::const_iterator TDictMap<T>::get_help_begin() const
{
	return m_help.begin(); 
}

template <typename T>
typename TDictMap<T>::THelpMap::const_iterator TDictMap<T>::get_help_end() const
{
	return m_help.end(); 
}

NS_MIA_END
#endif
