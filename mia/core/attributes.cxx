/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

NS_MIA_BEGIN 

template <typename T>
bool TTranslator<T>::register_for(const std::string& key)
{
	TTranslator<T> * me = new TTranslator<T>();
	if (!me->do_register(key)) {
		delete me; 
		return false; 
	}
	return true; 
}

template <typename T>
PAttribute TTranslator<T>::do_from_string(const std::string& value) const
{
	return PAttribute(new TAttribute<T>(dispatch_attr_string<T>::string2val(value)));
}
NS_MIA_END
