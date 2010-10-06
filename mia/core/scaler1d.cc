/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
	_M_in_size(in_size), 
	_M_out_size(out_size), 
	_M_support(kernel.size()), 
	_M_poles(kernel.get_poles()),
	_M_strategy(scs_unknown), 
	_M_input_buffer(in_size), 
	_M_output_buffer(out_size)
{
	assert(in_size); 
	assert(out_size); 

	if (in_size < out_size) {
		// prepare for upscaling 
		if (in_size == 1) {
			_M_strategy = scs_fill_output; 
		} else {
			_M_strategy = scs_upscale; 
			const double dx = double(in_size - 1) / (out_size-1); 
			double x = 0; 
			for(size_t i = 0; i < out_size; ++i, x+= dx) {
				std::vector<double> weight(_M_support); 
				std::vector<int> index(_M_support); 
				kernel(x, weight, index); 
				mirror_boundary_conditions(index, in_size, 2*in_size - 2); 
				_M_weights.push_back(weight); 
				_M_indices.push_back(index); 
			}
		}
	} else if (in_size == out_size){
		_M_strategy = scs_copy; 
	} else {
		_M_strategy = scs_downscale; 
		// prepare for downscaling 
		const double dx = double(out_size-1) / (in_size-1); 
		std::vector<double> weight(_M_support); 
		std::vector<int> index(_M_support); 

		_M_A = gsl::Matrix(in_size, out_size,  true);
		_M_tau = gsl::DoubleVector(out_size); 
		for (size_t k = 0; k < out_size; ++k) {
			double x = 0; 
			for (size_t j = 0; j < in_size; ++j, x+=dx) {
				kernel(x, weight, index);
				mirror_boundary_conditions(index, out_size, 2*out_size - 2); 
				for (size_t i = 0; i < _M_support; ++i) {
					double v = _M_A(j, index[i]); 
					_M_A.set(j, index[i], v + weight[i]);
				}
			}
		}
		for(size_t i = 0; i < out_size; ++i) {
			std::vector<double> weight(_M_support); 
			std::vector<int> index(_M_support); 
			kernel(i, weight, index); 
			mirror_boundary_conditions(index, out_size, 2*out_size - 2); 
			_M_weights.push_back(weight); 
			_M_indices.push_back(index); 
		}

		gsl_linalg_QR_decomp(_M_A, _M_tau); 
	}
}
 
void C1DScalarFixed::operator () (const gsl::DoubleVector& input, gsl::DoubleVector& output) const
{
	switch (_M_strategy) {
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
	return _M_input_buffer.begin(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::input_end() 
{
	return _M_input_buffer.end(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::output_begin()
{
	return _M_output_buffer.begin(); 
}

gsl::DoubleVector::iterator C1DScalarFixed::output_end() 
{
	return _M_output_buffer.end();
}



void C1DScalarFixed::run()
{
	switch (_M_strategy) {
	case scs_fill_output: fill(output_begin(), output_end(), _M_input_buffer[0]); 
		break; 
	case scs_upscale: upscale(_M_input_buffer, _M_output_buffer); 
		break; 
	case scs_copy: copy(_M_input_buffer.begin(), _M_input_buffer.end(), 
			    _M_output_buffer.begin()); 
		break; 
	case scs_downscale: 
		downscale(_M_input_buffer, _M_output_buffer);
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
	for (size_t k = 0; k < _M_poles.size() ; ++k) {
		lambda  *=  2 - _M_poles[k] - 1.0 / _M_poles[k];
	}
	
	/* apply the gain */
	transform(coeff.begin(), coeff.end(), result.begin(), _1 * lambda); 
	
	/* loop over all _M_poles */
	for (size_t k = 0; k < _M_poles.size(); ++k) {
		/* causal initialization */
		result[0] = initial_coeff(result, _M_poles[k]);
		
		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			result[n] += _M_poles[k] * result[n - 1];
		}
		
		/* anticausal initialization */
		result[coeff.size() - 1] = initial_anti_coeff(result, _M_poles[k]);
		/* anticausal recursion */
		for (int n = result.size() - 2; 0 <= n; n--) {
			result[n] = _M_poles[k] * (result[n + 1] - result[n]);
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
		const vector<double>& weight = _M_weights[i]; 
		const vector<int>& index = _M_indices[i]; 
		double v = 0.0; 
		for (size_t k = 0; k < _M_support; ++k)
			v += weight[k] * coefs[index[k]]; 
		*io = v; 
	}
}

void C1DScalarFixed::downscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const
{
	gsl::DoubleVector coefs(output.size()); 
	gsl::DoubleVector residual(input.size()); 
	gsl_linalg_QR_lssolve (_M_A, _M_tau, input, coefs, residual); 
	
	for (size_t i = 0; i < output.size(); ++i) {
		const vector<double>& weight = _M_weights[i]; 
		const vector<int>& index = _M_indices[i]; 
		double v = 0.0;
		for (size_t k = 0; k < _M_support; ++k)
			v += weight[k] * coefs[index[k]]; 
		output[i] = output.size()  * v; 
	}
}

NS_MIA_END
