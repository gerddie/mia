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

#include <mia/core/cost.hh>

NS_MIA_BEGIN

template <typename T, typename V>
TCost<T,V>::~TCost()
{

}

template <typename T, typename V>
double TCost<T,V>::value(const T& a) const
{
	return do_value(a, *m_reference); 
}

template <typename T, typename V>
double TCost<T,V>::evaluate_force(const T& a, V& force) const
{
	return do_evaluate_force(a, *m_reference, force); 
}

template <typename T, typename V>
void TCost<T,V>::set_reference(const T& ref)
{
	m_reference.reset(new RData(ref));
	post_set_reference(ref); 
}

template <typename T, typename V>
void TCost<T,V>::post_set_reference(const T& /*ref*/)
{
}

NS_MIA_END
