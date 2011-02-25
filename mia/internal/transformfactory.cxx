/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

template <typename T> 
TTransformCreator<T>::TTransformCreator()
{
}

template <typename T> 
typename T::Pointer TTransformCreator<T>::create(const typename T::Size& size) const
{
	typename T::Pointer result = do_create(size);
	result->set_creator_string(get_init_string()); 
	return result; 
}

template <typename T> 
bool TTransformCreator<T>::has_property(const char *property) const
{
	return _M_properties.find(property) != _M_properties.end();
}

template <typename T> 
void TTransformCreator<T>::add_property(const char *property)
{
	_M_properties.insert(property);
}

NS_MIA_END
