/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <cassert>
#include <cmath>

#include <gsl/gsl_linalg.h>
#include <mia/core/scaler1d.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

using namespace std; 
C1DScalarFixed::C1DScalarFixed(const CSplineKernel& kernel, size_t in_size, size_t out_size):
	m_in_size(in_size), 
	m_support(kernel.size()), 
	m_poles(kernel.get_poles()),
	m_strategy(scs_unknown), 
	m_bc(produce_spline_boundary_condition("mirror")), 
	m_out_size(out_size), 
	m_input_buffer(in_size, false), 
	m_output_buffer(out_size, false)
{
	assert(in_size); 
	assert(out_size); 
	m_scale = static_cast<double>(out_size - 1) / static_cast<double>(in_size - 1); 
	initialize(kernel); 
}

void C1DScalarFixed::initialize(const CSplineKernel& kernel)
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
		m_bc->set_width(m_out_size), 
		m_strategy = scs_downscale; 
		// prepare for downscaling 
		const double dx = m_scale; 
		CSplineKernel::VWeight weight(m_support); 
		CSplineKernel::VIndex index(m_support); 

		m_A = gsl::Matrix(m_in_size, m_out_size,  true);
		m_tau = gsl::DoubleVector(m_out_size, false); 
		for (size_t k = 0; k < m_out_size; ++k) {
			double x = 0; 
			for (size_t j = 0; j < m_in_size; ++j, x+=dx) {
				kernel(x, weight, index);
				m_bc->apply(index, weight); 
				for (size_t i = 0; i < m_support; ++i) {
					double v = m_A(j, index[i]); 
					m_A.set(j, index[i], v + weight[i]);
				}
			}
		}
		for(size_t i = 0; i < m_out_size; ++i) {
			CSplineKernel::VWeight weight(m_support); 
			CSplineKernel::VIndex index(m_support); 
			kernel(i, weight, index); 
			m_bc->apply(index, weight); 
			m_weights.push_back(weight); 
			m_indices.push_back(index); 
		}

		gsl_linalg_QR_decomp(m_A, m_tau); 
	}
}


C1DScalarFixed::C1DScalarFixed(const CSplineKernel& kernel, size_t in_size, double scale):
	m_in_size(in_size), 
	m_support(kernel.size()), 
	m_scale(scale), 
	m_poles(kernel.get_poles()),
	m_strategy(scs_unknown), 
	m_bc(produce_spline_boundary_condition("mirror")), 
	m_input_buffer(in_size, false)
{
	m_out_size = ceil((in_size-1)  * m_scale) + 1;
	m_output_buffer = gsl::DoubleVector(m_out_size, false); 
	initialize(kernel); 
}
	

size_t C1DScalarFixed::get_output_size() const 
{
	return m_out_size; 
}
 
void C1DScalarFixed::operator () (const gsl::DoubleVector& input, gsl::DoubleVector& output) const
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

gsl::DoubleVector::iterator C1DScalarFixed::input_begin()
{
	return m_input_buffer.begin(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::input_end() 
{
	return m_input_buffer.end(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::output_begin()
{
	return m_output_buffer.begin(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::output_end() 
{
	return m_output_buffer.end();
}



void C1DScalarFixed::run()
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

void C1DScalarFixed::upscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const
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

void C1DScalarFixed::downscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const
{
	gsl::DoubleVector coefs(output.size(), false); 
	gsl::DoubleVector residual(input.size(), false); 
	gsl_linalg_QR_lssolve (m_A, m_tau, input, coefs, residual); 
	
	for (size_t i = 0; i < output.size(); ++i) {
		const auto& weight = m_weights[i]; 
		const auto& index = m_indices[i]; 
		double v = 0.0;
		for (size_t k = 0; k < m_support; ++k)
			v += weight[k] * coefs[index[k]]; 
		output[i] = output.size()  * v; 
	}
}

NS_MIA_END
