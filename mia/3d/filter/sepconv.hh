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

#ifndef mia_3d_filter_sepconv_hh
#define mia_3d_filter_sepconv_hh

#include <mia/core/spacial_kernel.hh>
#include <mia/3d/filter.hh>

NS_BEGIN(sepconv_3dimage_filter)

class CSeparableConvolute: public mia::C3DFilter {
public:
	CSeparableConvolute(mia::P1DSpacialKernel kx,
			    mia::P1DSpacialKernel ky,
			    mia::P1DSpacialKernel kz);

	template <typename  T>
	CSeparableConvolute::result_type operator () (const mia::T3DImage<T>& data) const;

	template <typename T>
	void fold(std::vector<T>& v, const mia::C1DFilterKernel& kernel)const;

private:
	mia::C3DFilter::result_type do_filter(const mia::C3DImage& image) const;

	mia::P1DSpacialKernel m_kx;
	mia::P1DSpacialKernel m_ky;
	mia::P1DSpacialKernel m_kz;

};


class C3DSeparableConvoluteFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DSeparableConvoluteFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::P1DSpacialKernel m_kx;
	mia::P1DSpacialKernel m_ky;
	mia::P1DSpacialKernel m_kz;
};


class C3DGaussFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DGaussFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	int m_w;
};

NS_END

#endif
