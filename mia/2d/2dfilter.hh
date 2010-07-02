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

#ifndef mia_2d_filter_hh
#define mia_2d_filter_hh

#include <mia/2d/2DImage.hh>
#include <mia/core/factory.hh>
#include <mia/core/filter.hh>
#include <boost/any.hpp>

NS_MIA_BEGIN

typedef TImageFilter<C2DImage> C2DFilter;
typedef TImageFilterPlugin<C2DImage> C2DFilterPlugin;

typedef SHARED_PTR(C2DFilter) P2DFilter;


class EXPORT_2D C2DImageCombiner : public TFilter< SHARED_PTR( ::boost::any ) > ,
				   public CProductBase {
 public:
	virtual ~C2DImageCombiner();

	result_type combine( const C2DImage& a, const C2DImage& b) const;
private:
	virtual result_type do_combine( const C2DImage& a, const C2DImage& b) const = 0;
};


P2DImage run_filter_chain(P2DImage image, size_t nfilters, const char *filters[]);
P2DImage run_filter(const C2DImage& image, const char *filter);

double EXPORT_2D distance(const C2DImage& image1, const C2DImage& image2,  const C2DImageCombiner& measure);

//typedef TFactory<C2DFilter, C2DImage, filter_type> C2DFilterPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DFilterPlugin> > C2DFilterPluginHandler;


typedef TFactory<C2DImageCombiner, C2DImage, combiner_type> C2DImageCombinerPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DImageCombinerPlugin> > C2DImageCombinerPluginHandler;

NS_MIA_END


#endif
