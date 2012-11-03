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

#include <errno.h>
#include "sor_solver.hh"



#include <iostream>
using namespace std;
using namespace mia;

//#define USE_OLD
//#define USE_VERY_OLD

TSORSolver::TSORSolver(int _max_steps, float _rel_res, float _abs_res,
		       float mu, float lambda):
	TFluidHomogenSolver(_max_steps,_rel_res, _abs_res,mu,lambda)
{
}

float TSORSolver::solve_at_very_old(const C3DFVectorfield& B,C3DFVectorfield *V,size_t  x, size_t  y, size_t  z)
{
	C3DFVector vdxx = (*V)(x-1,y,z) + (*V)(x+1,y,z); // 3A
	C3DFVector vdyy = (*V)(x,y-1,z) + (*V)(x,y+1,z); // 3A
	C3DFVector vdzz = (*V)(x,y,z-1) + (*V)(x,y,z+1); // 3A

	const C3DFVector& Vm1m1p0 = (*V)(x-1,y-1,z);
	const C3DFVector& Vp1p1p0 = (*V)(x+1,y+1,z);
	const C3DFVector& Vp1m1p0 = (*V)(x+1,y-1,z);
	const C3DFVector& Vm1p1p0 = (*V)(x-1,y+1,z);
	float  vydxy = Vm1m1p0.y + Vp1p1p0.y - Vp1m1p0.y - Vm1p1p0.y;  // 3A
	float  vxdxy = Vm1m1p0.x + Vp1p1p0.x - Vp1m1p0.x - Vm1p1p0.x;  // 3A

	const C3DFVector& Vm1p0m1 = (*V)(x-1,y,z-1);
	const C3DFVector& Vp1p0p1 = (*V)(x+1,y,z+1);
	const C3DFVector& Vp1p0m1 = (*V)(x+1,y,z-1);
	const C3DFVector& Vm1p0p1 = (*V)(x-1,y,z+1);
	float  vzdxz = Vm1p0m1.z + Vp1p0p1.z - Vp1p0m1.z - Vm1p0p1.z; // 3A
	float  vxdxz = Vm1p0m1.x + Vp1p0p1.x - Vp1p0m1.x - Vm1p0p1.x; // 3A

	const C3DFVector& Vp0m1m1 = (*V)(x,y-1,z-1);
	const C3DFVector& Vp0p1p1 = (*V)(x,y+1,z+1);
	const C3DFVector& Vp0p1m1 = (*V)(x,y+1,z-1);
	const C3DFVector& Vp0m1p1 = (*V)(x,y-1,z+1);
	float  vzdyz = Vp0m1m1.z + Vp0p1p1.z  - Vp0p1m1.z - Vp0m1p1.z; // 3A
	float  vydyz = Vp0m1m1.y + Vp0p1p1.y  - Vp0p1m1.y - Vp0m1p1.y; // 3A

	C3DFVector p((a_b)*vdxx.x + a*(vdyy.x+vdzz.x),        // 6A 6M
		     (a_b)*vdyy.y + a*(vdxx.y+vdzz.y),
		     (a_b)*vdzz.z + a*(vdxx.z+vdyy.z));

	C3DFVector q(vydxy+vzdxz,vxdxy+vzdyz,vxdxz+vydyz);   // 3A

	C3DFVector R = (B(x,y,z)) + p + b_4 * q;            // 6A 3M
	C3DFVector S = (c * R);      // 3A 6M
	C3DFVector delta = S -  (*V)(x,y,z);
	(*V)(x,y,z) = S;                             // 3A
	return delta.norm2();	                             // 2A 3M
	// SUM: 50A 18M
}



