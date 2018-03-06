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


#include <cmath>
#include <limits>
#include <mia/core/kmeans.hh>

NS_MIA_BEGIN

int EXPORT_CORE kmeans_get_closest_clustercenter(const std::vector<double>& classes, size_t l, double val)
{
       double dmin = std::numeric_limits<double>::max();
       int c = 0;

       for (size_t i = 0; i <= l; i++) {
              double d = std::fabs (val - classes[i]);

              if (d < dmin) {
                     dmin = d;
                     c = i;
              };
       };

       return c;
}

NS_MIA_END
