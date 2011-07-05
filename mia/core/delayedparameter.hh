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


#ifndef mia_core_delayedparameter_hh
#define mia_core_delayedparameter_hh

#include <string>
#include <stdexcept>


#include <mia/core/defines.hh>
#include <mia/core/datapool.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN


/**
   \ingroup infrastructure 

   \brief A parameter proxy object with a key to identify it.  

   Class for a parameter that can be stored in the internal data pool. 
 */
template <typename T>
class TDelayedParameter {
public: 

	/**
	   Assosiate the parameter with its key in the data pool. 
	 */
	TDelayedParameter(const std::string& key); 
	
	/**
	   Get the data assosiated with this parameter. Throws \a invalid_argument if the 
	   key is not available in the data pool 
	 */
	const T get() const; 
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
	if (!(CDatapool::Instance().has_key(m_key))) {
		THROW(std::invalid_argument, "TDelayedParameter::get(): Key '" << m_key 
		      << "' is not availabe in the data pool");  
	}
	return boost::any_cast<T>(CDatapool::Instance().get(m_key)); 
}

NS_MIA_END

#endif
