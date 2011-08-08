/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#ifndef mia_3d_fatcost_hh
#define mia_3d_fatcost_hh

#include <mia/core/fatcost.hh>

#include <mia/core/factory.hh>
#include <mia/3d/3DImage.hh>
#include <mia/3d/transform.hh>

NS_MIA_BEGIN

class C3DImageFatCost;
typedef std::shared_ptr<C3DImageFatCost > P3DImageFatCost;

class EXPORT_3D C3DImageFatCost: public TFatCost<C3DTransformation, C3DFVectorfield> {
public:
	typedef  C3DImage plugin_data; 
	typedef  fatcost_type plugin_type; 

	C3DImageFatCost(P3DImage src, P3DImage ref, float weight);

	P3DImageFatCost get_downscaled(const C3DBounds& scale) const;

	C3DBounds get_size() const;
private:
	virtual P3DImageFatCost cloned(P3DImage src, P3DImage ref) const = 0;
};

class EXPORT_3D C3DFatImageCostPlugin: public TFactory<C3DImageFatCost> {
public:
	C3DFatImageCostPlugin(const char *name);
	C3DImageFatCost *create_directly( P3DImage src, P3DImage ref, P3DInterpolatorFactory ipf, float weight);
private:
	virtual C3DImageFatCost *do_create() const;
	virtual C3DImageFatCost *do_create(P3DImage src, P3DImage ref,
					   P3DInterpolatorFactory ipf, float weight) const = 0;

	std::string m_src_name;
	std::string m_ref_name;
	float m_weight;
	PSplineKernel m_interpolator;
};

class EXPORT_3D C3DImageFatCostList : public std::vector<P3DImageFatCost> {
public:
	double value() const;

	double evaluate_force(C3DFVectorfield& force) const;

	C3DImageFatCostList get_downscaled(const C3DBounds& scale) const;

	C3DBounds get_size() const;

	void transform(const C3DTransformation& transform);
};

class EXPORT_3D C3DFatImageCostPluginHandlerImpl: public TFactoryPluginHandler<C3DFatImageCostPlugin> {
public:
	C3DFatImageCostPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath);
	C3DImageFatCost *create_directly(const std::string& plugin, P3DImage src, P3DImage ref,
					P3DInterpolatorFactory ipf, float weight) const;
};

typedef THandlerSingleton<C3DFatImageCostPluginHandlerImpl> C3DFatImageCostPluginHandler;

NS_MIA_END


#endif
