/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
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

#include <mia/3d/3dfilter.hh>
#include <mia/core/combiner.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace boost;


C3DImageCombiner::~C3DImageCombiner()
{
}

C3DImageCombiner::result_type C3DImageCombiner::combine( const C3DImage& a,
							 const C3DImage& b) const
{
	return do_combine(a,b);
}

template class EXPORT_HANDLER TImageFilter<C3DImage>;

template class TPlugin<C3DImage, filter_type>;
template class TFactory<C3DFilter, C3DImage, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<C3DFilterPlugin> >;
template class TFactoryPluginHandler<C3DFilterPlugin>;
template class TPluginHandler<C3DFilterPlugin>;


template class EXPORT_HANDLER TPlugin<C3DImage, combiner_type>;
template class EXPORT_HANDLER TFactory<C3DImageCombiner, C3DImage, combiner_type>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C3DImageCombinerPlugin> >;
template class TFactoryPluginHandler<C3DImageCombinerPlugin>;
template class TPluginHandler<C3DImageCombinerPlugin>;

NS_MIA_END
