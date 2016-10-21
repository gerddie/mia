/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
