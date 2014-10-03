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


#include <mia/core/fastica/deflationnonlinearity.hh>
#include <gsl/gsl_blas.h>
#include <gsl++/matrix_vector_ops.hh>
#include <algorithm> 

NS_BEGIN(fastica_deflnonlin)

using namespace std; 
using namespace mia; 


void CFastICADeflPow3::do_apply(gsl::DoubleVector& w) 
{

	transform(get_XTw().begin(), get_XTw().end(), get_XTw().begin(), [](double x) -> double {
			return x*x*x;
		}); 

	multiply_m_v(get_workspace(), get_signal(), get_XTw());

	cblas_daxpy(get_workspace().size(), -3.0 * get_signal().cols(), w->data, w->stride, 
		    get_workspace()->data, get_workspace()->stride);

        double inv_m = 1.0 / get_signal().cols(); 
	transform(get_workspace().begin(), get_workspace().end(), w.begin(), 
		  [inv_m](double x) { return x * inv_m;}); 
}

void CFastICADeflPow3::do_apply_stabelized(gsl::DoubleVector& w)
{
        const double inv_m = 1.0 / get_signal().cols();
	transform(get_XTw().begin(), get_XTw().end(), get_XTw().begin(), [inv_m](double x) -> double {
			return x*x*x * inv_m;
		}); 


	multiply_m_v(get_workspace(), get_signal(), get_XTw());

	const double beta = multiply_v_v(w, get_workspace()); 

	cblas_daxpy(get_workspace().size(), -beta, w->data, w->stride, 
		    get_workspace()->data, get_workspace()->stride);

	
	cblas_daxpy(get_workspace().size(), - get_mu() / (3.0 - beta),
		    get_workspace()->data, get_workspace()->stride, 
		    w->data, w->stride);
}

CFastICADeflTanh::CFastICADeflTanh(double a):m_a(a)
{
}

void CFastICADeflTanh::do_apply(gsl::DoubleVector& w) 
{
        double inv_m = 1.0 / get_signal().cols(); 
	
	transform(get_XTw().begin(), get_XTw().end(), get_XTw().begin(), 
		  [this](double x) {
			  return tanh(m_a * x);
		  }); 


	multiply_m_v(get_workspace(), get_signal(), get_XTw());


	double scale = 0.0; 
	for_each(get_XTw().begin(), get_XTw().end(), [this, &scale](double x) {
			scale += 1 - x*x;
		}); 
	
	cblas_daxpy(get_workspace().size(), -m_a * scale, w->data, w->stride,
		    get_workspace()->data, get_workspace()->stride); 

	transform(get_workspace().begin(), get_workspace().end(), w.begin(), 
		  [inv_m](double x) { return x * inv_m;}); 

}

void CFastICADeflTanh::do_apply_stabelized(gsl::DoubleVector& w)
{
	transform(get_XTw().begin(), get_XTw().end(), get_XTw().begin(), 
		  [this](double x) {
			  return tanh(m_a * x);
		  }); 

	multiply_m_v(get_workspace(), get_signal(), get_XTw());

	const double beta = multiply_v_v(w, get_workspace());

	double scale = 0.0; 
	for_each(get_XTw().begin(), get_XTw().end(), [this, &scale](double x) {
			scale += 1 - x*x;
		}); 

	cblas_daxpy(get_workspace().size(), -beta, w->data, w->stride,
		    get_workspace()->data, get_workspace()->stride); 


	cblas_daxpy(get_workspace().size(), -get_mu() / ( m_a  * scale - beta), 
		    get_workspace()->data, get_workspace()->stride, 
		    w->data, w->stride); 
}


CFastICADeflGauss::CFastICADeflGauss(double a):m_a(a)
{
}

void CFastICADeflGauss::do_apply(gsl::DoubleVector& w)
{
	transform(get_XTw().begin(), get_XTw().end(), m_usquared.begin(), 
		  [](double x) {return x * x; }); 

	transform(m_usquared.begin(), m_usquared.end(), m_ex.begin(),
		  [this](double x) { return exp(- m_a * x / 2.0);}); 
	
        transform(get_XTw().begin(), get_XTw().end(), m_ex.begin(), get_XTw().begin(),
			  [](double u, double expu2) {return u * expu2;}); 
	
	multiply_m_v(get_workspace(), get_signal(), get_XTw());
	

	transform(m_usquared.begin(), m_usquared.end(), m_ex.begin(), m_ex.begin(),
		  [this](double u2, double expu2) { return (1 - m_a * u2 ) * expu2;});
	
	double scale = 0.0; 
	for_each(m_ex.begin(), m_ex.end(), [this, &scale](double x) {
			scale += x;
		}); 
	
	cblas_daxpy(w.size(), -scale, w->data, w->stride,
		    get_workspace()->data, get_workspace()->stride); 
	

        double inv_m = 1.0 / get_signal().cols(); 
        transform(get_workspace().begin(), get_workspace().end(), w.begin(), 
		  [inv_m](double x) { return x * inv_m;}); 

}

