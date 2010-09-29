/* -*- mona-c++  -*-
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


#ifndef mia_2d_nonrigidregister_hh
#define mia_2d_nonrigidregister_hh


#include <mia/2d/multicost.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_BEGIN

enum EMinimizers {
	min_cg_fr,
	min_cg_pr,
	min_bfgs,
	min_bfgs2,
	min_gd,
	min_undefined
};

/**
   Class for non-rigid registration. 
   The registration approach of this class lies in the optimization of a weighted sum of cost functions. 
   These cost functions may include image based cost functions, penalty functions on the transformation, 
   and others. The registration makes use of a multi-resolution scheme and support various transformation 
   models.  All cost functions implement the C2DFullCost interface.  
   Currently supported minimizers are those available in the GNU scientific library.
*/

class EXPORT_2D C2DNonrigidRegister {
public:
	/**
	   Constructor for the registration tool
	   \param cost cost function model
	   \param minimizer GSL provided minimizer
	   \param transform_type string describing which transformation is supported
	   \param ipf interpolator
	   \param mg_levels multigrisd levels to be used 
	 */

	C2DNonrigidRegister(C2DFullCostList& costs, EMinimizers minimizer,
			    P2DTransformationFactory transform_creation,
			    const C2DInterpolatorFactory& ipf,  size_t mg_levels);
	
	
	~C2DNonrigidRegister();

	/**
	   Run the registration of an image pair. 
	   \param src source (moving) image 
	   \param ref reference (fixed) image 
	   \returns the transformation registering src to ref that minimizes the constructor given 
	   cost function 
	 */
	P2DTransformation  run(P2DImage src, P2DImage ref) const;

private:
	struct C2DNonrigidRegisterImpl *impl;
};

NS_MIA_END

#endif
