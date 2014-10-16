/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/core/fastica_nonlinearity.hh>
#include <gsl++/matrix_vector_ops.hh>
#include <mia/core/export_handler.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <algorithm>

namespace mia {

using std::transform; 
using namespace gsl; 
using std::ostream_iterator; 

CFastICANonlinearityBase::CFastICANonlinearityBase():m_mu(1.0)
{
}

void CFastICANonlinearityBase::set_signal(const Matrix *signal)
{
	m_signal = signal; 
	assert(m_signal); 
	m_sample_scale = 1.0 / m_signal->cols(); 
	
	post_set_signal(); 
}

void CFastICANonlinearityBase::set_mu(double mu)
{
	m_mu = mu; 
}



const Matrix& CFastICANonlinearityBase::get_signal() const
{
	assert(m_signal); 
	return *m_signal; 
}

void CFastICADeflNonlinearity::post_set_signal()
{
	m_workspace = DoubleVector(get_signal().rows(), false); 
	m_XTw = DoubleVector(get_signal().cols(), false); 
}

const char *CFastICANonlinearityBase::data_descr = "fastica"; 
const char *CFastICADeflNonlinearity::type_descr = "deflation"; 
const char *CFastICASymmNonlinearity::type_descr = "symmetric"; 


void CFastICADeflNonlinearity::apply(gsl::DoubleVector& w)
{
	multiply_v_m(m_XTw, w, get_signal());
	const double scale = get_correction_and_scale(m_XTw, m_workspace); 

	if (get_mu() >= 1.0) 
		sum_final(w, scale);  
	else 
		sum_final_stabelized(w, scale); 
}

void CFastICADeflNonlinearity::apply(gsl::Matrix& W)
{
	for (unsigned c = 0; c < W.cols(); ++c) {
		auto wc = gsl_matrix_column(W, c); 
		gsl::DoubleVector w(&wc.vector); 
		
		cvdebug() << "IN: W(" << c << ")=["; 
		copy(w.begin(), w.end(), ostream_iterator<double>(cverb, ",")); 
		cverb << "]\n"; 
		
		multiply_v_m(m_XTw, w, get_signal());
		const double scale = get_correction_and_scale(m_XTw, m_workspace); 

		if (get_mu() >= 1.0) 
			sum_final(w, scale);  
		else 
			sum_final_stabelized(w, scale); 

		cvdebug() << "OUT: W(" << c << ")=["; 
		copy(w.begin(), w.end(), ostream_iterator<double>(cverb, ",")); 
		cverb << "]\n"; 

	}
}


void CFastICADeflNonlinearity::sum_final(gsl::DoubleVector& w, double scale)
{
	const double inv_m = get_sample_scale(); 

	transform(m_workspace.begin(), m_workspace.end(), w.begin(), w.begin(), 
		  [scale, inv_m](double x, double y) { return (x - scale * y) * inv_m;}); 

}

void CFastICADeflNonlinearity::sum_final_stabelized(gsl::DoubleVector& w, double scale)
{
	const double beta = dot(w, m_workspace); 
	const double a2 = get_mu() / (scale - beta); 
	const double a1 = 1 + beta * a2; 

	transform(m_workspace.begin(), m_workspace.end(), w.begin(), w.begin(), 
		  [a1, a2](double x, double y){return a1 * y - a2 * x;}); 

}


void CFastICASymmNonlinearity::post_set_signal()
{
        // create helper matrix 
}

template<>  const char * const 
TPluginHandler<TFactory<CFastICADeflNonlinearity>>::m_help = 
	"These plug-ins provide various non-linearity models for the FastICA algorithm "
        "that uses deflation.";

template<>  const char * const 
TPluginHandler<TFactory<CFastICASymmNonlinearity>>::m_help = 
	"These plug-ins provide various non-linearity models for the FastICA algorithm "
        "that uses symetric estimation.";


EXPORT_CORE PFastICADeflNonlinearity produce_fastica_nonlinearity(const std::string& descr)
{
	return CFastICADeflNonlinearityPluginHandler::instance().produce(descr);
}

EXPLICIT_INSTANCE_HANDLER(CFastICADeflNonlinearity); 
EXPLICIT_INSTANCE_HANDLER(CFastICASymmNonlinearity); 


}// end namespace 
