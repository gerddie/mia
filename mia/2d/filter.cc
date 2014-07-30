/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/core/export_handler.hh>

#include <mia/2d/filter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/2d/shape.hh>

#include <mia/template/combiner.hh>
#include <mia/template/combiner.cxx>

NS_MIA_BEGIN
using namespace boost;
using std::invalid_argument; 

P2DImage  EXPORT_2D run_filter_chain(P2DImage image, size_t nfilters, const char *filters[])
{
	const C2DFilterPluginHandler::Instance& ff = C2DFilterPluginHandler::instance();
	for (size_t i = 0; i < nfilters; ++i)  {
		auto f = ff.produce(filters[i]);
		if (!f)
			throw create_exception<invalid_argument>("unable to create filter from '", filters[i], "'");
		image = f->filter(*image);
	}
	return image;
}

P2DImage  EXPORT_2D run_filter_chain(P2DImage image, const std::vector<const char *>& filters)
{
	const auto& fh = C2DFilterPluginHandler::instance();
	for (auto fd = filters.begin(); fd != filters.end(); ++fd) {
		auto f = fh.produce(*fd);
		if (!f)
			throw create_exception<invalid_argument>("unable to create 2D filter from '", *fd, "'");
		image = f->filter(*image);
	}
	return image; 
}

P2DImage  EXPORT_2D run_filter(const C2DImage& image, const char *filter)
{
	auto f = C2DFilterPluginHandler::instance().produce(filter);
	if (!f)
		throw create_exception<invalid_argument>("unable to create filter from '", filter, "'");
	return f->filter(image);
}

template<> const  char * const 
TPluginHandler<C2DFilterPlugin>::m_help = 
	"These plug-ins provide 2D image filters. Unless otherwise noted, "
	"they take a gray scale image of abitrary pixel type as input, "
	"process it and return the result as newly created image. " 
	"Filters can be chained by specifying more then one filter description "
	"concated with the '+' sign. for example \n"
	"   bandpass:min=10,max=20+median:w=3+convert:repn=ushort:map=copy'\n"
	"will create a filter chain that first runs a bandpass, then a median "
	"filter and a conversion to unsigned short values by a plain copy."
; 

template<> const  char * const 
TPluginHandler<TFactory<C2DImageCombiner>>::m_help = 
    "These plug-ins provide 2D image combiners that merge two gray scale image."; 


template class TDataFilter<C2DImage>;
template class TDataFilterPlugin<C2DImage>;
template class TPlugin<C2DImage, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> >;
template class TFactoryPluginHandler<C2DFilterPlugin>;
template class TPluginHandler<C2DFilterPlugin>;
template class TFilterChain<C2DFilterPluginHandler>; 

template class TImageCombiner<C2DImage>; 
EXPLICIT_INSTANCE_HANDLER(C2DImageCombiner);

NS_MIA_END
