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

#include <mia/core/export_handler.hh>
#include <mia/2d/vfregularizerkernel.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>
#include <cassert>

NS_MIA_BEGIN

const char *C2DFVectorfieldRegularizerKernel::type_descr = "regularizerkernel";

C2DFVectorfieldRegularizerKernel::C2DFVectorfieldRegularizerKernel(bool has_pertuberation):
m_has_pertuberation(has_pertuberation)
{
}

C2DFVectorfieldRegularizerKernel::~C2DFVectorfieldRegularizerKernel()
{
}

float C2DFVectorfieldRegularizerKernel::evaluate_pertuberation_row(unsigned  y, CBuffers& buffers) const
{
	return do_evaluate_pertuberation_row(y,buffers); 
}

void C2DFVectorfieldRegularizerKernel::set_data_fields(C2DFVectorfield *output, const C2DFVectorfield  *input)
{
        m_output = output; 
        m_input = input; 
	post_set_data_fields(); 
}


void C2DFVectorfieldRegularizerKernel::set_update_fields(const T2DDatafield<unsigned char> *update_flags, 
                                                         T2DDatafield<unsigned char> *set_flags, 
							 T2DDatafield<float> *residua, 
							 float residual_thresh)
{
        m_update_flags = update_flags; 
        m_set_flags = set_flags; 
        m_residua = residua; 
	m_residual_thresh = residual_thresh; 
}

float C2DFVectorfieldRegularizerKernel::evaluate_row(unsigned y, CBuffers& buffers)
{
        assert(m_output); 
        assert(m_input); 
        
        return do_evaluate_row(y, buffers); 
}

float C2DFVectorfieldRegularizerKernel::evaluate_row_sparse(unsigned y, CBuffers& buffers)
{
        assert(m_output); 
        assert(m_input); 
        assert(m_residua); 
        assert(m_update_flags); 
        assert(m_set_flags); 

        return do_evaluate_row_sparse(y, buffers); 
}

C2DFVectorfieldRegularizerKernel::PBuffers 
C2DFVectorfieldRegularizerKernel::get_buffers() const
{
	return do_get_buffers(); 
}

void C2DFVectorfieldRegularizerKernel::start_row(unsigned y, CBuffers& buffers) const
{
	do_start_row(y, buffers); 
}


C2DFVectorfieldRegularizerKernel::PBuffers
C2DFVectorfieldRegularizerKernel::do_get_buffers() const
{
	return PBuffers(new CBuffers()); 
}

float C2DFVectorfieldRegularizerKernel::do_evaluate_pertuberation_row(unsigned  MIA_PARAM_UNUSED(y), 
								      CBuffers& MIA_PARAM_UNUSED(buffers)) const
{
	assert(!m_has_pertuberation && "The kernel says it has a pertuberation evaluator, "
	       "but 'do_evaluate_pertuberation_row' has not been overriden"); 
	return 0.0; 
}

void C2DFVectorfieldRegularizerKernel::do_start_row(unsigned MIA_PARAM_UNUSED(y), 
						      CBuffers& MIA_PARAM_UNUSED(buffers)) const
{
}

void C2DFVectorfieldRegularizerKernel::post_set_data_fields()
{
}

unsigned C2DFVectorfieldRegularizerKernel::get_boundary_padding() const
{
	return do_get_boundary_padding(); 
}

C2DFVectorfieldRegularizerKernel::CBuffers::~CBuffers()
{
}

template <> const char *  const 
TPluginHandler<C2DFVectorfieldRegularizerKernelPlugin>::m_help =  
        "This class of plug-ins implement various regularizations kernels for "
        "2D vector fields regularization solvers that are typically used in image registration "
        "to translate the force driving the registration to a smooth "
        "velocity field.";

EXPLICIT_INSTANCE_HANDLER(C2DFVectorfieldRegularizerKernel); 


NS_MIA_END

