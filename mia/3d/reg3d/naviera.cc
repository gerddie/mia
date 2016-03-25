/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

/*
  This plug-in implements the navier-stokes operator like registration model
  that accounts for linear elastic and fluid dynamic registration.
  Which model is used depends on the selected time step.

  To solve the PDE a full SOR is implemented.
*/

#include <iomanip>
#include <set>
#include <limits>

#ifdef __POWERPC__
#include <ppc_intrinsics.h>
#endif

#include <mia/3d/model.hh>

#ifndef WIN32
#define PRIVATE __attribute__((visibility("hidden")))
#else
#define PRIVATE
#endif

NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(naviera_regmodel)

class C3DNavierRegModel: public C3DRegModel {
public:
	C3DNavierRegModel(float mu, float lambda, size_t maxiter, float epsilon);

	bool  test_kernel();
private:
	virtual void do_solve(const C3DFVectorfield& b, C3DFVectorfield& x) const;
	virtual float do_force_scale() const;

	PRIVATE void set_flags(C3DUBImage::iterator loc)const;

	C3DFVector get_p(const C3DFVector& b, const C3DFVectorfield::iterator& v) const;
	C3DFVector get_q(const C3DFVectorfield::iterator& v)const;
	PRIVATE float  solve_at(const C3DFVector& b, C3DFVectorfield::iterator& x)const;
	float m_mu;
	float m_lambda;
	float m_a, m_c, m_a_b, m_b_4;
//	float m_omega;
	float m_epsilon;
	size_t m_max_iter;
	mutable int m_dx;
	mutable int m_dxy;

};

C3DNavierRegModel::C3DNavierRegModel(float mu, float lambda, size_t maxiter,  float epsilon):
	m_mu(mu),
	m_lambda(lambda),
	m_epsilon(epsilon),
	m_max_iter(maxiter)
{
	float a = mu;
	float b = lambda + mu;
	m_c = 1 / (6*a+2*b);
	m_b_4 = 0.25 * b * m_c;
	m_a_b = ( a + b ) * m_c;
	m_a = a * m_c;
	m_dx = 0; 
	m_dxy = 0; 

	cvdebug() << "initialise model with mu=" << mu << " lambda=" << lambda << "\n";
}


float C3DNavierRegModel::do_force_scale() const
{
	cvdebug() << "Force scale = " <<m_c << "\n";
	return m_c;
}

inline PRIVATE  void C3DNavierRegModel::set_flags(C3DUBImage::iterator loc)const
{
	const int dx = m_dx;
	const int mdx = - m_dx;

	C3DUBImage::iterator lmz = loc - m_dxy;
	C3DUBImage::iterator lpz = loc + m_dxy;
	C3DUBImage::iterator lmy = loc - dx;
	C3DUBImage::iterator lpy = loc + dx;

	lmz[     mdx] = 1;
	lmz[ -    1 ] = 1;
	lmz[      0 ] = 1;
	lmz[ +    1 ] = 1;
	lmz[      dx] = 1;

	lmy[ - 1 ] = 1;
	lmy[   0 ] = 1;
	lmy[   1 ] = 1;

	loc[ - 1 ] = 1;
	loc[   0 ] = 1;
	loc[ + 1 ] = 1;

	lpy[ - 1 ] = 1;
	lpy[   0 ] = 1;
	lpy[ + 1 ] = 1;

	lpz[   mdx ] = 1;
	lpz[ -   1 ] = 1;
	lpz[     0 ] = 1;
	lpz[     1 ] = 1;
	lpz[    dx ] = 1;
}


