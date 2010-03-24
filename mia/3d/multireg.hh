/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef mia_3d_multireg_hh
#define mia_3d_multireg_hh

#include <mia/3d/model.hh>
#include <mia/3d/timestep.hh>
#include <mia/3d/deformer.hh>
#include <mia/3d/fatcost.hh>
#include <mia/3d/transform.hh>
#include <mia/3d/gridtransformation.hh>

NS_MIA_BEGIN

typedef SHARED_PTR(C3DFVectorfield) P3DFVectorfield; 
/**
   The 3D non-rigid image registration class. Its usage is very simple: 
   Initialise it with the desired parameters and call it with the 
   source (template) and reference image to obtain a vector field 
   describing the registration. 
*/
class EXPORT_3D C3DMultiImageRegister {
public: 
	/**
	   The constructor: 
	   \param start_size an approximate size for the lowest resolution in the 
                   multi-resolution registration
	   \param cost an image similarity measure with an according gradient function
	   \param max_iter maximum number of times steps to be used at each multi-grid level
	   \param model the registration model (e.g.- navier for linear elasticity operator)
	   \param time_step the time step model (e.g. fluid to letthe model operator work on the 
                velocity field instead of the deformation field)
	   \param ipf an interpolator factory for image interpolation 
           \param outer_epsilon a relative cost function value per multi-grid level to stop
                  registration
	   \param save_steps save the deformed source image for each registration step
	 */
	C3DMultiImageRegister(size_t start_size, size_t max_iter, 
			      C3DRegModel& model, C3DRegTimeStep& time_step, 
			      float outer_epsilon); 

	/**
	   The registration operator that does the registration
	   \param source the source or template image to be registered to the ...
	   \param reference 
	   \returns a vector field describing the registration
	 */
	P3DTransformation operator () (C3DImageFatCostList& cost, P3DInterpolatorFactory  ipf);
private: 
	void reg_level(C3DImageFatCostList& cost, C3DGridTransformation& result, const C3DInterpolatorFactory& ipf); 
	void reg_level_regrid(C3DImageFatCostList& cost, C3DGridTransformation& result, const C3DInterpolatorFactory& ipf); 
	
	size_t _M_start_size;
	size_t _M_max_iter; 
	C3DRegModel& _M_model; 
	C3DRegTimeStep& _M_time_step; 
	float _M_outer_epsilon; 
}; 


NS_MIA_END

#endif
