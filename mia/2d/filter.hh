/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_2d_filter_hh
#define mia_2d_filter_hh

#include <mia/2d/image.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>
#include <mia/template/filter_chain.hh>
#include <mia/template/combiner.hh>

NS_MIA_BEGIN

/**   
      \ingroup filtering 
      2D image filter 
*/
typedef TDataFilter<C2DImage> C2DFilter;

/**
   \ingroup filtering 
   2D image filter plugin 
*/
typedef TDataFilterPlugin<C2DImage> C2DFilterPlugin;

/**   
      \ingroup filtering 
      shared pointer type for the C2DFilter class 
*/
typedef std::shared_ptr<C2DFilter > P2DFilter;

/**
   \ingroup filtering 
   2D filter plugin handler
*/
typedef THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> > C2DFilterPluginHandler;

/// @cond NEVER 
FACTORY_TRAIT(C2DFilterPluginHandler); 
/// @endcond 

/**
   Convenience function to create a filter from its string description
 */
inline P2DFilter produce_2dimage_filter(const char* descr) 
{
	return C2DFilterPluginHandler::instance().produce(descr); 
}


/**
   \ingroup filtering 
   2D filter chain to apply various filters in one run 
*/
typedef TFilterChain<C2DFilterPluginHandler> C2DImageFilterChain; 
/**
   \ingroup filtering 
   
   convenience function: run a filter chain on an image 
   @param image input image 
   @param nfilters number of filter definitions following 
   @param filters array of strings defining the filter to be applied 
   @returns the filtered image 
*/
P2DImage  EXPORT_2D run_filter_chain(P2DImage image, size_t nfilters, const char *filters[])
	__attribute__((deprecated));

/**
   \ingroup filtering 
   
   convenience function: run a filter chain on an image 
   @param image input image 
   @param filters vector of strings defining the filter to be applied 
   @returns the filtered image 
*/
P2DImage  EXPORT_2D run_filter_chain(P2DImage image, const std::vector<const char *>& filters);

/**
   \ingroup filtering 
   convenience function: create and run a filter on an image 
   @param image input image 
   @param filter string defining the filter to be applied 
   @returns the filtered image 
*/
P2DImage  EXPORT_2D run_filter(const C2DImage& image, const char *filter);

/**
   \ingroup filtering 
   convenience function: create and run a filter on an image 
   @param image input image 
   @param filter string defining the filter to be applied 
   @returns the filtered image 
*/
inline P2DImage  EXPORT_2D run_filter(P2DImage image, const char *filter)
{
	return run_filter(*image, filter); 
}


/**
   \ingroup filtering 
   \brief 2D Image combiner 
   
   A class to provides the base for operations that combine two images to create a new image
 */
typedef TImageCombiner< C2DImage > C2DImageCombiner; 
typedef std::shared_ptr<C2DImageCombiner> P2DImageCombiner; 
typedef TFactory<C2DImageCombiner> C2DImageCombinerPlugin;

/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DImageCombinerPlugin> > 
        C2DImageCombinerPluginHandler;
FACTORY_TRAIT(C2DImageCombinerPluginHandler); 

class EXPORT_2D C2DCombinerPluginHandlerTestPath {
public: 
	C2DCombinerPluginHandlerTestPath(); 
}; 


NS_MIA_END


#endif
