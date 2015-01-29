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

#include <mia/3d/vfregularizerkernel/fluid-generic.hh>

NS_MIA_BEGIN

C3DFVfFluidStandardRegularizerKernel::C3DFVfFluidStandardRegularizerKernel(float mu, float lambda, float relax):m_relax(relax), 
        m_dx(0), 
        m_dxy(0)
        
{
        float a = mu;
	float b = lambda + mu;
	m_c = 1 / (6.0f * a + 2.0f * b);
	m_b4 = 0.25f * b * m_c;
	m_a_b = ( a + b ) * m_c;
	m_a = a * m_c;
}

void C3DFVfFluidStandardRegularizerKernel::post_set_data_fields()
{
        m_dx = get_input_field().get_size().x; 
        m_dxy = m_dx * get_input_field().get_size().y; 
}

float C3DFVfFluidStandardRegularizerKernel::do_evaluate_row(unsigned y, unsigned z)
{
        float residuum = 0.0f; 
        unsigned linear_index = 1 + y * m_dx + z * m_dxy; 
        auto& v = get_output_field(); 
        auto& b = get_input_field(); 

        for (int x = 1; x < m_dx - 1; ++x, ++linear_index) {
                residuum += solve_at(&v[linear_index], b[linear_index]); 
        }
        return residuum; 
}

float C3DFVfFluidStandardRegularizerKernel::do_evaluate_row_sparse(unsigned y, unsigned z)
{
        float residuum = 0.0f; 
        unsigned linear_index = 1 + y * m_dx + z * m_dxy; 
        auto& v = get_output_field(); 
        auto& b = get_input_field(); 
        auto& update = get_update_flags(); 
        auto& res = get_residua(); 
        
        for (int x = 1; x < m_dx - 1; ++x, ++linear_index) {
                if (update[linear_index]) {
                        res[linear_index] = solve_at(&v[linear_index], b[linear_index]); 
                }
                residuum += res[linear_index];
                
                if (res[linear_index] > get_residual_thresh()) {
                        set_update_flags(linear_index); 
                }
        }
        return residuum; 

}

float C3DFVfFluidStandardRegularizerKernel::solve_at(C3DFVector *v, const C3DFVector& b)
{
        C3DFVector *v_loc = &v[ -m_dxy ];

	const C3DFVector Vp0m1m1 = v_loc[ -m_dx ];
	const C3DFVector Vm1p0m1 = v_loc[ -1   ];
	const C3DFVector Vp0p0m1 = v_loc[  0   ];
	const C3DFVector Vp1p0m1 = v_loc[  1   ];
	const C3DFVector Vp0p1m1 = v_loc[  m_dx ];

	v_loc = &v[ -m_dx ];
	const C3DFVector Vm1m1p0 = v_loc[ -1 ];
	const C3DFVector Vp0m1p0 = v_loc[  0 ];
	const C3DFVector Vp1m1p0 = v_loc[  1 ];

	v_loc = &v[ m_dx ];
	const C3DFVector Vm1p1p0 = v_loc[ -1 ];
	const C3DFVector Vp0p1p0 = v_loc[  0 ];
	const C3DFVector Vp1p1p0 = v_loc[  1 ];

	const float  vxdxy = Vm1m1p0.x - Vp1m1p0.x + Vp1p1p0.x - Vm1p1p0.x;
	const float  vydxy = Vm1m1p0.y - Vp1m1p0.y + Vp1p1p0.y - Vm1p1p0.y;

	v_loc = &v[ m_dxy ];
	const C3DFVector Vp0m1p1 = v_loc[ -m_dx ];
	const C3DFVector Vm1p0p1 = v_loc[ -1   ];
	const C3DFVector Vp0p0p1 = v_loc[  0   ];
	const C3DFVector Vp1p0p1 = v_loc[  1   ];
	const C3DFVector Vp0p1p1 = v_loc[  m_dx ];

	const float  vxdxz = Vm1p0m1.x - Vp1p0m1.x + Vp1p0p1.x  - Vm1p0p1.x;
	const float  vzdxz = Vm1p0m1.z - Vp1p0m1.z + Vp1p0p1.z  - Vm1p0p1.z;

	const C3DFVector vdxx = v[-1] + v[1]; // 3A
	const C3DFVector vdyy = Vp0p1p0 + Vp0m1p0;
	const C3DFVector vdzz = Vp0p0p1 + Vp0p0m1;

	const float  vydyz = Vp0m1m1.y - Vp0p1m1.y + Vp0p1p1.y  - Vp0m1p1.y;
	const float  vzdyz = Vp0m1m1.z - Vp0p1m1.z + Vp0p1p1.z  - Vp0m1p1.z;

	const C3DFVector p(m_a_b*vdxx.x + m_a*(vdyy.x+vdzz.x),        // 6A 6M
			   m_a_b*vdyy.y + m_a*(vdxx.y+vdzz.y),
			   m_a_b*vdzz.z + m_a*(vdxx.z+vdyy.z));

	const C3DFVector q(vydxy+vzdxz,vxdxy+vzdyz,vxdxz+vydyz);   // 3A
	const C3DFVector R = b + p + m_b4 * q;                // 6A 3M

	const C3DFVector delta = m_relax * ( R - *v );              // 3A 3M
	*v += delta;                                                // 3A

	return delta.norm2();

}

