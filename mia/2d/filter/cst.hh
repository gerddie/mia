/* -*- mona-c++  -*-
 * Copyright (c) 2006-2007 Gert Wollny <gert.wollny at acm.org> 
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

/*! \brief Filter for filtering in the dual space of the DCT

A filter that converts using the discrete cosine transform DCT applies a kernel and then 
applies the inverse DCT

\author Gert Wollny <gw.fossdev at gmail.com>

*/

#include <limits>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/cstkernel.hh>

NS_BEGIN(filter_2dimage_cst)

class C2DCst : public mia::C2DFilter {
public:
        C2DCst(const mia::PCST2DImageKernel&  kernel); 

	template <class T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& data) const ;
private: 
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	mia::PCST2DImageKernel _M_kernel;
};

class C2DFilterPluginFactory: public mia::C2DFilterPlugin {
public: 
	C2DFilterPluginFactory();
private: 
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const; 
	std::string _M_kernel; 
};

NS_END // end namespace cst_2dimage_filter
