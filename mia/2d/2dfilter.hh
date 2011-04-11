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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_filter_hh
#define mia_2d_filter_hh

#include <mia/2d/2DImage.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN

/// 2D image filter 
typedef TImageFilter<C2DImage> C2DFilter;

/// 2D image filter plugin 
typedef TImageFilterPlugin<C2DImage> C2DFilterPlugin;

typedef std::shared_ptr<C2DFilter > P2DFilter;

/// 2D filter plugin handler
typedef THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> > C2DFilterPluginHandler;


/**
   convenience function: run a filter chain on a image 
   @param image input image 
   @param nfilters number of filter definitions following 
   @param filters array of strings defining the filter to be applied 
   @returns the filtered image 
*/
P2DImage  EXPORT_2D run_filter_chain(P2DImage image, size_t nfilters, const char *filters[]);

/**
   convenience function: run a filter chain on a image 
   @param image input image 
   @param filter string defining the filter to be applied 
   @returns the filtered image 
*/
P2DImage  EXPORT_2D run_filter(const C2DImage& image, const char *filter);


/**
   2D Image combiner 
 */
class EXPORT_2D C2DImageCombiner : public TFilter< P2DImage > ,
				   public CProductBase {
public:
	typedef C2DImage plugin_data; 
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


double EXPORT_2D distance(const C2DImage& image1, const C2DImage& image2,  
			  const C2DImageCombiner& measure);


typedef TFactory<C2DImageCombiner> C2DImageCombinerPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DImageCombinerPlugin> > 
C2DImageCombinerPluginHandler;
FACTORY_TRAIT(C2DImageCombinerPluginHandler); 

NS_MIA_END


#endif
