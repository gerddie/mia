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

#include <mia/3d/vfregularizer/sor.hh>

class Mock3DVectorfieldRegularizerKernel: public C3DVectorfieldRegularizerKernel {
private: 
        virtual float do_evaluate_row(unsigned y, unsigned z, CBuffers& buffers); 
        virtual float do_evaluate_row_sparse(unsigned y, unsigned z, CBuffers& buffers); 
        virtual unsigned do_get_boundary_padding() const; 
        float solve_at(C3DFVector *v, const C3DFVector& b); 

}; 

BOOST_AUTO_TEST_CASE(test_sor_solver) 
{
        
}



float Mock3DVectorfieldRegularizerKernel::do_evaluate_row(unsigned y, unsigned z, 
                                                          CBuffers& MIA_PARAM_UNUSED(buffers))
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

float Mock3DVectorfieldRegularizerKernel::do_evaluate_row_sparse(unsigned y, unsigned z, 
                                                                 CBuffers& MIA_PARAM_UNUSED(buffers))
{
        
}

float Mock3DVectorfieldRegularizerKernel::solve_at(C3DFVector *v, const C3DFVector& b)
{
        // do a small gaussian patch 
        
        C3DFVector s = (6.0f * v[0] + 
                        v[1] + v[-1] + v[m_dx] + v[-m_dx] 
                        + v[m_dxy] + v[-m_dxy]) * 1.0f/12.0f; 
        
        C3DFVector r = 1.2 * ( b - s); 

        v[0] += r; 
        
        return r.norm(); 
}

unsigned Mock3DVectorfieldRegularizerKernel::do_get_boundary_padding() const
{
        return 1; 
}
