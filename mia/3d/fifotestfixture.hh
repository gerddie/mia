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

#ifndef mia_3d_fifotestfixture_hh
#define mia_3d_fifotestfixture_hh

#include <vector>
#include <mia/3d/2dimagefifofilter.hh>

NS_MIA_BEGIN

typedef std::vector<P2DImage> C2DImageStack;

/**
   @ingroup test
   @brief a fixture to test FIFO filtering

   @tparam A the input data type
   @tparam B the extected data type
   @param input_data the input data as flat array
   @param test_data the expected result data as flat array
   @param size the size of the input slices
   @param slices the number of slices to process.
*/
struct EXPORT_3D fifof_Fixture  {
       template <typename A, typename B>
       void prepare(const A *input_data, const B *test_data, const C2DBounds& size, size_t slices);
       void call_test( C2DImageFifoFilter& filter)const ;

       C2DImageStack m_in_data;
       C2DImageStack m_test_data;
};

template <typename A, typename B>
void fifof_Fixture::prepare(const A *input_data, const B *test_data, const C2DBounds& size, size_t n_slices)
{
       m_in_data.clear();
       m_test_data.clear();
       size_t slice_size = size.x * size.y;
       cvdebug() << "the test image size is " << size << "\n";

       for (size_t i = 0; i < n_slices; ++i, input_data += slice_size, test_data += slice_size) {
              m_in_data.push_back(P2DImage(new T2DImage<A>(size, input_data)));
              m_test_data.push_back(P2DImage(new T2DImage<B>(size, test_data)));
       }
}

NS_MIA_END

#endif
