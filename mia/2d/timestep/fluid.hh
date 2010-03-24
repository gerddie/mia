
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

/*
  This plug-in implements a fluid time step update of a deformation field.
*/

#include <mia/2d/timestep.hh>

NS_BEGIN(fluid_timestep_2d)

class C2DFluidRegTimeStep: public mia::C2DRegTimeStep {
public: 
	C2DFluidRegTimeStep(float min, float max); 
private: 
	virtual float do_calculate_pertuberation(mia::C2DFVectorfield& io, const mia::C2DTransformation& shift) const;
	virtual bool do_regrid_requested (const mia::C2DTransformation& b, const mia::C2DFVectorfield& v, float delta) 
		const; 
	virtual bool do_has_regrid () const; 
}; 

NS_END
