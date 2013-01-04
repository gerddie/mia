/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/export_handler.hh>

#include <mia/3d/filter.hh>
#include <mia/core/combiner.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

using namespace boost;


using boost::filesystem::path; 
C3DFilterPluginHandlerTestPath::C3DFilterPluginHandlerTestPath()
{
	CPathNameArray searchpath; 
	searchpath.push_back( path(MIA_BUILD_ROOT"/mia/3d/filter"));
	C3DFilterPluginHandler::set_search_path(searchpath); 
}


C3DImageCombiner::~C3DImageCombiner()
{
}

C3DImageCombiner::result_type C3DImageCombiner::combine( const C3DImage& a,
							 const C3DImage& b) const
{
	return do_combine(a,b);
}


template<> const  char * const 
TPluginHandler<C3DFilterPlugin>::m_help = 
   "These plug-ins provide 3D image filters. Unless otherwise noted, "
   "they take a gray scale image of abitrary pixel type as input, "
   "process it and hand it to the next filter in the pipeline." 
; 

template<> const  char * const 
TPluginHandler<TFactory<C3DImageCombiner>>::m_help = 
    "These plug-ins provide 3D image combiners that merge two gray scale image."; 


P3DImage  EXPORT_3D run_filter(const C3DImage& image, const char *filter)
{
	auto f = C3DFilterPluginHandler::instance().produce_unique(filter);
	return f->filter(image); 
}

P3DImage  EXPORT_3D run_filter_chain(P3DImage image, const std::vector<const char *>& filters)
{
	const auto& fh = C3DFilterPluginHandler::instance();
	for (auto fd = filters.begin(); fd != filters.end(); ++fd) {
		auto f = fh.produce(*fd);
		if (!f)
			throw create_exception<std::invalid_argument>( "unable to create filter from '", *fd , "'");
		image = f->filter(*image);
	}
	return image; 
}

template class TFilterChain<C3DFilterPluginHandler>; 
template class TDataFilter<C3DImage>;
template class TDataFilterPlugin<C3DImage>;
template class TPlugin<C3DImage, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<C3DFilterPlugin> >;
template class TFactoryPluginHandler<C3DFilterPlugin>;
template class TPluginHandler<C3DFilterPlugin>;
template class TPlugin<C3DImage, combiner_type>;
template class TFactory<C3DImageCombiner>;
template class THandlerSingleton<TFactoryPluginHandler<C3DImageCombinerPlugin> >;
template class TFactoryPluginHandler<C3DImageCombinerPlugin>;
template class TPluginHandler<C3DImageCombinerPlugin>;


NS_MIA_END
