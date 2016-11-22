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

#ifndef mia_3d_vfregularizer_sor_hh
#define mia_3d_vfregularizer_sor_hh

#include <mia/3d/vfregularizer.hh>
#include <mia/3d/vfregularizerkernel.hh>

NS_MIA_BEGIN

class C3DSORVectorfieldRegularizer : public C3DFVectorfieldRegularizer {
      
public: 
        C3DSORVectorfieldRegularizer(float abs_epsilon, float rel_epsilon, 
                                     unsigned maxiter, 
                                     P3DVectorfieldRegularizerKernel kernel);
        
private: 
        double do_run(C3DFVectorfield& velocity, C3DFVectorfield& force, const C3DFVectorfield& deform) const; 
        
        float m_abs_epsilon;
        float m_rel_epsilon; 
        unsigned m_maxiter; 
        P3DVectorfieldRegularizerKernel m_kernel; 
                
};  

class C3DSORVectorfieldRegularizerPlugin : public C3DFVectorfieldRegularizerPlugin {

public: 
        C3DSORVectorfieldRegularizerPlugin(); 
        
private:
        
        C3DFVectorfieldRegularizer *do_create() const;
        
        const std::string do_get_descr() const;


        float m_abs_epsilon;
        float m_rel_epsilon; 
        unsigned m_maxiter; 
        P3DVectorfieldRegularizerKernel m_kernel; 
        
}; 

NS_MIA_END

#endif 
