/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_3d_filter_hh
#define mia_3d_filter_hh

#include <boost/any.hpp>

#include <mia/3d/3DImage.hh>
#include <mia/core/combiner.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN


/**
   Base class for 3D image filters. Real filters are implemented as implemented plug-ins.
 */

typedef TImageFilter<C3DImage> C3DFilter;
typedef TImageFilterPlugin<C3DImage> C3DFilterPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C3DFilterPlugin> > C3DFilterPluginHandler;
typedef std::shared_ptr<C3DFilter> P3DFilter; 

class EXPORT_3D C3DImageCombiner : public TFilter< PCombinerResult >, public CProductBase {
public:
	typedef C3DImage plugin_data; 
	typedef combiner_type plugin_type; 
	
	virtual ~C3DImageCombiner();

	result_type combine( const C3DImage& a, const C3DImage& b) const;
private:
	virtual result_type do_combine( const C3DImage& a, const C3DImage& b) const = 0;
};

typedef TFactory<C3DImageCombiner> C3DImageCombinerPlugin;
typedef std::shared_ptr<C3DImageCombiner > P3DImageCombiner;
typedef THandlerSingleton<TFactoryPluginHandler<C3DImageCombinerPlugin> > C3DImageCombinerPluginHandler;



/**
   Convenience function to create a filter chain from a series of filter descriptions 
   \param chain the descriptions 
   \returns the filter chain as a vector 
 */

template <typename S>
std::vector<C3DFilterPlugin::ProductPtr> create_filter_chain(const std::vector<S>& chain)
{
	std::vector<C3DFilterPlugin::ProductPtr> filters;

	for (typename std::vector<S>::const_iterator i = chain.begin();
	     i != chain.end(); ++i) {
		cvdebug() << "Prepare filter " << *i << std::endl;
		C3DFilterPlugin::ProductPtr filter = C3DFilterPluginHandler::instance().produce(*i);
		if (!filter){
			std::stringstream error;
			error << "Filter " << *i << " not found";
			throw std::invalid_argument(error.str());
		}
		filters.push_back(filter);
	}
	return filters;
}


NS_MIA_END


#endif
