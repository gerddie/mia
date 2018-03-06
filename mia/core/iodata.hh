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

#ifndef mia_core_iodata_hh
#define mia_core_iodata_hh

#include <string>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup io

   \brief helper class to derive from for data that can be loaded and stored to a disk.

   Interface halper class for all IO data classes - defines the get/set format functions.
*/
class EXPORT_CORE CIOData
{
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