float TSORSolver::solve_at(C3DFVector *Data,const C3DFVector& bv){
	register int sizex = size.x;
	C3DFVector *Data_alt = &Data[ -d_xy ];
	C3DFVector q;
	C3DFVector p;

	q.z = Data_alt[-sizex].y; // D( 0,-1,-1).y
	q.y = Data_alt[-sizex].z; // D( 0,-1,-1).z

	q.z += Data_alt[-1].x;    // D(-1, 0,-1).x
	q.x = Data_alt[-1].z;     // D(-1, 0,-1).z

	q.z -= Data_alt[ 1].x;    // D( 1, 0,-1).x
	q.x -= Data_alt[ 1].z;    // D( 1, 0,-1).z

	q.z -= Data_alt[sizex].y; // D( 0, 1,-1).y
	q.y -= Data_alt[sizex].z; // D( 0, 1,-1).z

	Data_alt = &Data[ d_xy ];
	q.z -= Data_alt[-sizex].y; // D( 0,-1, 1).y
	q.y -= Data_alt[-sizex].z; // D( 0,-1, 1).z
	q.z -= Data_alt[-1].x;     // D(-1, 0,-1).x
	q.x -= Data_alt[-1].z;     // D(-1, 0,-1).z

	q.z += Data_alt[ 1].x;     // D( 1, 0,-1).x
	q.x += Data_alt[ 1].z;     // D( 1, 0,-1).z

	q.z += Data_alt[sizex].y;  // D( 0, 1, 1).y
	q.y += Data_alt[sizex].z;  // D( 0, 1, 1).z

	Data_alt = Data - size.x ;

	q.y += Data_alt[-1].x;     // D(-1,-1, 0).x
	q.x += Data_alt[-1].y;     // D(-1,-1, 0).y

	q.y -= Data_alt[ 1].x;     // D( 1,-1, 0).x
	q.x -= Data_alt[ 1].y;     // D( 1,-1, 0).y

	Data_alt = Data + size.x;

	q.y -= Data_alt[-1].x;     // D(-1, 1, 0).x
	q.x -= Data_alt[-1].y;     // D(-1, 1, 0).y

	q.y += Data_alt[ 1].x;     // D( 1, 1, 0).x
	q.x += Data_alt[ 1].y;     // D( 1, 1, 0).y


	q.x *= b_4;
	p.x  = Data[-1].x;         // D(-1, 0, 0).x

	q.y *= b_4;
	p.y  = Data[-sizex].y;     // D( 0,-1, 0).y

	q.z *= b_4;
	p.z  = Data[-d_xy].z;      // D( 0, 0,-1).z


	p.x += Data[ 1].x;         // D( 1, 0, 0).x
	p.y += Data[sizex].y;      // D( 0, 1, 0).y
	p.z += Data[d_xy].z;       // D( 0, 0, 1).z

	p *= a_b;

	q += p;

	p.x  = Data[-d_xy].x;     // D( 0, 0,-1).x
	p.y  = Data[-d_xy].y;     // D( 0, 0,-1).y

	p.x += Data[-sizex].x;    // D( 0,-1, 0).x
	p.z  = Data[-sizex].z;    // D( 0,-1, 0).z

	p.y += Data[-1].y;        // D(-1, 0, 0).y
	p.z += Data[-1].z; 	 // D(-1, 0, 0).z

	p.y += Data[ 1].y;        // D( 1, 0, 0).y
	p.z += Data[ 1].z;        // D( 1, 0, 0).z

	p.x += Data[sizex].x;     // D( 0, 1, 0).x
	p.z += Data[sizex].z;     // D( 0, 1, 0).z

	p.x += Data[d_xy].x;      // D( 0, 0, 1).x
	p.y += Data[d_xy].y;      // D( 0, 0, 1).y

	p *= a;
	q += p;
	q += bv;
	q *= c;
	p = q;
	p -= Data[0];

	Data[0] = q;

	return p.norm2();
}

