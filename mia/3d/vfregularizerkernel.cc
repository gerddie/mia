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

#include <mia/core/export_handler.hh>
#include <mia/3d/vfregularizerkernel.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>
#include <cassert>

NS_MIA_BEGIN

const char *C3DFVectorfieldRegularizerKernel::type_descr = "regularizerkernel";

C3DFVectorfieldRegularizerKernel::~C3DFVectorfieldRegularizerKernel()
{
}

void C3DFVectorfieldRegularizerKernel::set_data_fields(C3DFVectorfield *output, C3DFVectorfield  *input)
{
        m_output = output; 
        m_input = input; 
	post_set_data_fields(); 
}


void C3DFVectorfieldRegularizerKernel::set_update_fields(const T3DDatafield<unsigned char> *update_flags, 
                                                         T3DDatafield<unsigned char> *set_flags, 
							 T3DDatafield<float> *residua, 
							 float residual_thresh)
{
        m_update_flags = update_flags; 
        m_set_flags = set_flags; 
        m_residua = residua; 
	m_residual_thresh = residual_thresh; 
}

float C3DFVectorfieldRegularizerKernel::evaluate_row(unsigned y, unsigned z)
{
        assert(m_output); 
        assert(m_input); 
        
        return do_evaluate_row(y,z); 
}

float C3DFVectorfieldRegularizerKernel::evaluate_row_sparse(unsigned y, unsigned z)
{
        assert(m_output); 
        assert(m_input); 
        assert(m_residua); 
        assert(m_update_flags); 
        assert(m_set_flags); 

        return do_evaluate_row_sparse(y,z); 
}



void C3DFVectorfieldRegularizerKernel::post_set_data_fields()
{
}

unsigned C3DFVectorfieldRegularizerKernel::get_boundary_padding() const
{
	return do_get_boundary_padding(); 
}

template <> const char *  const 
TPluginHandler<C3DFVectorfieldRegularizerKernelPlugin>::m_help =  
        "This class of plug-ins implement various regularizations kernels for "
        "3D vector fields regularization solvers that are typically used in image registration "
        "to translate the force driving the registration to a smooth "
        "velocity field.";

EXPLICIT_INSTANCE_HANDLER(C3DFVectorfieldRegularizerKernel); 


NS_MIA_END

