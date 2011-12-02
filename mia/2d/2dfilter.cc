/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/*
  LatexBeginPluginSection{2D image filters}
  \label{sec:filter2d}
  
  2D image filters take a constant reference to a C2DImage as parameter and 
  return the result in a shared\_ptr P2DImage. 
  After constructions, filters work like functions, i.e. the output image is only dependent 
  on the input image and the construction parameters. 
  
  Filters are created by the C2DFilterPluginHandler singleton.

  LatexEnd
*/

/*
  LatexBeginPluginSection{2D image combiner}
  \label{sec:combiner2d}
  
  2D image combiners take two constant references to a C2DImage as parameter and 
  return the result in a shared\_ptr P2DImage. 
  After constructions, combinerss work like functions, i.e. the output image is only dependent 
  on the input images and the construction parameters. 

  Combiners are created by the C2DImageCombinerPluginHandler singleton. 

  LatexEnd
*/



#include <mia/core/export_handler.hh>

#include <mia/2d/2dfilter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/2d/shape.hh>

NS_MIA_BEGIN
using namespace boost;


C2DImageCombiner::~C2DImageCombiner()
{
}

C2DImageCombiner::result_type C2DImageCombiner::combine( const C2DImage& a, const C2DImage& b) const
{
	return do_combine(a,b);
}

P2DImage  EXPORT_2D run_filter_chain(P2DImage image, size_t nfilters, const char *filters[])
{
	const C2DFilterPluginHandler::Instance& ff = C2DFilterPluginHandler::instance();
	for (size_t i = 0; i < nfilters; ++i)  {
		auto f = ff.produce(filters[i]);
		if (!f)
			THROW(invalid_argument, "unable to create filter from '" <<filters[i]<<"'");
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
			THROW(invalid_argument, "unable to create filter from '" <<*fd<<"'");
		image = f->filter(*image);
	}
	return image; 
}

P2DImage  EXPORT_2D run_filter(const C2DImage& image, const char *filter)
{
	auto f = C2DFilterPluginHandler::instance().produce(filter);
	if (!f)
		THROW(invalid_argument, "unable to create filter from '" <<filter<<"'");
	return f->filter(image);
}

using boost::filesystem::path; 
C2DFilterPluginHandlerTestPath::C2DFilterPluginHandlerTestPath()
{
	C2DShapePluginHandlerTestPath filter_test_path;
	list< path> sksearchpath; 
	sksearchpath.push_back( path(MIA_BUILD_ROOT"/mia/2d/filter"));
	C2DFilterPluginHandler::set_search_path(sksearchpath); 
}

template class TImageFilter<C2DImage>;
template class TImageFilterPlugin<C2DImage>;

template class TPlugin<C2DImage, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> >;
template class TFactoryPluginHandler<C2DFilterPlugin>;
template class TPluginHandler<C2DFilterPlugin>;

template class TFilterChain<C2DFilterPluginHandler>; 

EXPLICIT_INSTANCE_HANDLER(C2DImageCombiner);

NS_MIA_END
