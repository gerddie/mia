/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <memory>
#include <mia/core/filter.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/interpolator.hh>

NS_BEGIN(scale_2dimage_filter)


class CScale: public mia::C2DFilter {
public:
	CScale(const mia::C2DBounds& size, const std::string& interpolator);

	~CScale();

	template <typename  T>
	mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& data) const;

private:
	CScale::result_type do_filter(const mia::C2DImage& image) const;

	const mia::C2DBounds _M_size;
	std::auto_ptr<mia::C2DInterpolatorFactory> _M_ipf;
};



class C2DScaleFilterPlugin: public mia::C2DFilterPlugin {
public:
	C2DScaleFilterPlugin();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mutable int _M_sx;
	mutable int _M_sy;
	std::string _M_interp;
};

NS_END
