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

#include <mia/internal/plugintester.hh>
#include <mia/2d/vfregularizerkernel/fluid-generic.hh>

using namespace mia; 



BOOST_AUTO_TEST_CASE( test_evaluate_row_evaluation_one_element ) 
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
	test_kernel << "fluid:mu=" << mu << ",lambda="<<lambda<<",relax="<< relax; 

        auto kernel = BOOST_TEST_create_from_plugin<C2DFVfFluidStandardRegularizerKernelPlugin>(test_kernel.str().c_str());
	
	C2DBounds size(3, 3); 
	C2DFVectorfield v(size); 
	C2DFVectorfield b(size); 

	C2DFVector b11(2,1); 

	b(1,1) = b11; 

	const C2DFVector Vmm0(1,2);
	const C2DFVector V0m0(2,4);
	const C2DFVector Vpm0(2,4);
	
	const C2DFVector Vm00(3,6);	
	const C2DFVector V000(4,2); 	
	const C2DFVector Vp00(5,1);	

	const C2DFVector Vmp0(0,2);
	const C2DFVector V0p0(9,5);
	const C2DFVector Vpp0(2,3);
	

	const C2DFVector fill(1000,2000); 
	
	const std::vector<C2DFVector> v_init = {
		Vmm0, V0m0, Vpm0,
		Vm00, V000, Vp00,
		Vmp0, V0p0, Vpp0,
	}; 
	
	copy(v_init.begin(), v_init.end(), v.begin()); 

	C2DFVector vdxx = Vm00 + Vp00; 
	C2DFVector vdyy = V0m0 + V0p0; 
	
	
	C2DFVector p(a_b*vdxx.x + a*vdyy.x,
		     a_b*vdyy.y + a*vdxx.y);
	

	float  vxdxy = Vmm0.x + Vpp0.x - Vpm0.x - Vmp0.x;
	float  vydxy = Vmm0.y + Vpp0.y - Vpm0.y - Vmp0.y;

	C2DFVector q(vydxy,vxdxy); 
	
	C2DFVector R = b11 + p + b_4 * q;
	C2DFVector delta = relax * (R -  V000);

	C2DFVector v000 = V000 + delta; 
	
	kernel->set_data_fields(&v, &b);

	auto buf = kernel->get_buffers(); 
	float residuum = kernel->evaluate_row(1, *buf);

	auto test_v = v(1,1); 

	BOOST_CHECK_CLOSE(residuum, delta.norm(), 0.1); 

	BOOST_CHECK_CLOSE(test_v.x, v000.x, 0.1); 
	BOOST_CHECK_CLOSE(test_v.y, v000.y, 0.1); 
	
}

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
	test_kernel << "fluid:mu=" << mu << ",lambda="<<lambda<<",relax="<< relax; 

        auto kernel = BOOST_TEST_create_from_plugin<C2DFVfFluidStandardRegularizerKernelPlugin>(test_kernel.str().c_str());
	
	C2DBounds size(5, 3); 
	C2DFVectorfield v(size); 
	C2DFVectorfield b(size); 

	C2DFVector b111(2,1); 
	C2DFVector b211(2,2); 
	C2DFVector b311(1,2); 

	b(1,1) = b111; 
	b(2,1) = b211; 
	b(3,1) = b311; 

	const C2DFVector fill(1000,2000); 

	const C2DFVector Vm0[5] = {C2DFVector(2,6), C2DFVector(1,3), C2DFVector(4,6), C2DFVector(2, 2), C2DFVector(5,2)}; 
	const C2DFVector V00[5] = {C2DFVector(3,2), C2DFVector(2,1), C2DFVector(3,1), C2DFVector(1, 3), C2DFVector(3,1)}; 
	C2DFVector T00[5]       = {C2DFVector(3,2), C2DFVector(2,1), C2DFVector(3,1), C2DFVector(1, 3), C2DFVector(3,1)}; 
	const C2DFVector Vp0[5] = {C2DFVector(1,4), C2DFVector(7,3), C2DFVector(1,6), C2DFVector(3, 2), C2DFVector(1,2)}; 
	

	copy(Vm0, Vm0 + 5, v.begin_at(0,0));
	copy(V00, V00 + 5, v.begin_at(0,1));
	copy(Vp0, Vp0 + 5, v.begin_at(0,2));


	float test_residuum = 0.0f; 

	for (int x = 1; x < 4; ++x) {
		C2DFVector vdxx = T00[x+1] + T00[x-1]; 
		C2DFVector vdyy = Vm0[x] + Vp0[x]; 
		
		C2DFVector p(a_b*vdxx.x + a*vdyy.x,
			     a_b*vdyy.y + a*vdxx.y);
		
		
		float  vydxy = Vm0[x-1].y + Vp0[x+1].y - Vm0[x+1].y - Vp0[x-1].y;
		float  vxdxy = Vm0[x-1].x + Vp0[x+1].x - Vm0[x+1].x - Vp0[x-1].x;
		
	
		
		C2DFVector q(vydxy,vxdxy); 
		
		C2DFVector R = b(x,1) + p + b_4 * q;
		C2DFVector delta = relax * (R -  T00[x]);

		cvdebug() << "vdxx="<< vdxx << ", vdyy=" << vdyy << "\n"; 
		cvdebug() << "p="<< p << ", q=" << q << ", delta=" << delta << "\n"; 

		test_residuum += delta.norm(); 

		T00[x] += delta;
		
	}

	
	kernel->set_data_fields(&v, &b);

	auto buf = kernel->get_buffers(); 
	float residuum = kernel->evaluate_row(1, *buf);
	
	BOOST_CHECK_CLOSE(residuum, test_residuum, 0.1); 


	for (int x = 1; x < 4; ++x) {
		BOOST_CHECK_CLOSE(v(x,1).x, T00[x].x, 0.1); 
		BOOST_CHECK_CLOSE(v(x,1).y, T00[x].y, 0.1); 
	}
	
}
