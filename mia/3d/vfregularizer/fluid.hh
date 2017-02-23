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


#include <mia/3d/vectorfieldregularizer.hh>

NS_BEGIN(fluid_vfregularizer) 

class C3DFluidVectorfieldRegularizer: public mia::C3DFVectorfieldRegularizer {
public: 
        C3DFluidVectorfieldRegularizer(float mu, float lambda, size_t maxiter, float omega, float epsilon);  
        
private: 
        virtual double do_run(C3DFVectorfield& output, C3DFVectorfield& input) const; 
        virtual void on_size_changed(); 

        float m_mu;
	float m_lambda;
	float m_omega;
	float m_epsilon;
	size_t m_max_iter;
	float m_a,  m_c, m_a_b, m_b_4;
	int m_dx;
	int m_dxy;



}; 
                
