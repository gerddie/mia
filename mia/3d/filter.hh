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

#ifndef mia_3d_filter_hh
#define mia_3d_filter_hh

#include <boost/any.hpp>

#include <mia/3d/image.hh>
#include <mia/core/combiner.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/template/filter_chain.hh>
#include <mia/template/combiner.hh>

NS_MIA_BEGIN


/**
   \ingroup filtering 
   @brief Base class for 3D image filters. Filters are implemented as plug-ins.

*/

typedef TDataFilter<C3DImage> C3DFilter;

/**   
      \ingroup filtering 
      @brief The 3D filter plugin type 
*/
typedef TDataFilterPlugin<C3DImage> C3DFilterPlugin;

/**   
      \ingroup filtering 
      @brief The 3D filter plugin handler
*/
typedef THandlerSingleton<TFactoryPluginHandler<C3DFilterPlugin> > C3DFilterPluginHandler;

/**   
      \ingroup filtering 
      @brief The 3D filter shared pointer 
*/
typedef std::shared_ptr<C3DFilter> P3DFilter; 

/**
   \ingroup filtering 
   \brief Base class for plug-ins that combine two 3D images in certain ways
   
   This class is the base class for all combiners that are used to combine two 3D images.  
   The result of the combination can be anything derived from CCombinerResult. 
*/
typedef TImageCombiner< C3DImage > C3DImageCombiner; 
typedef std::shared_ptr<C3DImageCombiner> P3DImageCombiner; 
typedef TFactory<C3DImageCombiner> C3DImageCombinerPlugin;

/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C3DImageCombinerPlugin> > 
        C3DImageCombinerPluginHandler;


/** 
    @cond INTERNAL 
    @ingroup test 
    @brief class to initialize the plug-in path for tests on the uninstalled library 
*/
class EXPORT_3D C3DFilterPluginHandlerTestPath {
public: 
	C3DFilterPluginHandlerTestPath(); 
private: 
	C1DSpacialKernelPluginHandlerTestPath spk_path; 
}; 

class EXPORT_3D C3DCombinerPluginHandlerTestPath {
public: 
	C3DCombinerPluginHandlerTestPath(); 
}; 

/// @endcond 






/// @cond NEVER 
FACTORY_TRAIT(C3DFilterPluginHandler); 
FACTORY_TRAIT(C3DImageCombinerPluginHandler); 
/// @endcond 


/**
   \ingroup filtering 
   @brief Convenience function to create a filter chain from a series of filter descriptions 
   \param chain the descriptions 
   \returns the filter chain as a vector 
   @remark obsolete, should use C3DImageFilterChain
 */

template <typename S>
std::vector<P3DFilter> create_filter_chain(const std::vector<S>& chain)
{
	std::vector<P3DFilter> filters;

	for (typename std::vector<S>::const_iterator i = chain.begin();
	     i != chain.end(); ++i) {
		cvdebug() << "Prepare filter " << *i << std::endl;
		auto filter = C3DFilterPluginHandler::instance().produce(*i);
		if (!filter){
			std::stringstream error;
			error << "Filter " << *i << " not found";
			throw std::invalid_argument(error.str());
		}
		filters.push_back(filter);
	}
	return filters;
}

/**
   \ingroup filtering 
   @brief 3D filter chain to apply various filters in one run 
*/
typedef TFilterChain<C3DFilterPluginHandler> C3DImageFilterChain; 


/**
   Convenience function to create a filter from its string description
 */
inline P3DFilter produce_3dimage_filter(const char* descr) 
{
	return C3DFilterPluginHandler::instance().produce(descr); 
}


/**
   \ingroup filtering 
   @brief convenience function: create and run a filter on an image 
   @param image input image 
   @param filter string defining the filter to be applied 
   @returns the filtered image 
*/
P3DImage  EXPORT_3D run_filter(const C3DImage& image, const char *filter);


/**
   \ingroup filtering 
   
   convenience function: run a filter chain on an image 
   @param image input image 
   @param filters vector of strings defining the filter to be applied 
   @returns the filtered image 
*/
P3DImage  EXPORT_3D run_filter_chain(P3DImage image, const std::vector<const char *>& filters);


NS_MIA_END


#endif
