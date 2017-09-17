/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_core_delayedparameter_hh
#define mia_core_delayedparameter_hh

#include <string>
#include <stdexcept>


#include <mia/core/defines.hh>
#include <mia/core/datapool.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN


/**
   \ingroup cmdline 
   \brief A parameter proxy object with a key to identify it.  

   This template is used to hold reference to data that may not yet be available and 
   will be/or is stored in the internal data pool. 
   One axample for its usage is to pass parameters to the filters in a filter 
   pipeline that are only created after the filters itself is created.
   \sa CDatapool.
*/
template <typename T>
class TDelayedParameter {
public: 

	TDelayedParameter() = default; 

	/**
	   Assosiate the parameter with its key in the data pool. 
	   At creation time, the data doesn't have to be available in the 
	   data pool. 
	 */
	TDelayedParameter(const std::string& key); 
	
	/**
	   Get the data assosiated with this parameter. Throws \a std::invalid_argument if the 
	   key is not available in the data pool 
	 */
	const T get() const; 

	/**
	   Check if the key is available in the data bool. 
	   \returns true if the key is available
	*/
	bool pool_has_key() const; 

	/**
	   Check if this key is actually valid (i.e. not  empty)
	   \returns true if the key is valid 
	*/
	bool key_is_valid() const; 

	const std::string& get_key() const;
private:
	std::string m_key;
}; 


template <typename T>
TDelayedParameter<T>::TDelayedParameter(const std::string& key):
	m_key(key)
{
}
	

template <typename T>
const T TDelayedParameter<T>::get() const
{
	if (!(CDatapool::instance().has_key(m_key))) {
		throw create_exception<std::invalid_argument>("TDelayedParameter::get(): Key '", m_key, 
						    "' is not available in the data pool");  
	}
	return boost::any_cast<T>(CDatapool::instance().get(m_key)); 
}

template <typename T>
bool TDelayedParameter<T>::pool_has_key() const
{
	return CDatapool::instance().has_key(m_key); 
}


template <typename T>
bool TDelayedParameter<T>::key_is_valid() const
{
	return !m_key.empty(); 
}

template <typename T>
const std::string& TDelayedParameter<T>::get_key() const
{
	return m_key; 
}

NS_MIA_END

#endif
