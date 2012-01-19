/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_filter_fifiof_gauss_hh
#define mia_2d_filter_fifiof_gauss_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iomanip>
#include <limits>
#include <numeric>

#include <boost/cast.hpp>
#include <boost/lambda/lambda.hpp>


#include <mia/core/spacial_kernel.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(gauss_2dstack_filter)


class C2DGaussFifoFilter: public mia::C2DImageFifoFilter {
public:
	typedef mia::C2DImage *result_type;

	C2DGaussFifoFilter(size_t hw);

	template <typename T>
	mia::C2DImage *operator()(const mia::T3DImage<T>& buffer) const ;

	template <typename T>
	mia::C2DImage *operator()(const mia::T2DImage<T>& input);
private:
	virtual void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x);
	virtual void do_push(::boost::call_traits<mia::P2DImage>::param_type x);
	virtual mia::P2DImage do_filter();
	virtual void post_finalize();
	void shift_buffer();
	size_t m_hw;
	mia::P2DFilter m_gauss2d;
	mia::C2DBounds m_slice_size;
	mia::P1DSpacialKernel m_1dfilter;
	std::auto_ptr<mia::C3DFImage> m_buffer;
	std::auto_ptr<mia::C3DImage> m_dummy;
};

NS_END

#endif
