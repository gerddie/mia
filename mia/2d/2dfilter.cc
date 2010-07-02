/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif

#include <mia/2d/2dfilter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace boost;


C2DImageCombiner::~C2DImageCombiner()
{
}

C2DImageCombiner::result_type C2DImageCombiner::combine( const C2DImage& a, const C2DImage& b) const
{
	return do_combine(a,b);
}

double distance(const C2DImage& a, const C2DImage& b,  const C2DImageCombiner& measure)
{
	SHARED_PTR(any) result = measure.combine(a,b);
	return any_cast<double>(*result);
}

P2DImage run_filter_chain(P2DImage image, size_t nfilters, const char *filters[])
{
	const C2DFilterPluginHandler::Instance& ff = C2DFilterPluginHandler::instance();
	for (size_t i = 0; i < nfilters; ++i)  {
		C2DFilterPlugin::ProductPtr f = ff.produce(filters[i]);
		if (!f)
			THROW(invalid_argument, "unable to create filter from '" <<filters[i]<<"'");
		image = f->filter(*image);
	}
	return image;
}

P2DImage run_filter(const C2DImage& image, const char *filter)
{
	C2DFilterPlugin::ProductPtr f = C2DFilterPluginHandler::instance().produce(filter);
	if (!f)
		THROW(invalid_argument, "unable to create filter from '" <<filter<<"'");
	return f->filter(image);
}

template class EXPORT_HANDLER TImageFilter<C2DImage>;
template class EXPORT_HANDLER TImageFilterPlugin<C2DImage>;

template class EXPORT_HANDLER TPlugin<C2DImage, filter_type>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> >;
template class EXPORT_HANDLER TFactoryPluginHandler<C2DFilterPlugin>;
template class EXPORT_HANDLER TPluginHandler<C2DFilterPlugin>;

template class EXPORT_HANDLER TPlugin<C2DImage, combiner_type>;
template class EXPORT_HANDLER TFactory<C2DImageCombiner, C2DImage, combiner_type>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C2DImageCombinerPlugin> >;
template class EXPORT_HANDLER TFactoryPluginHandler<C2DImageCombinerPlugin>;
template class EXPORT_HANDLER TPluginHandler<C2DImageCombinerPlugin>;

NS_MIA_END
