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

#include <mia/core/cost.hh>

NS_MIA_BEGIN

template <typename T, typename V>
TCost<T,V>::~TCost()
{

}

template <typename T, typename V>
double TCost<T,V>::value(const T& a, const T& b) const
{
	return do_value(a,b); 
}

template <typename T, typename V>
void TCost<T,V>::evaluate_force(const T& a, const T& b, float scale, V& force) const
{
	do_evaluate_force(a, b, scale, force); 
}

NS_MIA_END
