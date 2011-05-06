/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_filter_fifiof_byslice_hh
#define mia_2d_filter_fifiof_byslice_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory>
#include <mia/2d/2dfilter.hh>
#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(byslice_2dstack_filter)



class C2DBysliceFifoFilter : public mia::C2DImageFifoFilter {
public:
	C2DBysliceFifoFilter(std::string filter);
private:
	void do_push(::boost::call_traits<mia::P2DImage>::param_type x);
	mia::P2DImage do_filter();

	mia::P2DImage m_last_image;
	mia::C2DFilterPluginHandler::ProductPtr m_filter; 
};

NS_END
#endif
