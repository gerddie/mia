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

#include <limits>
#include <mia/3d/timestep.hh>
NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(direct_timestep)

class C3DDirectRegTimeStep: public	C3DRegTimeStep {
public:
	C3DDirectRegTimeStep(float min, float max);
private:
	virtual float do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const;
	virtual bool do_regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const;
	virtual bool do_has_regrid () const;
};

C3DDirectRegTimeStep::C3DDirectRegTimeStep(float min, float max):
	C3DRegTimeStep(min, max)
{
}

bool C3DDirectRegTimeStep::do_regrid_requested (const C3DFVectorfield& /*b*/,
						const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	return false;
}

bool C3DDirectRegTimeStep::do_has_regrid () const
{
	return false;
}

float C3DDirectRegTimeStep::do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& /*shift*/) const
{
	C3DFVectorfield::const_iterator hi = io.begin();
	C3DFVectorfield::const_iterator he = io.end();

	float max_norm2 = 0.0;
	while (hi != he){
		float norm2 = hi->norm2();
		if (max_norm2 < norm2)
			max_norm2 = norm2;
		++hi;
	}
	max_norm2 = sqrt(max_norm2);
	cvdebug() << "max_norm = " << max_norm2 << "\n";

	return max_norm2;
}

class C3DDirectRegTimeStepPlugin : public C3DRegTimeStepPlugin {
public:
	C3DDirectRegTimeStepPlugin();
private:
	C3DRegTimeStep *do_create()const;
	const string do_get_descr()const;
};


C3DDirectRegTimeStepPlugin::C3DDirectRegTimeStepPlugin():
	C3DRegTimeStepPlugin("direct")
{
}

C3DRegTimeStep *C3DDirectRegTimeStepPlugin::do_create()const
{
	cverr() << get_name() << ": This time step code is currently not working properly\n";
	return new C3DDirectRegTimeStep(get_min_timestep(), get_max_timestep());
}

const string C3DDirectRegTimeStepPlugin::do_get_descr()const
{
	return "a directly applied time step operator";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDirectRegTimeStepPlugin();
}


NS_END

