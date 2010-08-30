/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#ifndef mia_2d_fatcost_hh
#define mia_2d_fatcost_hh

#include <mia/core/fatcost.hh>

#include <mia/core/factory.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN

class C2DImageFatCost;
typedef std::shared_ptr<C2DImageFatCost > P2DImageFatCost;

class EXPORT_2D C2DImageFatCost: public TFatCost<C2DTransformation, C2DFVectorfield> {
public:
	C2DImageFatCost(P2DImage src, P2DImage ref, P2DInterpolatorFactory ipf, float weight);

	P2DImageFatCost get_downscaled(const C2DBounds& scale) const;

	C2DBounds get_size() const;
private:
	virtual P2DImageFatCost cloned(P2DImage src, P2DImage ref) const = 0;
};

class EXPORT_2D C2DFatImageCostPlugin: public TFactory<C2DImageFatCost, C2DImage, fatcost_type> {
public:
	C2DFatImageCostPlugin(const char *name);
	P2DImageFatCost create_directly( P2DImage src, P2DImage ref,
					 P2DInterpolatorFactory ipf,
					 float weight);
private:
	virtual C2DFatImageCostPlugin::ProductPtr do_create() const;
	virtual C2DFatImageCostPlugin::ProductPtr do_create(P2DImage src, P2DImage ref,
							    P2DInterpolatorFactory ipf,
							    float weight) const = 0;

	std::string _M_src_name;
	std::string _M_ref_name;
	EInterpolation _M_interpolator;
	float _M_weight;
};

class EXPORT_2D C2DImageFatCostList : public std::vector<P2DImageFatCost> {
public:
	double value() const;

	double evaluate_force(C2DFVectorfield& force) const;

	C2DImageFatCostList get_downscaled(const C2DBounds& scale) const;

	C2DBounds get_size() const;

	void transform(const C2DTransformation& transform);
};

class EXPORT_2D C2DFatImageCostPluginHandlerImpl: public TFactoryPluginHandler<C2DFatImageCostPlugin> {
public:
	C2DFatImageCostPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath);
	P2DImageFatCost create_directly(const std::string& plugin, P2DImage src, P2DImage ref,
					P2DInterpolatorFactory ipf,
					float weight) const;
};

typedef THandlerSingleton<C2DFatImageCostPluginHandlerImpl> C2DFatImageCostPluginHandler;

NS_MIA_END


#endif
