/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/dictmap.hh>
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


class C2DSobelFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DSobelFilterPlugin();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	enum EGradientDirection {gd_x, gd_y, gd_undefined};

	static const mia::TDictMap<EGradientDirection>::Table dir_dict[];
	static const mia::TDictMap<C2DSobelFilterPlugin::EGradientDirection> Ddirection;

	EGradientDirection m_direction; 
};



NS_END

