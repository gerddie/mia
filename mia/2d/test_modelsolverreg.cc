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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <mia/2d/modelsolverreg.hh>


NS_MIA_USE
using namespace boost;
using namespace std;

PrepareTestPluginPath plugin_path_init; 

class TestTimeStep : public C2DRegTimeStep {
private:

	virtual float do_calculate_pertuberation(C2DFVectorfield& io,
		         const C2DTransformation& shift) const;
	virtual bool do_regrid_requested (const C2DTransformation& b,
                         const C2DFVectorfield& v, float delta) const;
	virtual bool do_has_regrid () const;
};


float TestTimeStep::do_calculate_pertuberation(C2DFVectorfield& /*io*/,
					       const C2DTransformation& /*shift*/) const
{
	return 2.0;
}

bool TestTimeStep::do_regrid_requested (const C2DTransformation& /*b*/,
					const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	return false;
}

bool TestTimeStep::do_has_regrid () const
{
	return false;
}


BOOST_AUTO_TEST_CASE ( bla)
{
}


