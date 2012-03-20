/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <limits>
#include <mia/2d/timestep/fluid.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(fluid_timestep_2d)

#ifdef WIN32
#undef min
#undef max
#endif


C2DFluidRegTimeStep::C2DFluidRegTimeStep(float min, float max):
	C2DRegTimeStep(min, max)
{
}

float C2DFluidRegTimeStep::do_calculate_pertuberation(C2DFVectorfield& io, const C2DTransformation& shift) const
{
	return shift.pertuberate(io);
}

bool C2DFluidRegTimeStep::do_regrid_requested (const C2DTransformation& u, const C2DFVectorfield& v, float delta) const
{
	return u.get_jacobian(v, delta) < 0.5;
}

bool C2DFluidRegTimeStep::do_has_regrid () const
{
	return true;
}

class C2DFluidRegTimeStepPlugin : public C2DRegTimeStepPlugin {
public:
	C2DFluidRegTimeStepPlugin();
private:
	C2DRegTimeStep *do_create()const;
	bool do_test() const;
	const string do_get_descr()const;
};


C2DFluidRegTimeStepPlugin::C2DFluidRegTimeStepPlugin():
	C2DRegTimeStepPlugin("fluid")
{
}

C2DRegTimeStep *C2DFluidRegTimeStepPlugin::do_create()const
{
	return new C2DFluidRegTimeStep(get_min_timestep(), get_max_timestep());
}

bool C2DFluidRegTimeStepPlugin::do_test() const
{
	return true;
}

const string C2DFluidRegTimeStepPlugin::do_get_descr()const
{
	return "a fluidly applied time step operator";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DFluidRegTimeStepPlugin();
}


NS_END

