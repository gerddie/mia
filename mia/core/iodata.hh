/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_core_iodata_hh
#define mia_core_iodata_hh

#include <string>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   Interface halper class for all IO data classes - defines the get/set format
*/
class EXPORT_CORE CIOData {
public:
	/**
	   Set the file format, the data was loaded from
	   \param format
	 */
	void set_source_format(const std::string& format);

	/// \returns the file format the data was loaded from
	const std::string& get_source_format() const;

private:
	std::string m_source_format;
};

NS_MIA_END

#endif
