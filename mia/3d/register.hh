/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef reg3d_register_hh
#define reg3d_register_hh

#include <mia/3d/deformer.hh>
#include <mia/3d/model.hh>
#include <mia/3d/timestep.hh>
#include <mia/3d/cost.hh>


NS_MIA_BEGIN

typedef std::shared_ptr< C3DFVectorfield > P3DFVectorfield;

/**
   \ingroup registration
   \brief A 3D non-rigid image registration class. 

   A 3D non-rigid image registration class. 
   Initialise it with the desired parameters and call it with the
   source (template) and reference image to obtain a vector field
   describing the registration.
*/

typedef std::shared_ptr<C3DImageCost > P3DImageCost;

class EXPORT_3D C3DImageRegister {
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
	C3DImageRegister(size_t start_size, C3DImageCost& cost, size_t max_iter,
			 C3DRegModel& model, C3DRegTimeStep& time_step,
			 const C3DInterpolatorFactory&  ipf, float outer_epsilon, bool save_steps);

	/**
	   The registration operator that does the registration
	   \param source the source or template image to be registered to the ...
	   \param reference
	   \returns a vector field describing the registration
	 */
	P3DFVectorfield operator () (const C3DImage& source, const C3DImage& reference);
private:
	void reg_level(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result);
	void reg_level_regrid(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result);
	void reg_level_regrid_opt(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result);
	size_t m_start_size;
	C3DImageCost& m_cost;
	size_t m_max_iter;
	C3DRegModel& m_model;
	C3DRegTimeStep& m_time_step;
	const C3DInterpolatorFactory& m_ipf;
	float m_outer_epsilon;
	bool m_save_steps;
	size_t m_mnum;
};

typedef std::shared_ptr<C3DRegModel > P3DRegModel;

NS_MIA_END

#endif