float TSORSolver::solve_at_old(C3DFVector *Data,const C3DFVector& bv)
{
	C3DFVector *Data_alt = &Data[ -d_xy ];
	// caching data

	const C3DFVector Vp0m1m1 = Data_alt[ -size.x ];
	const C3DFVector Vm1p0m1 = Data_alt[ -1   ];
	const C3DFVector Vp0p0m1 = Data_alt[  0   ];
	const C3DFVector Vp1p0m1 = Data_alt[  1   ];
	const C3DFVector Vp0p1m1 = Data_alt[  size.x ];

	Data_alt = &Data[ -size.x ];
	const C3DFVector Vm1m1p0 = Data_alt[ -1 ];
	const C3DFVector Vp0m1p0 = Data_alt[  0 ];
	const C3DFVector Vp1m1p0 = Data_alt[  1 ];

	Data_alt = &Data[ size.x ];
	const C3DFVector Vm1p1p0 = Data_alt[ -1 ];
	const C3DFVector Vp0p1p0 = Data_alt[  0 ];
	const C3DFVector Vp1p1p0 = Data_alt[  1 ];

	const float  vxdxy = Vm1m1p0.x - Vp1m1p0.x + Vp1p1p0.x - Vm1p1p0.x;
	const float  vydxy = Vm1m1p0.y - Vp1m1p0.y + Vp1p1p0.y - Vm1p1p0.y;

	Data_alt = &Data[ d_xy ];
	const C3DFVector Vp0m1p1 = Data_alt[ -size.x ];
	const C3DFVector Vm1p0p1 = Data_alt[ -1   ];
	const C3DFVector Vp0p0p1 = Data_alt[  0   ];
	const C3DFVector Vp1p0p1 = Data_alt[  1   ];
	const C3DFVector Vp0p1p1 = Data_alt[  size.x ];

	const float  vxdxz = Vm1p0m1.x - Vp1p0m1.x + Vp1p0p1.x  - Vm1p0p1.x;
	const float  vzdxz = Vm1p0m1.z - Vp1p0m1.z + Vp1p0p1.z  - Vm1p0p1.z;



	const C3DFVector vdxx = Data[-1] + Data[1]; // 3A
	const C3DFVector vdyy = Vp0p1p0 + Vp0m1p0;
	const C3DFVector vdzz = Vp0p0p1 + Vp0p0m1;

	const float  vydyz = Vp0m1m1.y - Vp0p1m1.y + Vp0p1p1.y  - Vp0m1p1.y;
	const float  vzdyz = Vp0m1m1.z - Vp0p1m1.z + Vp0p1p1.z  - Vp0m1p1.z;



	const C3DFVector p((a_b)*vdxx.x + a*(vdyy.x+vdzz.x),        // 6A 6M
			   (a_b)*vdyy.y + a*(vdxx.y+vdzz.y),
			   (a_b)*vdzz.z + a*(vdxx.z+vdyy.z));

	const C3DFVector q(vydxy+vzdxz,vxdxy+vzdyz,vxdxz+vydyz);   // 3A
	const C3DFVector R = c *(bv + p + b_4 * q);            // 6A 3M

	const C3DFVector S = /*overrex */ ( R - *Data );      // 3A 6M
	*Data += S;                             // 3A


	return S.norm2();

}



int TSORSolver::solve(const C3DFVectorfield& b,C3DFVectorfield *xvf)
{
	assert(b.get_size() == xvf->get_size());


	size = b.get_size();
	d_xy = size.x * size.y;
	bool firsttime = true;
	int nIter = 0;
	float res;
	float firstres=0;
	do {

		nIter++;
		res = 0;
		int hardcode = d_xy + size.x + 1;
		for (size_t  z = 1; z < size.z - 1; z++) {
			cvinfo() << "SORA: [" << nIter << "]" << z << std::endl;
			for (size_t  y = 1; y < size.y - 1; y++){
				for (size_t  x = 1; x < size.x - 1; x++,hardcode++) {
					C3DFVector bv = b[hardcode]; // cache on stack

#ifdef USE_VERY_OLD
					res += solve_at_very_old(b,xvf,x,y,z);
#else
#ifndef USE_OLD
					res += solve_at(&(*xvf)[hardcode],bv);
#else
					res += solve_at_old(&(*xvf)[hardcode],bv);
#endif
#endif


				}
				hardcode+=2;
			}
			hardcode += size.x << 1;
		}
		if (firsttime) {
			firstres = res;
			firsttime = false;
		}
		cvinfo() << "SORA: [" << nIter << "]" <<res << "\n";
	}while (res > firstres * rel_res && nIter < max_steps && res > abs_res );

	// return why we have finished
	if (nIter >= max_steps) return 1;
	if (res < firstres * rel_res) return 2;
	if (res <abs_res) return 3;
	return 0;
}

