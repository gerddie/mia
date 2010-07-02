/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
  This plug-in implements a direct time step update of a deformation field.
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
	C2DRegTimeStepPlugin::ProductPtr do_create()const;
	bool do_test() const;
	const string do_get_descr()const;
};


C2DDirectRegTimeStepPlugin::C2DDirectRegTimeStepPlugin():
	C2DRegTimeStepPlugin("direct")
{
}

C2DRegTimeStepPlugin::ProductPtr C2DDirectRegTimeStepPlugin::do_create()const
{
	return C2DRegTimeStepPlugin::ProductPtr(new C2DDirectRegTimeStep(get_min_timestep(), get_max_timestep()));
}

bool C2DDirectRegTimeStepPlugin::do_test() const
{
	return true;
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

