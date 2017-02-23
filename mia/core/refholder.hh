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
#ifndef mia_core_refholder_hh
#define mia_core_refholder_hh

#include <mia/core/defines.hh>
#include <memory> 

NS_MIA_BEGIN

/**
   \ingroup misc 

   \brief A class to make a const reference not so const 
   
   This is a little hepler class to make it possible to change what 
   a constant reference member variable points to after construction of a class.
   \tparam T the type a reference is used form 
 */

template <typename T> 
class TRefHolder {
public: 
	/// pointer type of this reference holder.  
	typedef std::shared_ptr<TRefHolder<T> > Pointer; 
	
	/// the type of the actual reference this class holds
	typedef const T& const_reference; 
	
	/**
	   Constructor 
	   \param r the const reference to be hold 
	 */
	TRefHolder(const_reference r):
		m_r(r){}
	
	/// returns the reference variable to allow seemless usage 
	operator const_reference(){return m_r;}
private: 
	const_reference m_r; 
}; 

NS_MIA_END

#endif 
