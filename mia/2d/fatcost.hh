/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


/**
   Image cost function class that handles the multiresolution part on its own. 
   \todo This class should probably removed and C2DFullcost be used. 
 */

class EXPORT_2D C2DImageFatCost: public TFatCost<C2DTransformation, C2DFVectorfield> {
public:
	/// Pointer type of this class 
	typedef std::shared_ptr<C2DImageFatCost > Pointer;

	/**
	   Cunstructor to create the cost base 
	   @param src floating image 
	   @param ref reference image 
	   @param ipf interpolation factory 
	   @param weight of this cost function 
	 */
	C2DImageFatCost(P2DImage src, P2DImage ref, P2DInterpolatorFactory ipf, float weight);

	/**
	   return a copy of this cost function that holds downscaled versions of the images, 
	   @param scale inverse scaling parameter
	 */
	Pointer get_downscaled(const C2DBounds& scale) const;


	/**
	   @returns image size of this cost function instance 
	 */
	C2DBounds get_size() const;
private:
	virtual Pointer cloned(P2DImage src, P2DImage ref) const = 0;
};

/// Short for the C2DImageFatCost pointer type 
typedef C2DImageFatCost::Pointer P2DImageFatCost; 

/**
   The base plugin interface for the creation of the C2DImageFatCost cost function 
 */

class EXPORT_2D C2DFatImageCostPlugin: public TFactory<C2DImageFatCost> {
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

	std::string m_src_name;
	std::string m_ref_name;
	EInterpolation m_interpolator;
	float m_weight;
};


/**
   A list of cost functions of type C2DImageFatCost. 
   This class is used to combine different image cost functions 
 */

class EXPORT_2D C2DImageFatCostList : public std::vector<P2DImageFatCost> {
public:

	/// @returns the weightes sum of the cost function values contained in the list 
	double value() const;
	
        /**
	   Evaluate the registration force resulting from the image gradient 
	   @retval force 
	   @returns the weightes sum of the cost function values contained in the list 
	*/
	double evaluate_force(C2DFVectorfield& force) const;

	/**
	   @param scale inverse scaling parameter
	   @returns the doenscaled version of this cost function list 
	 */
	C2DImageFatCostList get_downscaled(const C2DBounds& scale) const;

	/**
	   @returns image size of this cost function instance 
	 */
	C2DBounds get_size() const;

	/**
	   Transform the floating image according to the given transformation 
	   @param transform
	 */
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
