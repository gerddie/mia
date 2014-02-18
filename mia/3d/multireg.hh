/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_3d_multireg_hh
#define mia_3d_multireg_hh

#include <mia/3d/model.hh>
#include <mia/3d/timestep.hh>
#include <mia/3d/deformer.hh>
#include <mia/3d/fatcost.hh>
#include <mia/3d/transform.hh>

NS_MIA_BEGIN

/**
   @ingroup basic 
   @brief pointer type for a 3D vector field 
*/
typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;

/**
   @ingroup registration 
   \brief A 3D non-rigid image registration class. 
   
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
	   \param max_iter maximum number of times steps to be used at each multi-grid level
	   \param model the registration model (e.g.- navier for linear elasticity operator)
	   \param time_step the time step model (e.g. fluid to letthe model operator work on the
                velocity field instead of the deformation field)
	   \param outer_epsilon a relative cost function value per multi-grid level to stop
                  registration
	*/
	C3DMultiImageRegister(size_t start_size, size_t max_iter,
			      C3DRegModel& model, C3DRegTimeStep& time_step,
			      float outer_epsilon);

	/**
	   The registration operator that does the registration
	   \param cost the cost function to be minimized 
	   \param ipf interpolation factory used for image transformations 
	   \returns a vector field describing the registration
	 */
	P3DTransformation operator () (C3DImageFatCostList& cost, P3DInterpolatorFactory  ipf);
private:
	void reg_level(C3DImageFatCostList& cost, C3DGridTransformation& result, const C3DInterpolatorFactory& ipf);
	void reg_level_regrid(C3DImageFatCostList& cost, C3DGridTransformation& result, const C3DInterpolatorFactory& ipf);

	size_t m_start_size;
	size_t m_max_iter;
	C3DRegModel& m_model;
	C3DRegTimeStep& m_time_step;
	float m_outer_epsilon;
};


NS_MIA_END

#endif
