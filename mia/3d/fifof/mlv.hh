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

#ifndef mia_2d_filter_fifiof_mlv_hh
#define mia_2d_filter_fifiof_mlv_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory>
#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(mlv_2dstack_filter)

class C2DMLVnFifoFilter : public mia::C2DImageFifoFilter
{
public:
       C2DMLVnFifoFilter(size_t hwidth);

       typedef mia::C2DImage *result_type;

       template <typename T>
       mia::C2DImage *operator()(const mia::T3DImage<T>& dummy) const;

       template <typename T>
       mia::C2DImage *operator()(const mia::T2DImage<T>& input);

private:
       void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x);
       void do_push(::boost::call_traits<mia::P2DImage>::param_type x);
       mia::P2DImage do_filter();
       void do_shift();
       void evaluate(size_t slice);
       void shift_buffer();
       virtual void post_finalize();

       mia::C2DBounds m_slice_size;

       std::vector<mia::C2DFImage> m_mu;
       std::vector<mia::C2DFImage> m_sigma;
       std::vector<mia::C2DFImage> m_n;
       std::vector<bool> m_evaluated;

       std::vector<float> m_buf1;
       std::vector<float> m_buf2;
       std::unique_ptr<mia::C3DImage> m_prototype;

       mia::C2DFImage m_n_template;

       size_t m_hw;
       size_t m_w;
       size_t m_buf_slice_size;
};

NS_END

#endif
