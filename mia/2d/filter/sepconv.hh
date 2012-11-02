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
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filter.hh>


#include <limits>

NS_BEGIN(SeparableConvolute_2dimage_filter)

class CSeparableConvolute: public mia::C2DFilter {
public:
	CSeparableConvolute(mia::P1DSpacialKernel kx,
			    mia::P1DSpacialKernel ky);

	template <typename  T>
	CSeparableConvolute::result_type operator () (const mia::T2DImage<T>& data) const;

	template <typename T>
	void fold(std::vector<T>& v, const mia::C1DFilterKernel& kernel)const;

private:
	CSeparableConvolute::result_type do_filter(const mia::C2DImage& image) const;

	mia::P1DSpacialKernel m_kx;
	mia::P1DSpacialKernel m_ky;
};


class C2DSeparableConvoluteFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DSeparableConvoluteFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::P1DSpacialKernel m_kx;
	mia::P1DSpacialKernel m_ky;
};

class C2DGaussFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DGaussFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	int m_w;
};

NS_END

