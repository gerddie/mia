/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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



#ifndef mia_3d_rigidregister_hh
#define mia_3d_rigidregister_hh

#include <mia/core/minimizer.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/transform.hh>
#include <mia/3d/transformfactory.hh>

NS_MIA_BEGIN

/**
   \brief 3D linear registration class 
   
   Class for registration without regularization - i.e. should only be used
   for affine, rigid and translation only registrations
*/

class EXPORT_3D C3DRigidRegister {
public:
	/**
	   Constructor for the registration tool
	   @param cost cost function model
	   @param minimizer GSL provided minimizer
	   @param transform_creator string describing which transformation is supported
	   @param ipf interpolator
	   @param mg_levels number of multi-resolution levels to be used 
	 */

	C3DRigidRegister(P3DImageCost cost, PMinimizer minimizer,
			 P3DTransformationFactory transform_creator,
			 const C3DInterpolatorFactory& ipf,  size_t mg_levels);

	
	~C3DRigidRegister();

	/**
	   Run the registration of an image pair. 
	   @param src source (moving) image 
	   @param ref reference (fixed) image 
	   @returns the transformation registering src to ref that minimizes the constructor given 
	   cost function 
	 */
	P3DTransformation  run(P3DImage src, P3DImage ref) const;

private:
	struct C3DRigidRegisterImpl *impl;
};

NS_MIA_END

#endif
