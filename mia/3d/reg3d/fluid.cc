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

#include <limits>
#include <mia/3d/timestep.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(fluid_timestep)

class C3DFluidRegTimeStep: public	C3DRegTimeStep {
	friend class C3DFluidRegTimeStepPlugin;
public:
	C3DFluidRegTimeStep(float min, float max);
private:
	float do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const;
	bool do_regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const;
	virtual bool do_has_regrid () const;
	float  perturbation_at(size_t x, size_t y, size_t z, C3DFVectorfield::iterator iv, const C3DFVectorfield& u) const;
	float jacobian_at(C3DFVectorfield::const_iterator ib, C3DFVectorfield::const_iterator iv, float delta, int dx,int dxy) const;


};


C3DFluidRegTimeStep::C3DFluidRegTimeStep(float min, float max):
	C3DRegTimeStep(min, max)
{
}

inline float  C3DFluidRegTimeStep::perturbation_at(size_t x, size_t y, size_t z,
						   C3DFVectorfield::iterator iv, const C3DFVectorfield& u)const
{
	if (x < 1 || x > u.get_size().x - 2 ||
	    y < 1 || y > u.get_size().y - 2 ||
	    z < 1 || z > u.get_size().z - 2
		) {
		*iv = C3DFVector();
		return 0;
	} else {
		C3DFVector ux = ( u(x+1, y, z) - u(x-1, y, z)) * iv->x;
		C3DFVector uy = ( u(x, y+1, z) - u(x, y-1, z)) * iv->y;
		C3DFVector uz = ( u(x, y, z+1) - u(x, y, z-1)) * iv->z;
		C3DFVector ue = ( ux + uy + uz) * 0.5;
		*iv -= ue;
		return iv->norm2();
	}
}


float C3DFluidRegTimeStep::do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const
{
	// get the new
	C3DFVectorfield::iterator iv = io.begin();
	float max_gamma = 0.0f;
	int z;
#ifdef __OPENMP
#pragma omp parallel for  shared(max_gamma) private(z) schedule(dynamic)
#endif
	for (z = 0; z < (int)io.get_size().z; ++z) {
		float mg = 0.0f;
		for (size_t y = 0; y < io.get_size().y; ++y)
			for (size_t x = 0; x < io.get_size().x; ++x, ++iv){
				float gamma = perturbation_at(x, y, z, iv, shift);
				if (gamma > mg)
					mg = gamma;
			}
#ifdef __OPENMP
#pragma omp critical
#endif
		if (mg > max_gamma)
			max_gamma = mg;
	}
	return sqrt(max_gamma);
}

float C3DFluidRegTimeStep::jacobian_at(C3DFVectorfield::const_iterator ib, C3DFVectorfield::const_iterator iv, float delta, int dx, int dxy)const
{
	C3DFVector dux =  ib[1] - ib[-1];
	C3DFVector duy =  ib[dx] - ib[-dx];
	C3DFVector duz =  ib[dxy] - ib[-dxy];

	C3DFVector dvx =  iv[ 1] - iv[ -1];
	C3DFVector dvy =  iv[dx] - iv[-dx];
	C3DFVector dvz =  iv[dxy] - iv[-dxy];

	float a = 2 - dux.x - delta * dvx.x;
	float d =   - dux.y - delta * dvx.y;
	float g =   - dux.z - delta * dvx.z;

	float b =   - duy.x - delta * dvy.x;
	float e = 2 - duy.y - delta * dvy.y;
	float h =   - duy.z - delta * dvy.z;

	float c =   - duz.x - delta * dvz.x;
	float f =   - duz.y - delta * dvz.y;
	float k = 2 - duz.z - delta * dvz.z;

	return a*(e*k-f*h)-b*(d*k-f*g)+c*(d*h-e*g);
}

bool C3DFluidRegTimeStep::do_regrid_requested (const C3DFVectorfield& u, const C3DFVectorfield& v, float delta) const
{
	float min_j = numeric_limits<float>::max();
#ifdef __OPENMP
#pragma omp parallel for  shared(min_j) schedule(dynamic)
#endif
	for (size_t z = 1; z < u.get_size().z - 1; ++z) {

		float lmin_j = numeric_limits<float>::max();
		C3DFVectorfield::const_iterator ib = u.begin() + u.get_size().x + u.get_plane_size_xy() * z;
		C3DFVectorfield::const_iterator iv = v.begin() + u.get_size().x + u.get_plane_size_xy() * z;

		for (size_t y = 1; y < u.get_size().y - 1; ++y) {
			++ib;
			++iv;
			for (size_t x = 1; x < u.get_size().x - 1; ++x, ++ib, ++iv) {

				float j = jacobian_at(ib, iv, delta, u.get_size().x, u.get_plane_size_xy());
				if ( j < lmin_j) {
					lmin_j = j;
				}

			}
			++ib;
			++iv;
		}
#ifdef __OPENMP
#pragma omp critical
#endif
		if (min_j > lmin_j)
			min_j = lmin_j;
	}
	min_j *= 0.125;
	cvdebug() << "jac = " <<  min_j  << "\n";
	return min_j < 0.5;
}

bool C3DFluidRegTimeStep::do_has_regrid () const

{
	return true;
}

class C3DFluidRegTimeStepPlugin : public C3DRegTimeStepPlugin {
public:
	C3DFluidRegTimeStepPlugin();
private:
	C3DRegTimeStep *do_create()const;
	const string do_get_descr()const;
};


C3DFluidRegTimeStepPlugin::C3DFluidRegTimeStepPlugin():
	C3DRegTimeStepPlugin("fluid")
{
}

C3DRegTimeStep *C3DFluidRegTimeStepPlugin::do_create()const
{
	return new C3DFluidRegTimeStep(get_min_timestep(), get_max_timestep());
}

const string C3DFluidRegTimeStepPlugin::do_get_descr()const
{
	return "a fluidly applied time step operator";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DFluidRegTimeStepPlugin();
}


NS_END

