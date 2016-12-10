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

#ifndef mia_2d_vfregularizer_sor_hh
#define mia_2d_vfregularizer_sor_hh

#include <mia/2d/vfregularizer.hh>
#include <mia/2d/vfregularizerkernel.hh>

NS_MIA_BEGIN

class C2DSORVectorfieldRegularizer : public C2DFVectorfieldRegularizer {
      
public: 
        C2DSORVectorfieldRegularizer(float abs_epsilon, float rel_epsilon, 
                                     unsigned maxiter, 
                                     P2DVectorfieldRegularizerKernel kernel);
        
private: 
        double do_run(C2DFVectorfield& velocity, C2DFVectorfield& force, const C2DFVectorfield& deform) const override; 
        
        float m_abs_epsilon;
        float m_rel_epsilon; 
        unsigned m_maxiter; 
        P2DVectorfieldRegularizerKernel m_kernel; 
                
};  

class C2DSORVectorfieldRegularizerPlugin : public C2DFVectorfieldRegularizerPlugin {

public: 
        C2DSORVectorfieldRegularizerPlugin(); 
        
private:
        
        C2DFVectorfieldRegularizer *do_create() const;
        
        const std::string do_get_descr() const;


        float m_abs_epsilon;
        float m_rel_epsilon; 
        unsigned m_maxiter; 
        P2DVectorfieldRegularizerKernel m_kernel; 
        
}; 

NS_MIA_END

#endif 
