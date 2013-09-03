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

#ifndef mia_core_traits__hh
#define mia_core_traits__hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup traits
   \brief A trait to obtaine information about of dimensions of a class 

   A template to get an generic dimension vector 
   \tparam T must be a template that defines a type \a dimsize_type
 */
template <typename T>
struct dim_traits {
	/// the trait 
	typedef typename T::dimsize_type dimsize_type; 
};

/**
   \ingroup traits
   \brief Structure to enable the use of the dim_traits class for std::vector 
 */
struct vector_dimsize {
	
	/// contruct the dimsize object 
	vector_dimsize():m_n(0) {
	}

	/// contruct the dimsize object 
	vector_dimsize(size_t n):m_n(n) {
	}
	
	/// \return the dimension 1 as dimension of this vector 
	size_t size() {
		return 1; 
	}

	/// return a read-write reference to the first (and only)dimension
	size_t& operator [] (int /*i*/)
	{
		return m_n; 
	}

	/// return a read-only  reference to the first (and only)dimension
	const size_t& operator [] (int /*i*/) const 
	{
		return m_n; 
	}
private: 
	size_t m_n; 
}; 


/**
   @cond INTERNAL 
   \ingroup traits
   \brief A trait to obtain the dimension of a 1D vector 

   \tparam T template parameter for the std::vector
 */
template <typename T>
struct dim_traits<std::vector<T> > {
	typedef vector_dimsize dimsize_type; 
}; 

/**
   \ingroup traits
   \brief A trait to set the Factory plugin handler cache policy 

   \tparam T factory plug-in to set the trait for
 */

template <typename T>
struct __cache_policy {
	static  bool apply() {
		return false; 
	}
}; 
/// @endcond 




NS_MIA_END

#endif
