/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Genoa 1999-2017 Gert Wollny
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


#ifndef mia_core_convergence_hh
#define mia_core_convergence_hh

#include <deque>
#include <cstdint>

#include <mia/core/defines.hh>

namespace mia
{

/**
   \brief Class to get a convergence value as mean over various time steps

   This class can is used to measure the mean of a value over a user-defined
   number of steps.


*/


class EXPORT_CORE CConvergenceMeasure
{
public:

       /**
          Contruct the class by giving the number of steps the
          measure should be averaged over.

          \param size default number of steps to take into account

       */

       CConvergenceMeasure(uint32_t size);

       /**
          Add a new value to the measure. If the maximum number of
          values is already in consideration, the oldest value will
          be dropped.
        */

       void push(double value);

       /// 	   \returns current average of the measure
       double value() const;


       /** \returns the (linear) rate of change as linear regression of
           the values currently in the buffer.
       */
       double rate() const;

       ///  \returns the number of values currently in the buffer
       uint32_t fill() const;

       /// \returns whether the buffer is at maximal expected capacity
       bool is_full_size() const;

private:
       std::deque<double> m_v;
       uint32_t m_size;
};

}; // namespace mia

#endif
