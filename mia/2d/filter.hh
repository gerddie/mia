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

#ifndef mia_2d_filter_hh
#define mia_2d_filter_hh

#include <mia/2d/image.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>
#include <mia/template/filter_chain.hh>

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

/** @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
struct EXPORT_2D C2DFilterPluginHandlerTestPath {
	C2DFilterPluginHandlerTestPath(); 
}; 
/// @endcond 

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
class EXPORT_2D C2DImageCombiner : public TFilter< P2DImage > ,
				   public CProductBase {
public:
	/// data type for plug-in serachpath component 
	typedef C2DImage plugin_data; 
	/// plug-in type for plug-in serachpath component 
	typedef combiner_type plugin_type; 
	
	virtual ~C2DImageCombiner();
	/**
	   Combine two images by a given operator 
	   @param a 
	   @param b 
	   @returns combined image 
	   
	 */
	result_type combine( const C2DImage& a, const C2DImage& b) const;
private:
	virtual result_type do_combine( const C2DImage& a, const C2DImage& b) const = 0;
};



/// Base class for image combiners 

typedef TFactory<C2DImageCombiner> C2DImageCombinerPlugin;

/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DImageCombinerPlugin> > 
        C2DImageCombinerPluginHandler;
FACTORY_TRAIT(C2DImageCombinerPluginHandler); 

NS_MIA_END


#endif
