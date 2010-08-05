/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Evolutionary Anthropoloy
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

/*! \brief basic type of a plugin handler

A bandpass filter

\file bandpass_2dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/2dfilter.hh>


#include <limits>

NS_BEGIN(SeparableConvolute_2dimage_filter)

class CSeparableConvolute: public mia::C2DFilter {
public:
	CSeparableConvolute(mia::C1DSpacialKernelPlugin::ProductPtr kx,
			    mia::C1DSpacialKernelPlugin::ProductPtr ky);

	template <typename  T>
	CSeparableConvolute::result_type operator () (const mia::T2DImage<T>& data) const;

	template <typename T>
	void fold(std::vector<T>& v, const mia::C1DFilterKernel& kernel)const;

private:
	CSeparableConvolute::result_type do_filter(const mia::C2DImage& image) const;

	mia::C1DSpacialKernelPlugin::ProductPtr _M_kx;
	mia::C1DSpacialKernelPlugin::ProductPtr _M_ky;
};


class C2DSeparableConvoluteFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DSeparableConvoluteFilterPlugin();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:

	virtual void prepare_path() const;

	std::string _M_kx;
	std::string _M_ky;
};

class C2DGaussFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DGaussFilterPlugin();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:
	int _M_w;
};

NS_END

