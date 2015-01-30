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

#ifndef mia_3d_vfregularizerkernel_fluid_standard_hh
#define mia_3d_vfregularizerkernel_fluid_standard_hh

#include <mia/3d/vfregularizerkernel.hh>

NS_MIA_BEGIN

class  C3DFVfFluidStandardRegularizerKernel: public C3DFVectorfieldRegularizerKernel {
public: 
        C3DFVfFluidStandardRegularizerKernel(float mu, float lambda, float relax); 
        
private: 
	void post_set_data_fields(); 

        float do_evaluate_row(unsigned y, unsigned z, CBuffers& buf); 
                
        float do_evaluate_row_sparse(unsigned y, unsigned z, CBuffers& buf); 

	float do_evaluate_pertuberation_row(unsigned  y, unsigned  z, CBuffers& buffers) const; 

	unsigned do_get_boundary_padding() const; 
        
        void set_update_flags(unsigned idx); 

	float solve_at(C3DFVector *v, const C3DFVector& b); 

        float m_a; 
        float m_a_b; 
        float m_b4; 
        float m_relax; 
        float m_c; 
        int m_dx;
	int m_dxy;

}; 

class  C3DFVfFluidStandardRegularizerKernelPlugin: public C3DFVectorfieldRegularizerKernelPlugin {
public: 
        C3DFVfFluidStandardRegularizerKernelPlugin(); 
        
private: 
        C3DFVfFluidStandardRegularizerKernel *do_create() const;
        
        const std::string do_get_descr() const;

        float m_mu; 
        float m_lambda; 
        float m_relax; 
}; 

NS_MIA_END
#endif 
