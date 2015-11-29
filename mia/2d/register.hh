/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef mia_2d_register_hh
#define mia_2d_register_hh

#include <mia/2d/model.hh>
#include <mia/2d/timestep.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_BEGIN


/**
   @ingroup registration 
   \brief A 2D non-rigid registration class for time-marching PDE based registration  

   The 2D non-rigid image registration class. This class allows the combination of 
   various image cost functions, provides multi-level registration and the use 
   of various PDE models (depending on the provided plug-ins.
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
	*/
	C2DMultiImageRegister(size_t start_size, size_t max_iter, 
			      C2DRegModel& model, 
			      C2DRegTimeStep& time_step, 
			      C2DTransformCreator& trans_factory, 
			      float outer_epsilon); 

	/**
	   The registration operator that does the registration
	   \param cost cost function combination to be optimized 
	   \param ipf interpolator to be used for image transformations 
	   \returns a vector field describing the registration
	 */
	P2DTransformation operator () (C2DImageFatCostList& cost, P2DInterpolatorFactory  ipf);
private: 
	void reg_level(C2DImageFatCostList& cost, C2DGridTransformation& result, const C2DInterpolatorFactory& ipf); 
	void reg_level_regrid(C2DImageFatCostList& cost, C2DGridTransformation& result, const C2DInterpolatorFactory& ipf); 
	
	size_t m_start_size;
	size_t m_max_iter; 
	C2DRegModel& m_model; 
	C2DRegTimeStep& m_time_step; 
	float m_outer_epsilon; 
}; 


NS_MIA_END

#endif

