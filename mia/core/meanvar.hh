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


#ifndef mia_core_meanvar_hh
#define mia_core_meanvar_hh

#include <map>
#include <cmath>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   Evaluate the mean and the variance of a series of scalars
   \tparam ForwardIterator
   \param begin
   \param end
   \returns mean as .first and variance as .second of the std::pair.
   \todo could be extended to vector types
 */

template <typename ForwardIterator>
std::pair<double, double> mean_var(ForwardIterator begin, ForwardIterator end)
{
       std::pair<double, double> result;
       result.first  = 0.0;
       result.second = 0.0;
       size_t n = 0;

       while (begin != end)  {
              const double help = *begin;
              result.first += help;
              result.second += help * help;
              ++n;
              ++begin;
       }

       if (n > 0)
              result.first /= n;

       if (n > 1)
              result.second = sqrt((result.second - n * result.first * result.first) / (n - 1));
       else
              result.second = 0.0;

       return result;
}

NS_MIA_END

#endif
