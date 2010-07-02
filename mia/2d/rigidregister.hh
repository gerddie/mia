/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#ifndef mia_2d_rigidregister_hh
#define mia_2d_rigidregister_hh


#include <mia/2d/cost.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN

enum EMinimizers {
	min_nmsimplex, 
	min_cg_fr, 
	min_cg_pr, 
	min_bfgs, 
	min_bfgs2, 
	min_gd, 
	min_undefined
}; 

/**
   Class for registration without regularization - i.e. should only be used 
   for affine, rigid and translation only registrations 
*/

class C2DRigidRegister {
public: 
	/**
	   Constructor for the registration tool
	   \param cost cost function model 
	   \param minimizer GSL provided minimizer 
	   \param transform_type string describing which transformation is supported
	   \param ipf interpolator
	 */

	C2DRigidRegister(P2DImageCost cost, EMinimizers minimizer, 
			 const string& transform_type, 
			 const C2DInterpolatorFactory& ipf);
	~C2DRigidRegister(); 

	P2DTransformation  run(P2DImage src, P2DImage ref,  size_t mg_levels) const; 

private: 
	struct C2DRigidRegisterImpl *impl; 
}; 

NS_MIA_END

#endif
