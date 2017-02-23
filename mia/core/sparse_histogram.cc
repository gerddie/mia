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

#include <mia/core/sparse_histogram.hh>

NS_MIA_BEGIN

using std::vector;
using std::pair;
using std::make_pair;

CSparseHistogram::CSparseHistogram():
        m_shift(0), 
        m_pixeltype(it_none)
{
}

CSparseHistogram::Compressed CSparseHistogram::get_compressed_histogram()const
{
        int nonzero_bins = 0;
        for (auto b: m_histogram) {
                if (b > 0)
                        ++nonzero_bins; 
        }

        Compressed result;
        result.reserve(nonzero_bins);
        for (unsigned i = 0; i < m_histogram.size(); ++i) {
                if (m_histogram[i] != 0)
                        result.push_back(make_pair(i - m_shift, m_histogram[i])); 
        }
        return result; 
}

EXPORT_CORE  std::ostream& operator << (std::ostream& os, const std::pair<short, unsigned long>& pair)
{
	os << "[" << pair.first << ": " << pair.second << "]";
	return os; 
}


NS_MIA_END
