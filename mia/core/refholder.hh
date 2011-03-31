/* -*- mia-c++  -*-
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

#include <mia/core/defines.hh>
#include <memory> 

NS_MIA_BEGIN

/**
   This is a little hepler class to make it possible to change a 
   constant reference member variable after construction of a class. 
   \tparam T the type a reference is used form 
 */

template <typename T> 
class TRefHolder {
public: 
	typedef std::shared_ptr<TRefHolder<T> > Pointer; 
	typedef const T& const_reference; 
	
	/**
	   Constructor 
	 */
	TRefHolder(const_reference r):
		m_r(r){}
	
	/// returns the reference variable to allow seemless usage 
	operator const_reference(){return m_r;}
private: 
	const_reference m_r; 
}; 

NS_MIA_END
