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

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/cstkernel.hh>

NS_BEGIN(filter_2dimage_cst)

class EXPORT_2D C2DCst : public mia::C2DFilter {
public:
        C2DCst(const mia::PCST2DImageKernel&  kernel);

	template <class T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& data) const ;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	mia::PCST2DImageKernel m_kernel;
};

class EXPORT_2D C2DFilterPluginFactory: public mia::C2DFilterPlugin {
public:
	C2DFilterPluginFactory();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	mia::PCST2DImageKernel m_kernel;
};

NS_END // end namespace cst_2dimage_filter
