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

#include <mia/3d/vfregularizer/sor.hh>
#include <mia/core/parallel.hh>

NS_MIA_BEGIN

C3DSORVectorfieldRegularizer::C3DSORVectorfieldRegularizer(float abs_epsilon, float rel_epsilon, 
                                                           unsigned maxiter, 
                                                           P3DVectorfieldRegularizerKernel kernel):
        m_abs_epsilon(abs_epsilon), 
        m_rel_epsilon(rel_epsilon), 
        m_maxiter(maxiter), 
        m_kernel(kernel)
{
}

double C3DSORVectorfieldRegularizer::do_run(C3DFVectorfield& velocity, C3DFVectorfield& force, const C3DFVectorfield& deform) const
{
        m_kernel->set_data_fields(&velocity, &force);

        unsigned padding = m_kernel->get_boundary_padding(); 

        unsigned iter = 0; 
        float residuum = 0.0; 
        float first_rel_residuum = -1.0; 

        unsigned work_size_z = velocity.get_size().z - padding;  
        unsigned work_size_y = velocity.get_size().y - padding;  
        
        do {
                iter++; 

                residuum = 0.0f; 

		// to be parallized, needs reduce for residuum

		auto callback_solver  = [this, padding, work_size_y]
			(const C1DParallelRange& range, float res) {
			auto buffers = m_kernel->get_buffers(); 
			for (auto  z = range.begin(); z < range.end(); ++z) {
				m_kernel->start_slice(z, *buffers);
				for (unsigned y = padding; y < work_size_y; ++y) {
					res += m_kernel->evaluate_row(y, z, *buffers);
				}
			}
			return res; 
		};
		
		residuum = preduce(C1DParallelRange(padding, work_size_z), 0.0f, callback_solver,
					[](float a, float b){ return a+b;}); 
				
		if (first_rel_residuum < 0) 
			first_rel_residuum = m_rel_epsilon * residuum; 
                
                
		cvinfo() << "[" << iter << "] res=" << residuum << "\n"; 
        } while (iter < m_maxiter && 
                 residuum > m_abs_epsilon && 
                 residuum > first_rel_residuum);

       
        float max_pert = 0.0f; 
        if (m_kernel->has_pertuberation()) {
                m_kernel->set_data_fields(&velocity, &deform);

		auto callback_pert = [this, padding, work_size_y]
			(const C1DParallelRange& range, float maxpert) {
			
			// to be parallized (reduce because of max ) 
			auto buffers = m_kernel->get_buffers();
			for (auto  z = range.begin(); z < range.end(); ++z) {
				m_kernel->start_slice(z, *buffers);
				for (unsigned y = padding; y < work_size_y; ++y) {
					float pert = m_kernel->evaluate_pertuberation_row(y, z, *buffers);
					if (maxpert < pert) 
						maxpert = pert; 
				}
			}
			return maxpert; 
                };
		
		max_pert = preduce(C1DParallelRange(padding, work_size_z), 0.0f, callback_pert,
				   [](float a, float b){ return std::max(a,b);});
	}else{
                // find maximum in velocity field 
                for (auto v : velocity) {
                        float pert = v.norm2(); 
                        if (max_pert < pert) 
                                max_pert = pert; 
                }
        }
        return sqrt(max_pert);
}

C3DSORVectorfieldRegularizerPlugin::C3DSORVectorfieldRegularizerPlugin():
        C3DFVectorfieldRegularizerPlugin("sor"), 
        m_abs_epsilon(0.01), 
        m_rel_epsilon(1e-5), 
        m_maxiter(100)
{
        add_parameter("rel_f", make_oi_param(m_rel_epsilon, 0.0, 1.0, false, 
                                                        "breaking condition: relative residuum"));  
        add_parameter("abs_f", make_lc_param(m_abs_epsilon, 0.0, false, 
                                                        "breaking condition: absolute residuum"));  
        add_parameter("maxiter", make_lc_param(m_maxiter, 0, false, "maximum number of iterations"));  
        
        // there should be some method to select the arch optimized version 
        // 
        add_parameter("kernel", make_param(m_kernel, "fluid", false, "solver kernel to be used")); 

}
        

C3DFVectorfieldRegularizer *C3DSORVectorfieldRegularizerPlugin::do_create() const
{

        return new C3DSORVectorfieldRegularizer(m_abs_epsilon, m_rel_epsilon, 
                                                m_maxiter, m_kernel); 
}
        
const std::string C3DSORVectorfieldRegularizerPlugin::do_get_descr() const
{
        return "This plugin implements successive (over-)relaxation as a solver "
                "to regularize the vector field."; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSORVectorfieldRegularizerPlugin();
}

NS_MIA_END