void C3DNavierRegModel::do_solve(const C3DFVectorfield& b, C3DFVectorfield& v) const
{
	// init velocity fields
	float start_residuum = 0.0;
	float residuum;
	float thresh = 0.0;

	size_t i = 0;
	assert(b.get_size() == v.get_size());

	m_dx = b.get_size().x;
	m_dxy = b.get_plane_size_xy();

	C3DFImage residua(b.get_size());
	fill(residua.begin(), residua.end(), 0.0f);

	C3DUBImage update_flags1(b.get_size());
	C3DUBImage update_flags2(b.get_size());

	C3DUBImage& update_flags = update_flags1;
	C3DUBImage& dset_flags = update_flags2;

	fill(update_flags.begin(), update_flags.end(), 1);

	do {
		++i;
		residuum = 0;



		for (size_t z = 1; z < b.get_size().z-1; z++) {

			size_t step = m_dx + m_dxy * z;

			C3DFVectorfield::const_iterator ib = b.begin() + step;
			C3DFVectorfield::iterator iv = v.begin() + step;
			C3DFImage::iterator ir = residua.begin() + step;
			C3DUBImage::iterator iu = update_flags.begin() + step;
			C3DUBImage::iterator is = dset_flags.begin() + step;


			for (size_t y = 1; y < b.get_size().y-1; y++) {
				++ib;
				++iv;
				++ir;
				++iu;
				++is;
				for (size_t x = 1; x < b.get_size().x-1; x++, ++ib, ++iv, ++ir, ++iu, ++is){
					if (*iu) {
						*iu = 0;
						*ir = solve_at(*ib, iv);
					}
					if (*ir > thresh)
						set_flags(is);

					residuum += *ir;
				}
				++ib;
				++iv;
				++ir;
				++iu;
				++is;

			}
			if (i ==1)
				start_residuum = residuum;

		}

		swap(update_flags, dset_flags);


		thresh = (residuum * residuum) / (start_residuum * residua.size() * (i+1));

		cvdebug() << "SOLVE " << setw(3) << i <<":" << residuum << " : " << thresh<< " : "<< residua.size()<<"\n";

		if (residuum < 1)
			break;

	} while (i < m_max_iter && residuum / start_residuum > m_epsilon);

}

inline C3DFVector C3DNavierRegModel::get_p(const C3DFVector& b, const C3DFVectorfield::iterator& v)const
{
	const C3DFVector vm = v[-1];
	const C3DFVector vp = v[1];

	const C3DFVector vym = v[-m_dx];
	const C3DFVector vyp = v[ m_dx];
	const C3DFVector vzm = v[-m_dxy];
	const C3DFVector vzp = v[ m_dxy];


	float rx = (vm.x + vp.x) * m_a_b;
	float ry = (vm.y + vp.y) * m_a_b;
	float rz = (vm.z + vp.z) * m_a_b;

	rx += b.x;
	ry += b.y;
	rz += b.z;

	float hx = (vym.x + vyp.x + vzp.x + vzm.x) * m_a;
	float hy = (vym.y + vyp.y + vzp.y + vzm.y) * m_a;
	float hz = (vym.z + vyp.z + vzp.z + vzm.z) * m_a;

	return  C3DFVector(rx + hx, ry + hy, rz + hz);
}

inline C3DFVector C3DNavierRegModel::get_q(const C3DFVectorfield::iterator& v)const
{

	const C3DFVector& vmm0 = v[ -1 - m_dx];
	const C3DFVector& vpm0 = v[  1 - m_dx];
	const C3DFVector& vmp0 = v[ -1 + m_dx];
	const C3DFVector& vpp0 = v[  1 + m_dx];

	const  C3DFVectorfield::iterator vm = v - m_dxy;
	const  C3DFVectorfield::iterator vp = v + m_dxy;

	const C3DFVector& vm0m = vm[ -1];
	const C3DFVector& vp0m = vm[  1];
	const C3DFVector& vm0p = vp[ -1];
	const C3DFVector& vp0p = vp[  1];

	const C3DFVector& v0mm = vm[ -m_dx];
	const C3DFVector& v0pm = vm[  m_dx];
	const C3DFVector& v0mp = vp[ -m_dx];
	const C3DFVector& v0pp = vp[  m_dx];


	return C3DFVector( (vmm0.y + vpp0.y - vmp0.y - vpm0.y + vm0m.z + vp0p.z - vm0p.z - vp0m.z) * m_b_4,
			   (vmm0.x + vpp0.x - vmp0.x - vpm0.x + v0mm.z + v0pp.z - v0mp.z - v0pm.z) * m_b_4,
			   (vm0m.x + vp0p.x - vm0p.x - vp0m.x + v0mm.y + v0pp.y - v0mp.y - v0pm.y) * m_b_4);
}

