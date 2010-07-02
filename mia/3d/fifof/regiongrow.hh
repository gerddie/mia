/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_3d_fifof_regiongrow_hh
#define mia_3d_fifof_regiongrow_hh

#include <queue>
#include <iomanip>
#include <limits>

#include <mia/core/probmap.hh>
#include <mia/3d/2dimagefifofilter.hh>
#include <mia/3d/shape.hh>

NS_BEGIN(regiongrow_fifof);

class C2DRegiongrowFifoFilter: public mia::C2DImageFifoFilter {

public:
	C2DRegiongrowFifoFilter(const mia::CProbabilityVector probmap,
				float low, float seed, int cls, int depth);

	typedef int result_type;

	template <typename T>
	int operator ()( const mia::T2DImage<T>& image);
private:
	struct FSeed {
		FSeed(double thresh):_M_thresh(thresh){}
		bool operator () (double x) const {
			return x >= _M_thresh;
		}
	private:
		double _M_thresh;
	};

	void do_push(::boost::call_traits<mia::P2DImage>::param_type x);
	void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x);
	mia::P2DImage do_filter();
	void shift_buffer();
	void seed_env(const mia::C3DBounds& center, std::queue<mia::C3DBounds>& seeds) const;
	void  grow();

	mia::CProbabilityVector _M_probmap;
	float _M_low;
	FSeed _M_seed;
	int   _M_class;
	int   _M_depth;

	mia::C2DBounds   _M_slice_size;
	mia::C3DDImage   _M_in_buffer;
	mia::C3DBitImage _M_out_buffer;
	mia::C2DImageFifoFilter::CShiftSlices _M_ss;
	mia::P3DShape _M_shape;
	size_t _M_slice_emls;
};

NS_END

#endif
