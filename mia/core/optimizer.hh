/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
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


#ifndef mia_core_optimizer_hh
#define mia_core_optimizer_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   Base class for all optimizers 
*/

template <template <typename, typename>  class C> 
class TOptimizer {
public: 
	typedef C Cost; 
	virtual ~TOptimizer(); 
	typename C::Transform operator () (const typename C::Force& force) const; 
private: 

	/**
	   This one needs to be implemented to obtaine
	*/
	virtual typename C::Transform apply(const typename C::Force& force) const = 0;
}; 


template <template <typename, typename>  class C> 
TOptimizer<C>::~TOptimizer()
{
}

template <template <typename, typename>  class C> 
typename C::Transform TOptimizer<Cost>::operator () (const typename C::Force& force) const
{
	return apply(force); 
}

NS_MIA_END

#endif
