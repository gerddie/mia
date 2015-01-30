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

#include <mia/internal/plugintester.hh>
#include <mia/3d/vfregularizerkernel/fluid-generic.hh>

using namespace mia; 



BOOST_AUTO_TEST_CASE( test_evaluate_row_evaluation ) 
{
	float mu = 1.2; 
	float lambda = 1.1; 
	float relax = 1.8; 


	float c = 1 / (6.0f * mu + 2.0f * (mu + lambda));
	float a = mu * c; 
	float _b = (mu + lambda) * c; 

	float a_b = a + _b; // mu + lambda
	float b_4 = 0.25 * _b; // 0.25 * (mu + lambda)

	cvdebug() << "a=" << a << ", b4=" << b_4 << ", a+b=" << a_b << ", c=" << c << "\n"; 

	std::stringstream test_kernel; 
	test_kernel << "fluid-generic:mu=" << mu << ",lambda="<<lambda<<",relax="<< relax; 

        auto kernel = BOOST_TEST_create_from_plugin<C3DFVfFluidStandardRegularizerKernelPlugin>(test_kernel.str().c_str());
	
	C3DBounds size(3, 3, 3); 
	C3DFVectorfield v(size); 
	C3DFVectorfield b(size); 

	C3DFVector b111(2,1,1); 

	b(1,1,1) = b111; 

	const C3DFVector V0mm(0,2,7);
	const C3DFVector Vm0m(2,3,6);
	const C3DFVector V00m(3,6,9);
	const C3DFVector Vp0m(-2,-4,-5);
	const C3DFVector V0pm(2,3,3);

	const C3DFVector Vmm0(1,2,1);
	const C3DFVector V0m0(2,4,2);
	const C3DFVector Vpm0(2,4,5);
	
	const C3DFVector Vm00(3,6,4);	
	const C3DFVector V000(4,2,5); 	
	const C3DFVector Vp00(5,1,3);	

	const C3DFVector Vmp0(0,2,7);
	const C3DFVector V0p0(9,5,1);
	const C3DFVector Vpp0(2,3,3);
	

	const C3DFVector V0mp(2,-1,-7);
	const C3DFVector Vm0p(-3,-4,-8);
	const C3DFVector V00p(6,2,4);
	const C3DFVector Vp0p(3,2,1);
	const C3DFVector V0pp(1,2,1);

	const C3DFVector fill(1000,2000,3000); 
	
	const std::vector<C3DFVector> v_init = {
		fill, V0mm, fill, 
		Vm0m, V00m, Vp0m, 
		fill, V0pm, fill, 

		Vmm0, V0m0, Vpm0,
		Vm00, V000, Vp00,
		Vmp0, V0p0, Vpp0,

		fill, V0mp, fill, 
		Vm0p, V00p, Vp0p, 
		fill, V0pp, fill

	}; 
	
	copy(v_init.begin(), v_init.end(), v.begin()); 

	C3DFVector vdxx = Vm00 + Vp00; 
	C3DFVector vdyy = V0m0 + V0p0; 
	C3DFVector vdzz = V00m + V00p; 
	
	
	C3DFVector p(a_b*vdxx.x + a*(vdyy.x+vdzz.x),
		     a_b*vdyy.y + a*(vdxx.y+vdzz.y),
		     a_b*vdzz.z + a*(vdxx.z+vdyy.z));
	
	
	float  vydxy = Vmm0.y + Vpp0.y - Vpm0.y - Vmp0.y;
	float  vxdxy = Vmm0.x + Vpp0.x - Vpm0.x - Vmp0.x;

	float  vzdxz = Vm0m.z + Vp0p.z - Vp0m.z - Vm0p.z;
	float  vxdxz = Vm0m.x + Vp0p.x - Vp0m.x - Vm0p.x;

	float  vzdyz = V0mm.z + V0pp.z  - V0pm.z - V0mp.z;
	float  vydyz = V0mm.y + V0pp.y  - V0pm.y - V0mp.y;


	C3DFVector q(vydxy+vzdxz,vxdxy+vzdyz,vxdxz+vydyz); 
	
	C3DFVector R = b111 + p + b_4 * q;
	C3DFVector delta = relax * (R -  V000);

	C3DFVector v000 = V000 + delta; 
	
	kernel->set_data_fields(&v, &b);

	auto buf = kernel->get_buffers(); 
	float residuum = kernel->evaluate_row(1,1, *buf);

	auto test_v = v(1,1,1); 

	BOOST_CHECK_CLOSE(residuum, delta.norm2(), 0.1); 

	BOOST_CHECK_CLOSE(test_v.x, v000.x, 0.1); 
	BOOST_CHECK_CLOSE(test_v.y, v000.y, 0.1); 
	BOOST_CHECK_CLOSE(test_v.z, v000.z, 0.1); 
	
	
}