inline float  C3DNavierRegModel::solve_at(const C3DFVector& b, C3DFVectorfield::iterator& v)const
{
	C3DFVector& vv = *v;
	const C3DFVector old_v = vv;

	const  C3DFVectorfield::iterator vm = v - m_dxy;
	const  C3DFVectorfield::iterator vp = v + m_dxy;

	const int dx = m_dx;
	const int mx = - m_dx;

	const C3DFVector& v0mm = vm[ mx];
	const C3DFVector& vm0m = vm[ -1];
	const C3DFVector& vzm =  vm[0];
	const C3DFVector& vp0m = vm[  1];
	const C3DFVector& v0pm = vm[  dx];


	const C3DFVector& vmm0 = v[ -1 - dx];
	const C3DFVector& vpm0 = v[  1 - dx];

	const C3DFVector& vym = v[mx];
	const C3DFVector& vm00 = v[-1];
	const C3DFVector& vp00 = v[1];
	const C3DFVector& vyp = v[ dx];

	const C3DFVector& vmp0 = v[ -1 + dx];
	const C3DFVector& vpp0 = v[  1 + dx];

	const C3DFVector& vm0p = vp[ -1];
	const C3DFVector& v0mp = vp[ mx];
	const C3DFVector& vzp = *vp;
	const C3DFVector& v0pp = vp[  dx];
	const C3DFVector& vp0p = vp[  1];

	float rx = (vm00.x + vp00.x) * m_a_b + b.x;
	const float hx = vym.x + vyp.x + vzp.x + vzm.x;

	float ry = (vm00.y + vp00.y) * m_a_b + b.y;
	const float hy = vym.y + vyp.y + vzp.y + vzm.y;

	float rz = (vm00.z + vp00.z) * m_a_b + b.z;
	const float hz = vym.z + vyp.z + vzp.z + vzm.z;

	{
		float ax = vmm0.y + vpp0.y;
		float ay = vmm0.x + vpp0.x;
		float az = vm0m.x + vp0p.x;

		ax -= vmp0.y;
		ay -= vmp0.x;
		az -= vm0p.x;

		ax -= vpm0.y;
		ay -= vpm0.x;
		az -= vp0m.x;

		ax += vm0m.z;
		ay += v0mm.z;
		az += v0mm.y;

		ax += vp0p.z;
		ay += v0pp.z;
		az += v0pp.y;

		ax -= vm0p.z;
		ay -= v0mp.z;
		az -= v0mp.y;

		ax -= vp0m.z;
		ay -= v0pm.z;
		az -= v0pm.y;

		rx += m_b_4 * ax;
		ry += m_b_4 * ay;
		rz += m_b_4 * az;

		rx += hx * m_a;
		ry += hy * m_a;
		rz += hz * m_a;

	}

	vv.x = rx;
	rx -= old_v.x;

	vv.y = ry;
	ry -= old_v.y;

	vv.z = rz;
	rz -= old_v.z;

	rx *= rx;
	ry *= ry;
	rz *= rz;
	rx += ry + rz;

	//C3DFVector q = get_q(v);

	//const C3DFVector hmm((( p + q ) - *v) * m_omega);



#ifdef __POWERPC__
	return rx > 0 ?__fres( __frsqrte(rx) ) : 0.0f;
#else
	return rx > 0 ? sqrtf(rx) : 0.0f;
#endif
}


class C3DNavierRegModelPlugin: public C3DRegModelPlugin {
public:
	C3DNavierRegModelPlugin();
	C3DRegModel *do_create()const;

private:
	const string do_get_descr()const;

	float m_mu;
	float m_lambda;
	float m_epsilon;
	int m_maxiter;
};

C3DNavierRegModelPlugin::C3DNavierRegModelPlugin():
	C3DRegModelPlugin("naviera"),
	m_mu(1.0),
	m_lambda(1.0),
	m_epsilon(0.0001),
	m_maxiter(40)
{
	add_parameter("mu", make_nonnegative_param(m_mu, false, "isotropic compliance"));
	add_parameter("lambda", make_nonnegative_param(m_lambda, false, "isotropic compression"));

	add_parameter("epsilon", make_oci_param(m_epsilon, 0.0, 0.1, false, "stopping parameter"));
	add_parameter("iter", make_lc_param(m_maxiter, 1, false, "maximum number of iterations"));
}

C3DRegModel *C3DNavierRegModelPlugin::do_create()const
{
	return new C3DNavierRegModel(m_mu, m_lambda, m_maxiter, m_epsilon);
}

const string C3DNavierRegModelPlugin::do_get_descr()const
{
	return "navier-stokes based registration model";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DNavierRegModelPlugin();
}

NS_END

