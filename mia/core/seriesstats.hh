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

#ifndef mia_2d_seriesstats_hh
#define mia_2d_seriesstats_hh


#include <mia/core/filter.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   \brief data structure to store te results of a statistical analyis of images
 */
struct SIntensityStats {
       /// Sum of all values
       double sum;
       /// Sum of the squares of all values
       double sumsq;
       /// mean of all values
       double mean;
       /// variation of the values
       double variation;
       /// minimum value
       double min;
       /// masimum values
       double max;
       /// number of values
       size_t n;
};

/**
   \ingroup misc
   \brief Functor to accumulate statistics of data.

   This functior is used to accumulate the statistics over the data various
   containers or images.

 */

class EXPORT_CORE FIntensityStatsAccumulator : public TFilter<void>
{
public:
       FIntensityStatsAccumulator();

       /**
          This function gets called for each entity from the input that the statistics is
          evaluated for.
          \param data the data container to be processed
        */
       template <typename Container>
       void operator () ( const Container& data);


       /**
          Evaluate the statistics and return it
          \returns the statistical measures of the accumulated data
        */
       const SIntensityStats& get_result() const;
private:
       mutable SIntensityStats m_stats;
       mutable bool m_stats_valid;
};

template <typename Container>
void FIntensityStatsAccumulator::operator () ( const Container& data)
{
       m_stats_valid = false;
       m_stats.n += data.size();

       for (auto i = data.begin(); i != data.end(); ++i) {
              m_stats.sum += *i;
              m_stats.sumsq += *i * *i;

              if (m_stats.min > *i)
                     m_stats.min = *i;

              if (m_stats.max < *i)
                     m_stats.max = *i;
       }
}

NS_MIA_END

#endif
