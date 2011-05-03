/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
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
#include <boost/lambda/lambda.hpp>
#include <mia/core/scaler1d.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

using boost::lambda::_1; 
using namespace std; 
C1DScalarFixed::C1DScalarFixed(const CBSplineKernel& kernel, size_t in_size, size_t out_size):
	m_in_size(in_size), 
	m_out_size(out_size), 
	m_support(kernel.size()), 
	m_poles(kernel.get_poles()),
	m_strategy(scs_unknown), 
	m_input_buffer(in_size), 
	m_output_buffer(out_size)
{
	assert(in_size); 
	assert(out_size); 

	if (in_size < out_size) {
		// prepare for upscaling 
		if (in_size == 1) {
			m_strategy = scs_fill_output; 
		} else {
			m_strategy = scs_upscale; 
			const double dx = double(in_size - 1) / (out_size-1); 
			double x = 0; 
			for(size_t i = 0; i < out_size; ++i, x+= dx) {
				std::vector<double> weight(m_support); 
				std::vector<int> index(m_support); 
				kernel(x, weight, index); 
				mirror_boundary_conditions(index, in_size, 2*in_size - 2); 
				m_weights.push_back(weight); 
				m_indices.push_back(index); 
			}
		}
	} else if (in_size == out_size){
		m_strategy = scs_copy; 
	} else {
		m_strategy = scs_downscale; 
		// prepare for downscaling 
		const double dx = double(out_size-1) / (in_size-1); 
		std::vector<double> weight(m_support); 
		std::vector<int> index(m_support); 

		m_A = gsl::Matrix(in_size, out_size,  true);
		m_tau = gsl::DoubleVector(out_size); 
		for (size_t k = 0; k < out_size; ++k) {
			double x = 0; 
			for (size_t j = 0; j < in_size; ++j, x+=dx) {
				kernel(x, weight, index);
				mirror_boundary_conditions(index, out_size, 2*out_size - 2); 
				for (size_t i = 0; i < m_support; ++i) {
					double v = m_A(j, index[i]); 
					m_A.set(j, index[i], v + weight[i]);
				}
			}
		}
		for(size_t i = 0; i < out_size; ++i) {
			std::vector<double> weight(m_support); 
			std::vector<int> index(m_support); 
			kernel(i, weight, index); 
			mirror_boundary_conditions(index, out_size, 2*out_size - 2); 
			m_weights.push_back(weight); 
			m_indices.push_back(index); 
		}

		gsl_linalg_QR_decomp(m_A, m_tau); 
	}
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


gsl::DoubleVector C1DScalarFixed::filter_line(const gsl::DoubleVector& coeff)const
{
	TRACE_FUNCTION; 
	/* special case required by mirror boundaries */
	if (coeff.size() < 2) {
		return coeff;
	}
	gsl::DoubleVector result(coeff.size()); 
	/* compute the overall gain */
	double	lambda = 1.0;
	for (size_t k = 0; k < m_poles.size() ; ++k) {
		lambda  *=  2 - m_poles[k] - 1.0 / m_poles[k];
	}
	
	/* apply the gain */
	transform(coeff.begin(), coeff.end(), result.begin(), _1 * lambda); 
	
	/* loop over all m_poles */
	for (size_t k = 0; k < m_poles.size(); ++k) {
		/* causal initialization */
		result[0] = initial_coeff(result, m_poles[k]);
		
		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			result[n] += m_poles[k] * result[n - 1];
		}
		
		/* anticausal initialization */
		result[coeff.size() - 1] = initial_anti_coeff(result, m_poles[k]);
		/* anticausal recursion */
		for (int n = result.size() - 2; 0 <= n; n--) {
			result[n] = m_poles[k] * (result[n + 1] - result[n]);
		}
	}
	return result; 
}

double C1DScalarFixed::initial_coeff(const gsl::DoubleVector& coeff, double pole)const
{
	TRACE_FUNCTION; 
	/* full loop */
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	double sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n <= coeff.size()  - 2L; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	return(sum / (1.0 - zn * zn));
}

double C1DScalarFixed::initial_anti_coeff(const gsl::DoubleVector& coeff, double pole)const
{
	TRACE_FUNCTION; 
	return((pole / (pole * pole - 1.0)) * 
	       (pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
}

void C1DScalarFixed::upscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const
{
	TRACE_FUNCTION; 
	auto io = output.begin(); 
	// the input should be filtered 
	
	gsl::DoubleVector coefs = filter_line(input); 

	for (size_t i = 0; i < output.size(); ++i, ++io) {
		const vector<double>& weight = m_weights[i]; 
		const vector<int>& index = m_indices[i]; 
		double v = 0.0; 
		for (size_t k = 0; k < m_support; ++k)
			v += weight[k] * coefs[index[k]]; 
		*io = v; 
	}
}

void C1DScalarFixed::downscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const
{
	gsl::DoubleVector coefs(output.size()); 
	gsl::DoubleVector residual(input.size()); 
	gsl_linalg_QR_lssolve (m_A, m_tau, input, coefs, residual); 
	
	for (size_t i = 0; i < output.size(); ++i) {
		const vector<double>& weight = m_weights[i]; 
		const vector<int>& index = m_indices[i]; 
		double v = 0.0;
		for (size_t k = 0; k < m_support; ++k)
			v += weight[k] * coefs[index[k]]; 
		output[i] = output.size()  * v; 
	}
}

NS_MIA_END
