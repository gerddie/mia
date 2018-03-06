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

#include <mia/core/export_handler.hh>

#include <mia/3d/filter.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/template/combiner.cxx>


NS_MIA_BEGIN

template<> const  char *const
TPluginHandler<C3DFilterPlugin>::m_help =
       "These plug-ins provide 3D image filters. Unless otherwise noted, "
       "they take a gray scale image of abitrary pixel type as input, "
       "process it and return the filtered result as a new image. "
       "Filters can be chained by specifying more then one filter description "
       "concated with the '+' sign. for example \n"
       "   bandpass:min=10,max=20+median:w=3+convert:repn=ushort:map=copy'\n"
       "will create a filter chain that first runs a bandpass, then a median "
       "filter and a conversion to unsigned short values by a plain copy."
       ;

template<> const  char *const
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
                     throw create_exception<std::invalid_argument>( "unable to create filter from '", *fd, "'");

              image = f->filter(*image);
       }

       return image;
}

template class TFilterChain<C3DFilterPluginHandler>;
template class TDataFilter<C3DImage>;
template class TDataFilterPlugin<C3DImage>;
template class TPlugin<C3DImage, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<C3DFilterPlugin>>;
template class TFactoryPluginHandler<C3DFilterPlugin>;
template class TPluginHandler<C3DFilterPlugin>;


template class TImageCombiner<C3DImage>;
EXPLICIT_INSTANCE_HANDLER(C3DImageCombiner);

NS_MIA_END