void C3DFVfFluidStandardRegularizerKernel::set_update_flags(unsigned idx)
{
        auto& sf = get_set_flags();
        
        unsigned idx_mz = idx - m_dxy; 

        sf[idx_mz - m_dx] = 1; 
        sf[idx_mz - 1] = 1; 
        sf[idx_mz] = 1; 
        sf[idx_mz + 1] = 1; 
        sf[idx_mz + m_dx] = 1; 

        unsigned idx_my = idx - m_dx; 

        sf[idx_my - 1] = 1; 
        sf[idx_my] = 1; 
        sf[idx_my + 1] = 1; 

        sf[idx - 1] = 1; 
        sf[idx] = 1; 
        sf[idx + 1] = 1; 

        unsigned idx_py = idx + m_dx; 

        sf[idx_py - 1] = 1; 
        sf[idx_py] = 1; 
        sf[idx_py + 1] = 1; 

        unsigned idx_pz = idx - m_dxy; 
        
        sf[idx_pz - m_dx] = 1; 
        sf[idx_pz - 1] = 1; 
        sf[idx_pz] = 1; 
        sf[idx_pz + 1] = 1; 
        sf[idx_pz + m_dx] = 1; 

	
}

C3DFVfFluidStandardRegularizerKernelPlugin::C3DFVfFluidStandardRegularizerKernelPlugin():
        C3DFVectorfieldRegularizerKernelPlugin("fluid-generic"),
        m_mu(1.0f), 
        m_lambda(1.0f), 
        m_relax(1.0)
{
        this->add_parameter("mu", new mia::CFloatParameter(m_mu, 1e-10, 10000.0, false, "dynamic viscosity (shear)"));  
        this->add_parameter("lambda", new mia::CFloatParameter(m_lambda, 1e-10, 10000.0, false, "bulk viscosity (compressibility)"));  
        this->add_parameter("relax", new mia::CFloatParameter(m_relax, 0.1, 2.0, false, "Relaxation parameter for the solver"));  
}
         
C3DFVfFluidStandardRegularizerKernel *C3DFVfFluidStandardRegularizerKernelPlugin::do_create() const
{
        return new C3DFVfFluidStandardRegularizerKernel(m_mu, m_lambda, m_relax); 
}

const std::string C3DFVfFluidStandardRegularizerKernelPlugin::do_get_descr() const
{
        return "Evaluation kernel for the fluid-dynamics solver, either using "
                "successive (over-)relaxation, or a Gauss-Southwell relaxation. "
                "This implementation is generic and doesn't implement any "
                "architecture specific optimizations."; 
}

NS_MIA_END
     
extern "C" EXPORT mia::CPluginBase *get_plugin_interface()
{
	return new mia::C3DFVfFluidStandardRegularizerKernelPlugin();
}







