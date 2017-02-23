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

#include <iomanip>
#include <cassert>
#include <cmath>

#include <gsl/gsl_linalg.h>
#include <mia/core/scaler1d.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN

using namespace std; 
C1DScalar::C1DScalar(const CSplineKernel& kernel, size_t in_size, size_t out_size):
	m_in_size(in_size), 
	m_support(kernel.size()), 
	m_poles(kernel.get_poles()),
	m_strategy(scs_unknown), 
	m_bc(produce_spline_boundary_condition("mirror")), 
	m_input_buffer(in_size), 
	m_out_size(out_size), 
	m_output_buffer(out_size)
{
	assert(in_size); 
	assert(out_size); 
	m_scale = static_cast<double>(out_size - 1) / static_cast<double>(in_size - 1); 
	initialize(kernel); 
}

void C1DScalar::initialize(const CSplineKernel& kernel)
{
	assert(m_scale > 0.0); 
	
	if (m_scale > 1.0) {
		// prepare for upscaling 
		m_bc->set_width(m_in_size); 
		
		if (m_in_size == 1) {
			m_strategy = scs_fill_output; 
		} else {
			m_strategy = scs_upscale; 
			const double dx = 1.0/m_scale; 
			double x = 0; 
			for(size_t i = 0; i < m_out_size; ++i, x+= dx) {
				CSplineKernel::VWeight weight(m_support); 
				CSplineKernel::VIndex index(m_support); 
				kernel(x, weight, index); 
				m_bc->apply(index, weight); 
				m_weights.push_back(weight); 
				m_indices.push_back(index); 
			}
		}
	} else if (m_scale == 1.0){
		m_strategy = scs_copy; 
		m_bc->set_width(m_in_size); 
	} else {
		m_bc->set_width(m_in_size), 
		m_strategy = scs_downscale; 
		const double dx = 1.0/m_scale; 
		CSplineKernel::VWeight weight(m_support); 
		CSplineKernel::VIndex index(m_support); 
		
		stringstream gdescr; 
		gdescr << "gauss:w=" << static_cast<int>(floor(1.0 / m_scale + 0.5)); 
		m_gauss = produce_spacial_kernel(gdescr.str()); 

		cvdebug() << "downscale: prefilter='" << gdescr.str() << "'\n"; 
		for(size_t i = 0; i < m_out_size; ++i) {
			CSplineKernel::VWeight weight(m_support); 
			CSplineKernel::VIndex index(m_support); 
			kernel(i * dx, weight, index); 
			m_bc->apply(index, weight); 
			m_weights.push_back(weight); 
			m_indices.push_back(index); 
			cvdebug() << "["<< setw(2) << i << "]" << setw(10) << i*dx << ", "
				  << index << "\n"; 

		}
	}
}


C1DScalar::C1DScalar(const CSplineKernel& kernel, size_t in_size, double scale):
	m_in_size(in_size), 
	m_support(kernel.size()), 
	m_scale(scale), 
	m_poles(kernel.get_poles()),
	m_strategy(scs_unknown), 
	m_bc(produce_spline_boundary_condition("mirror")), 
	m_input_buffer(in_size), 
	m_out_size(ceil((in_size-1)  * m_scale  + 1)), 
	m_output_buffer(m_out_size)
{
	initialize(kernel); 
}
	

size_t C1DScalar::get_output_size() const 
{
	return m_out_size; 
}
 
void C1DScalar::operator () (const std_double_vector& input, std_double_vector& output) const
{
	switch (m_strategy) {
	case scs_fill_output: fill(output.begin(), output.end(), input[0]); 
		break; 
	case scs_upscale: upscale(input, output); 
		break; 
	case scs_copy: output = input; 
		break; 
	case scs_downscale: 
		downscale(input, output);
		break; 
	default: 
		assert(0 && "Programming error: unknown scaling strategy"); 
	};
}

C1DScalar::std_double_vector::iterator C1DScalar::input_begin()
{
	return m_input_buffer.begin(); 
}

C1DScalar::std_double_vector::iterator C1DScalar::input_end() 
{
	return m_input_buffer.end(); 
}

C1DScalar::std_double_vector::iterator C1DScalar::output_begin()
{
	return m_output_buffer.begin(); 
}

C1DScalar::std_double_vector::iterator C1DScalar::output_end() 
{
	return m_output_buffer.end();
}



void C1DScalar::run()

{
	switch (m_strategy) {
	case scs_fill_output: fill(output_begin(), output_end(), m_input_buffer[0]); 
		break; 
	case scs_upscale: upscale(m_input_buffer, m_output_buffer); 
		break; 
	case scs_copy: copy(m_input_buffer.begin(), m_input_buffer.end(), 
			    m_output_buffer.begin()); 
		break; 
	case scs_downscale: 
		downscale(m_input_buffer, m_output_buffer);
		break; 
	default: 
		assert(0 && "Programming error: unknown scaling strategy"); 
	};
	
}

void C1DScalar::upscale(const std_double_vector& input, std_double_vector& output) const
{
	TRACE_FUNCTION; 
	auto io = output.begin(); 
	// the input should be filtered 

	vector<double> coeffs(input.begin(), input.end()); 
	m_bc->filter_line(coeffs, m_poles); 

	for (size_t i = 0; i < output.size(); ++i, ++io) {
		const auto& weight = m_weights[i]; 
		const auto& index = m_indices[i]; 
		double v = 0.0; 
		for (size_t k = 0; k < m_support; ++k)
			v += weight[k] * coeffs[index[k]]; 
		*io = v; 
	}
}

void C1DScalar::downscale(const std_double_vector& input, std_double_vector& output) const
{
	std_double_vector coeffs = m_gauss->apply(input);
	m_bc->filter_line(coeffs, m_poles); 
	for (size_t i = 0; i < output.size(); ++i) {
		const auto& weight = m_weights[i]; 
		const auto& index = m_indices[i]; 
		double v = 0.0;
		for (size_t k = 0; k < m_support; ++k)
			v += weight[k] * coeffs[index[k]]; 
		output[i] = v; 
	}
}

NS_MIA_END
