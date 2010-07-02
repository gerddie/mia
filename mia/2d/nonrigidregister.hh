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

#ifndef mia_2d_nonrigidregister_hh
#define mia_2d_nonrigidregister_hh

#include <mia/2d/model.hh>
#include <mia/2d/timestep.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_BEGIN


/**
   The 2D non-rigid image registration class. Its usage is very simple:
   Initialise it with the desired parameters and call it with the
   source (template) and reference image to obtain a vector field
   describing the registration.
*/
class EXPORT_2D C2DMultiImageNonrigidRegister {
public:
	/**
	   The constructor:
	   \param start_size an approximate size for the lowest
	        resolution in the multi-resolution registration
	   \param max_iter maximum number of times steps to be
                used at each multi-grid level
	   \param model the registration model (e.g.- navier for
                linear elasticity operator)
	   \param time_step the time step model (e.g. fluid to let
                the model operator work on the velocity field instead
                of the deformation field)
	   \param trans_factory factory for creation of transformations
           \param outer_epsilon a relative cost function value per
                multi-grid level to stop registration
	   \param save_steps save the deformed source image for each
                registration step
	*/
	C2DMultiImageNonrigidRegister(size_t start_size, size_t max_iter,
				      P2DRegModel model,
				      P2DRegTimeStep time_step,
				      P2DTransformationFactory trans_factory,
				      float outer_epsilon);


	~C2DMultiImageNonrigidRegister();
	/**
	   The registration operator that does the registration
	   \param source the source or template image to be registered to the ...
	   \param reference
	   \returns a vector field describing the registration
	 */
	P2DTransformation operator () (C2DImageFatCostList& cost);
private:
	struct C2DMultiImageNonrigidRegisterImpl *impl;
};


NS_MIA_END

#endif

