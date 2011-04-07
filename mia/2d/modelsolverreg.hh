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

#ifndef mia_2d_model_solve_register_hh
#define mia_2d_model_solve_register_hh

#include <mia/2d/model.hh>
#include <mia/2d/timestep.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_BEGIN


class CModelSolverRegistration  {

	CModelSolverRegistration(P2DRegModel model,
				 P2DRegTimeStep time_step,
				 P2DTransformationFactory tf,
				 size_t start_size,
				 size_t max_iter,
				 float outer_epsilon);

	P2DTransformation operator ()(C2DImageFatCostList& cost) const;

 private:

	auto_ptr<struct CModelSolverRegistrationImpl> m_impl;
};

NS_MIA_END
#endif
