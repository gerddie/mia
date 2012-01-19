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

NS_BEGIN(fluid_timestep)

class C3DFluidRegTimeStep: public	C3DRegTimeStep {
	friend class C3DFluidRegTimeStepPlugin;
public:
	C3DFluidRegTimeStep(float min, float max);

	void test() const;
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

void C3DFluidRegTimeStep::test() const
{

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
	bool do_test() const;
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

bool C3DFluidRegTimeStepPlugin::do_test() const
{
	C3DFluidRegTimeStep plugin(0.5f, 1.5f);

	C3DFVectorfield infield(C3DBounds(3,3,3));
	C3DFVectorfield outfield(C3DBounds(3,3,3));

	fill(infield.begin(), infield.end(), C3DFVector(0,0,0));
	fill(outfield.begin(), outfield.end(), C3DFVector(0,0,0));

	infield(0,1,1) = C3DFVector(4,2,6);
	infield(2,1,1) = C3DFVector(4,8,2);  //  0, 6, -4
	                                     //  0, 3, -2, (* delta)
	outfield(0,1,1) = C3DFVector(3,2,1);
	outfield(2,1,1) = C3DFVector(1,1,2); // -2, -1, 1
                                             // -2,  2,-1 (out + delta * in)

	infield(1,0,1) = C3DFVector(2,6,4);
	infield(1,2,1) = C3DFVector(2,8,4);  //  0, 2, 0
                                             //  0, 1, 0  (* delta)
	outfield(1,0,1) = C3DFVector(7,1,9);
	outfield(1,2,1) = C3DFVector(3,6,5); // -4, 5, -4
                                             // -4, 6, -4 (out + delta * in)
	infield(1,1,0) = C3DFVector(6,4,0);
	infield(1,1,2) = C3DFVector(2,6,4);  // -4, 2, 4
                                             // -2, 1, 2  (* delta)
	outfield(1,1,0) = C3DFVector(7,1,2);
	outfield(1,1,2) = C3DFVector(3,6,5); // -4, 5, 3
                                             // -6, 6, 5 (out + delta * in)

        // I-J:  4  4  6
   	//      -2 -4 -6  ->det = - 24   +96   ) = 72
        //       1  4 -3
	infield(1,1,1)  = C3DFVector(2,-6,-4);
	// ux =  -4,  -2,  2
        // uy =  24, -30, 24
	// uz =  16, -20,-12
        // ue =  18, -26,  7
	// vin = -16, 20, -11
	// vin.norm2 = 777
	// -8, 10, -5.5

	outfield(1,1,1) = C3DFVector(3,5,4);

	// u =

	size_t center_offset = infield.get_plane_size_xy() + infield.get_size().x + 1;


	const float j_at_test = 72.0f;
	const float j_at = plugin.jacobian_at(outfield.begin() + center_offset, infield.begin() + center_offset, 0.5,
					      infield.get_size().x, infield.get_plane_size_xy());

	bool success = true;
	if (j_at_test != j_at) {
		cvfail() << "jacobian_at = " << j_at << ", expect: " << j_at_test << "\n";
		success = false;
	}

	const float pat_at = 777.0f;
	const float pat = plugin.perturbation_at(1, 1, 1, infield.begin() + center_offset, outfield);

	if (pat != pat_at) {
		cvfail() << "pertubaration_at = " << pat << ", expect: " << pat_at << "\n";
		success = false;
	}

	plugin.apply(infield, outfield, 0.5);

	return success && (outfield(1,1,1) == C3DFVector(-5.0f, 15.0f, -1.5f));
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

