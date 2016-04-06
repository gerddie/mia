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

#include <mia/2d/timestep/direct.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(direct_timestep_2d)

C2DDirectRegTimeStep::C2DDirectRegTimeStep(float min, float max):
	C2DRegTimeStep(min, max)
{
}

bool C2DDirectRegTimeStep::do_regrid_requested (const C2DTransformation& /*b*/,
						const C2DFVectorfield& /*v*/, float /*delta*/) const
{
	return false;
}

bool C2DDirectRegTimeStep::do_has_regrid () const
{
	return false;
}

float C2DDirectRegTimeStep::do_calculate_pertuberation(C2DFVectorfield& io, const C2DTransformation& /*shift*/) const
{
	// this should be done based of the transformation type
	C2DFVectorfield::const_iterator i = io.begin();
	C2DFVectorfield::const_iterator e = io.end();
	float max_norm = 0.0;
	while (i != e)  {
		float n = i->norm2();
		if (n > max_norm)
			max_norm = n;
		++i;
	}
	return sqrt(max_norm);
}

class C2DDirectRegTimeStepPlugin : public C2DRegTimeStepPlugin {
public:
	C2DDirectRegTimeStepPlugin();
private:
	C2DRegTimeStep *do_create()const;
	const string do_get_descr()const;
};


C2DDirectRegTimeStepPlugin::C2DDirectRegTimeStepPlugin():
	C2DRegTimeStepPlugin("direct")
{
}

C2DRegTimeStep *C2DDirectRegTimeStepPlugin::do_create()const
{
	return new C2DDirectRegTimeStep(get_min_timestep(), get_max_timestep());
}

const string C2DDirectRegTimeStepPlugin::do_get_descr()const
{
	return "a directly applied time step operator";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDirectRegTimeStepPlugin();
}

NS_END