void CFastICADeflGauss::do_apply_stabelized(gsl::DoubleVector& w)
{
	transform(get_XTw().begin(), get_XTw().end(), m_usquared.begin(), 
		  [](double x) {return x * x; }); 

	transform(m_usquared.begin(), m_usquared.end(), m_ex.begin(),
		  [this](double x) { return exp(- m_a * x / 2.0);}); 
	
        transform(get_XTw().begin(), get_XTw().end(), m_ex.begin(), get_XTw().begin(),
			  [](double u, double expu2) {return u * expu2;}); 
	
	multiply_m_v(get_workspace(), get_signal(), get_XTw());
	

	transform(m_usquared.begin(), m_usquared.end(), m_ex.begin(), m_ex.begin(),
		  [this](double u2, double expu2) { return (1 - m_a * u2 ) * expu2;});
	
	double scale = 0.0; 
	for_each(m_ex.begin(), m_ex.end(), [this, &scale](double x) {
			scale += x;
		}); 
	
	const double beta = multiply_v_v(w,  get_workspace()); 
	cblas_daxpy(w.size(), -beta, w->data, w->stride,
		    get_workspace()->data, get_workspace()->stride); 
	
	cblas_daxpy(w.size(), -get_mu() / (scale - beta), 
		    get_workspace()->data, get_workspace()->stride, 
		    w->data, w->stride); 

}


void CFastICADeflGauss::post_set_signal()
{
        m_usquared = gsl::DoubleVector(get_signal().cols(), false); 
	m_ex = gsl::DoubleVector(get_signal().cols(), false); 
	CFastICADeflNonlinearity::post_set_signal();
}



CFastICADeflPow3Plugin::CFastICADeflPow3Plugin():
	CFastICADeflNonlinearityPlugin("pow3")
{
}

CFastICADeflNonlinearity *CFastICADeflPow3Plugin::do_create() const
{
       return new CFastICADeflPow3(); 
}

const std::string CFastICADeflPow3Plugin::do_get_descr()const
{
	return "Implementation of the simple pow3 non-linearity for the deflation "
		"based FastICA algorithm. It's use is justified on statistical grounds "
		"only for estimating sub-Gaussian independent components without outliers."; 
}

CFastICADeflTanhPlugin::CFastICADeflTanhPlugin():
	CFastICADeflNonlinearityPlugin("tanh")
{
	this->add_parameter("a", 
			    new CDoubleParameter(m_a, 1, 2, 
						 false, "Tuning parameter")); 
	
}

CFastICADeflNonlinearity *CFastICADeflTanhPlugin::do_create() const
{
	return new CFastICADeflTanh(m_a); 
}

const std::string CFastICADeflTanhPlugin::do_get_descr()const
{
	return "Implementation of the 'log cosh' non-linearity "
		"for the deflation based FastICA algorithm. This is a good general "
		"purpouse contrast function.";
}



CFastICADeflGaussPlugin::CFastICADeflGaussPlugin():
	CFastICADeflNonlinearityPlugin("gauss"), 
	m_a(1.0)
{
	this->add_parameter("a", 
			    new CDoubleParameter(m_a, 0, std::numeric_limits<float>::max(), 
						 false, 
						 "Tuning parameter: a in (0,2) for super-Gaussian"
						 "density, a > 2 for sub-Gaussian density. "
						 "a~1 is usually a good choice.")); 
}

CFastICADeflNonlinearity *CFastICADeflGaussPlugin::do_create() const
{
	return new CFastICADeflGauss(m_a); 
}

const std::string CFastICADeflGaussPlugin::do_get_descr()const
{
	return "Implementation of the Gauss non-linearity for the deflation based "
		"FastICA algorithm. Use it when the independend components are highly "
		"super-Gaussian or robustness is very important."; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	auto result = new CFastICADeflGaussPlugin(); 
	result->append_interface(new CFastICADeflTanhPlugin()); 
	result->append_interface(new CFastICADeflPow3Plugin()); 
	return result;
}


NS_END
