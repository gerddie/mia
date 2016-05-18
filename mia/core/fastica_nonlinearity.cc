/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/core/gsl_matrix_vector_ops.hh>
#include <mia/core/export_handler.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <algorithm>

namespace mia {

using std::transform; 
using std::vector; 
using namespace gsl; 
using std::ostream_iterator; 

CFastICANonlinearityBase::CFastICANonlinearityBase():
	m_mu(1.0),
	m_sample_scale(1.0), 
        m_signal(nullptr)
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
	m_workspace = Vector(get_signal().rows(), false); 
	m_XTw = Vector(get_signal().cols(), false); 
}

const char *CFastICANonlinearityBase::data_descr = "fastica"; 
const char *CFastICADeflNonlinearity::type_descr = "deflation"; 


void CFastICADeflNonlinearity::apply(gsl::Vector& w)
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
		gsl::Vector w(&wc.vector); 
		
		multiply_v_m(m_XTw, w, get_signal());
		const double scale = get_correction_and_scale(m_XTw, m_workspace); 

		if (get_mu() >= 1.0) 
			sum_final(w, scale);  
		else 
			sum_final_stabelized(w, scale); 
	}
}

vector<double> CFastICADeflNonlinearity::get_saddle_test_table(const gsl::Matrix& ics) const
{
	vector<double> result(ics.rows()); 
	for (unsigned i = 0; i < ics.rows(); ++i) {
		auto row = ics.get_row(i); 
		result[i] = do_get_saddle_test_value(row);
	}
	return result; 
}

double CFastICADeflNonlinearity::get_saddle_test_value(const gsl::Vector& ic) const
{
	return do_get_saddle_test_value(ic); 
}


void CFastICADeflNonlinearity::sum_final(gsl::Vector& w, double scale)
{
	const double inv_m = get_sample_scale(); 

	transform(m_workspace.begin(), m_workspace.end(), w.begin(), w.begin(), 
		  [scale, inv_m](double x, double y) { return (x - scale * y) * inv_m;}); 

}

void CFastICADeflNonlinearity::sum_final_stabelized(gsl::Vector& w, double scale)
{
	const double beta = dot(w, m_workspace); 
	const double a2 = get_mu() / (scale - beta); 
	const double a1 = 1 + beta * a2; 

	transform(m_workspace.begin(), m_workspace.end(), w.begin(), w.begin(), 
		  [a1, a2](double x, double y){return a1 * y - a2 * x;}); 

}

template<>  const char * const 
TPluginHandler<TFactory<CFastICADeflNonlinearity>>::m_help = 
	"These plug-ins provide various non-linearity models for the FastICA algorithm "
        "that uses deflation.";

EXPORT_CORE PFastICADeflNonlinearity produce_fastica_nonlinearity(const std::string& descr)
{
	return CFastICADeflNonlinearityPluginHandler::instance().produce(descr);
}

EXPLICIT_INSTANCE_HANDLER(CFastICADeflNonlinearity); 


}// end namespace 
