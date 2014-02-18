/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <sstream>
#include <mia/core/plugin_base.hh>

NS_MIA_BEGIN

template <typename D, typename T>
TPlugin<D, T>::TPlugin(const char* name):
	CPluginBase(name)
{
}

template <typename D, typename T>
void TPlugin<D, T>::get_help(std::ostream& os) const
{
	os << "\nPlugin: "<< get_name() << "\n";
	os << " Data: " << D::data_descr << "\n"
	   << " Type: " << T::type_descr << "\n";
	os << " Description: " << get_descr() << "\n";
	CPluginBase::get_help(os);
	os  << "\n";
}

template <typename D, typename T>
const std::string TPlugin<D, T>::get_long_name() const
{
	std::stringstream msg;
	msg << get_name() << '-' << D::data_descr << '-' << T::type_descr;
	return msg.str();
}

template <typename D, typename T>
::boost::filesystem::path TPlugin<D, T>::search_path()
{
	return ::boost::filesystem::path(D::data_descr)/ 
		::boost::filesystem::path(T::type_descr);
}


NS_MIA_END


