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
	bool  do_test() const;
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

bool C3DDirectRegTimeStepPlugin::do_test() const
{
	C3DDirectRegTimeStep plugin(0.5f, 1.5f);

	C3DFVectorfield infield(C3DBounds(1,1,1));
	C3DFVectorfield outfield(C3DBounds(1,1,1));

	infield(0,0,0) = C3DFVector(3.0, 4.0, 2.0);
	outfield(0,0,0) =  C3DFVector(1.0, 2.0, 3.0);

	plugin.apply(infield, outfield, 0.2f);
	cvdebug() << outfield(0,0,0) << "\n";
	return (outfield(0,0,0) == C3DFVector(1.6f, 2.8f, 3.4f));
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

