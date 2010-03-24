/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

\file bandpass_3dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/

#ifndef mia_3d_filter_sepconv_hh
#define mia_3d_filter_sepconv_hh

#include <mia/core/spacial_kernel.hh>
#include <mia/3d/3dfilter.hh>

NS_BEGIN(sepconv_3dimage_filter)

class CSeparableConvolute: public mia::C3DFilter {
public:
	CSeparableConvolute(mia::C1DSpacialKernelPlugin::ProductPtr kx,
			    mia::C1DSpacialKernelPlugin::ProductPtr ky,
			    mia::C1DSpacialKernelPlugin::ProductPtr kz);

	template <typename  T>
	CSeparableConvolute::result_type operator () (const mia::T3DImage<T>& data) const;

	template <typename T>
	void fold(std::vector<T>& v, const mia::C1DFilterKernel& kernel)const;

private:
	mia::C3DFilter::result_type do_filter(const mia::C3DImage& image) const;

	mia::C1DSpacialKernelPlugin::ProductPtr _M_kx;
	mia::C1DSpacialKernelPlugin::ProductPtr _M_ky;
	mia::C1DSpacialKernelPlugin::ProductPtr _M_kz;

};


class C3DSeparableConvoluteFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DSeparableConvoluteFilterPlugin();
	virtual mia::C3DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:

	virtual bool  do_test() const;
	void prepare_path() const;

	std::string _M_kx;
	std::string _M_ky;
	std::string _M_kz;
};


class C3DGaussFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DGaussFilterPlugin();
	virtual mia::C3DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:
	virtual bool do_test() const;
	int _M_w;
};

NS_END

#endif
