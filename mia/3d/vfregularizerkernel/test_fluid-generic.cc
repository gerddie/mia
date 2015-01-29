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


	const C3DFVector Vm1m1p0(1,2,1);
	const C3DFVector Vp1p1p0(2,3,3);
	const C3DFVector Vp1m1p0(2,4,5);
	const C3DFVector Vm1p1p0(0,2,7);

	const C3DFVector Vm1p0m1(2,3,6);
	const C3DFVector Vp1p0p1(3,2,1);
	const C3DFVector Vp1p0m1(-2,-4,-5);
	const C3DFVector Vm1p0p1(-3,-4,-8);

	const C3DFVector Vp0m1m1(0,2,7);
	const C3DFVector Vp0p1p1(1,2,1);
	const C3DFVector Vp0p1m1(2,3,3);
	const C3DFVector Vp0m1p1(2,-1,-7);

	const C3DFVector Vp0p0m1(3,6,9);
	const C3DFVector Vp0p0p1(6,2,4);

	const C3DFVector Vp0m1p0(2,4,2);
	const C3DFVector Vp0p1p0(9,5,1);

	const C3DFVector Vm1p0p0(3,6,4);
	const C3DFVector Vp1p0p0(5,1,3);

	const C3DFVector Vp0p0p0(4,2,5); 
	
	const C3DFVector V_fill_(1000,2000,3000); 
	
	const std::vector<C3DFVector> v_init = {
		V_fill_, Vp0m1m1, V_fill_, 
		Vm1p0m1, Vp0p0m1, Vp1p0m1, 
		V_fill_, Vp0p1m1, V_fill_, 


		Vm1m1p0, Vp0m1p0, Vp1m1p0,
		Vm1p0p0, Vp0p0p0, Vp1p0p0,
		Vm1p1p0, Vp0p1p0, Vp1p1p0,

		V_fill_, Vp0m1p1, V_fill_, 
		Vm1p0p1, Vp0p0p1, Vp1p0p1, 
		V_fill_, Vp0p1p1, V_fill_

	}; 
	
	copy(v_init.begin(), v_init.end(), v.begin()); 

	C3DFVector vdxx = Vm1p0p0 + Vp1p0p0; 
	C3DFVector vdyy = Vp0m1p0 + Vp0p1p0; 
	C3DFVector vdzz = Vp0p0m1 + Vp0p0p1; 
	
	
	C3DFVector p(a_b*vdxx.x + a*(vdyy.x+vdzz.x),
		     a_b*vdyy.y + a*(vdxx.y+vdzz.y),
		     a_b*vdzz.z + a*(vdxx.z+vdyy.z));
	
	
	float  vydxy = Vm1m1p0.y + Vp1p1p0.y - Vp1m1p0.y - Vm1p1p0.y;
	float  vxdxy = Vm1m1p0.x + Vp1p1p0.x - Vp1m1p0.x - Vm1p1p0.x;

	float  vzdxz = Vm1p0m1.z + Vp1p0p1.z - Vp1p0m1.z - Vm1p0p1.z;
	float  vxdxz = Vm1p0m1.x + Vp1p0p1.x - Vp1p0m1.x - Vm1p0p1.x;

	float  vzdyz = Vp0m1m1.z + Vp0p1p1.z  - Vp0p1m1.z - Vp0m1p1.z;
	float  vydyz = Vp0m1m1.y + Vp0p1p1.y  - Vp0p1m1.y - Vp0m1p1.y;


	C3DFVector q(vydxy+vzdxz,vxdxy+vzdyz,vxdxz+vydyz); 
	
	C3DFVector R = b111 + p + b_4 * q;
	C3DFVector delta = relax * (R -  Vp0p0p0);

	C3DFVector v000 = Vp0p0p0 + delta; 
	
	kernel->set_data_fields(&v, &b);

	float residuum = kernel->evaluate_row(1,1);

	auto test_v = v(1,1,1); 

	BOOST_CHECK_CLOSE(residuum, delta.norm2(), 0.1); 

	BOOST_CHECK_CLOSE(test_v.x, v000.x, 0.1); 
	BOOST_CHECK_CLOSE(test_v.y, v000.y, 0.1); 
	BOOST_CHECK_CLOSE(test_v.z, v000.z, 0.1); 
	
	
}

