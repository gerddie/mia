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
	os << " Type: " << T::value << "\n"
	   << " Data: " << D::type_descr << "\n";
	os << " Description: " << get_descr() << "\n";
	CPluginBase::get_help(os);
	os  << "\n";
}

template <typename D, typename T>
const std::string TPlugin<D, T>::get_long_name() const
{
	std::stringstream msg;
	msg << get_name() << '-' << T::value << '-' << D::type_descr;
	return msg.str();
}

template <typename D, typename T>
::boost::filesystem::path TPlugin<D, T>::search_path()
{
	return ::boost::filesystem::path(T::value) /
		::boost::filesystem::path(D::type_descr);
}


NS_MIA_END


