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

#ifndef mia_template_cvd_io_trait_hh
#define mia_template_cvd_io_trait_hh

#include <mia/core/defines.hh>
#include <istream>
#include <ostream>

NS_MIA_BEGIN

/**
   \ingroup traits
   \brief Structure to read and write ND vectors to and from cvd files
 */
template <typename T>
struct NDVectorIOcvd {
       static bool read(std::istream& MIA_PARAM_UNUSED(is), T& MIA_PARAM_UNUSED(value))
       {
              static_assert(sizeof(T) == 0, "NDVectorIOcvd::read needs specialization");
              return false;
       }
       static void write(std::ostream& MIA_PARAM_UNUSED(os), const T& MIA_PARAM_UNUSED(value))
       {
              static_assert(sizeof(T) == 0, "NDVectorIOcvd::read needs specialization");
       }
};
NS_MIA_END

#endif