TSORASolver::TSORASolver(int _max_steps, float _rel_res, float _abs_res,
			 float mu, float lambda):
	TSORSolver(_max_steps, _rel_res,_abs_res,mu,lambda)
{
}


int TSORASolver::solve(const C3DFVectorfield& b,C3DFVectorfield *xvf)
{
	float doorstep = 0;
	float res = 0;
	float firstres = 0;
	float lastres;


	assert(b.get_size() == xvf->get_size());
	size = b.get_size();
	d_xy = size.x * size.y;

	TUpdateInfo  *update_needed;
	TUpdateInfo  *need_update;
	TUpdateInfo update_info_1(size);
	TUpdateInfo update_info_2(size);
	C3DFDatafield residua(size);

	update_needed = &update_info_1;
	need_update = &update_info_2;

	//	xvf->clear();

	float  gSize = b.get_size().z *  b.get_size().y * b.get_size().x;

	cvinfo() << "SORA: [" << gSize << "]\n";
	// first run on full field
	int hardcode = d_xy + size.x + 1;
	for (size_t  z = 1; z < size.z-1; z++) {
		for (size_t  y = 1; y < size.y - 1; y++){
			for (size_t  x = 1; x < size.x - 1; x++,hardcode++){
				C3DFVector bv = b[hardcode]; // cache on stack
				cvinfo() << "SORA: [" << hardcode << "]" << bv << "\n";
				if (bv.norm2() > 0) {
					float step;
#ifndef USE_OLD
					firstres += (residua[hardcode] = step =
						     solve_at(&(*xvf)[hardcode],bv));
#else
					firstres += (residua[hardcode] = step =
					             solve_at_old(&(*xvf)[hardcode],bv));
#endif
					if (step > 0){
						cvinfo() << "update\n";
						update_needed->set_update(hardcode);
						cvinfo() << "done\n";

					}
				}

			}
			hardcode += 2;
		}
		hardcode += size.x << 1;
	}
	doorstep = firstres / gSize;
	lastres = firstres;
	int nIter = 1;
	cvinfo() << "SORA: [" << nIter << "]" << res << "\n";
	do {
		nIter++;
		res = 0;

		hardcode = d_xy + size.x + 1;
		for (size_t  z = 1; z < size.z-1; z++) {
			for (size_t  y = 1; y < size.y - 1; y++){
				auto ixvf =  &(*xvf)[hardcode]; 
				for (size_t  x = 1; x < size.x - 1; x++,hardcode++, ++ixvf){
					float  step;
					if ((*update_needed)[hardcode]) {
						C3DFVector bv = b[hardcode];
#ifndef USE_OLD
						residua[hardcode] = step =
							solve_at(ixvf,bv);
#else
						residua[hardcode] = step =
							solve_at_old(ixvf,bv);
#endif
					}else
						step = residua[hardcode];

					if (step > doorstep) {
						need_update->set_update(hardcode);
					}
					res += step;
				}
				hardcode += 2;
			}
			hardcode += size.x << 1;
		}

		doorstep = res * res / (gSize * lastres * nIter * nIter);
		lastres = res;

		// FIXME replace this by STL confom fill
		update_needed->clear();

		TUpdateInfo * update_temp = update_needed;
		update_needed = need_update;
		need_update = update_temp;
		cvinfo() << "SORA: [" << nIter << "]" << res << "\n";
	}while (res > firstres * rel_res && nIter < max_steps && res > abs_res );

	// return why we have finished
	if (nIter >= max_steps) return 1;
	if (res < firstres * rel_res) return 2;
	if (res <abs_res) return 3;
	return 0;
}

