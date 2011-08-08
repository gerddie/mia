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

#ifndef mia_2d_rigidregister_hh
#define mia_2d_rigidregister_hh

#include <mia/core/minimizer.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/transform.hh>
#include <mia/2d/transformfactory.hh>

//#include <mia/internal/rigidregister.hh>


NS_MIA_BEGIN

/**
   @brief Linear Registration of 2D images.  
   
   Class for registration without regularization - i.e. should only be used
   for affine, rigid and translation only registrations. 
   However, one might note that the generic implementation of the C2DTransformtion 
   class makes is possible to use any kind of transformation that is provided by 
   a transformation plug-in. 
   This class allows that exactly one cost function is used. 
*/

class EXPORT_2D C2DRigidRegister {
public:
	/**
	   Constructor for the registration tool
	   @param  cost cost function model
	   @param  minimizer GSL provided minimizer
	   @param  transform_creator object used to create the transformation type 
	   @param mg_levels number of used multi-resolution levels 
	 */

	C2DRigidRegister(P2DImageCost cost, PMinimizer minimizer,
			 P2DTransformationFactory transform_creator, size_t mg_levels);

	
	~C2DRigidRegister();

	/**
	   Run the registration of an image pair. 
	   @param  src source (moving) image 
	   @param  ref reference (fixed) image 
	   @returns the transformation registering src to ref that minimizes the constructor given 
	   cost function 
	 */
	P2DTransformation  run(P2DImage src, P2DImage ref) const;

private:
	struct C2DRigidRegisterImpl *impl;
};

NS_MIA_END

#endif
