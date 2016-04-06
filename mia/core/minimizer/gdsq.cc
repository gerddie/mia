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

#include <mia/core/minimizer/gdsq.hh>
#include <gsl/gsl_cblas.h>


NS_BEGIN(minimizer_gdsq)
using namespace mia; 
using namespace std; 


CGDSQMinimizer::CGDSQMinimizer(double start_step, double step_scale, 
			       double xtol, double gtol, double ftolr, 
			       unsigned int maxiter, double min_step):
	
	m_xtol( xtol ), 
	m_gtol( gtol ), 
	m_maxiter( maxiter ), 
	m_step( start_step ),
	m_step_scale(step_scale), 
	m_ftolr(ftolr),
	m_min_step(min_step)
{
	add(property_gradient); 
}

	
void CGDSQMinimizer::do_set_problem()
{
}

int CGDSQMinimizer::test_tol(CDoubleVector& dx, double tol, int cause)const
{
	for (auto ix = dx.begin(); ix != dx.end(); ++ix) {
		if (fabs(*ix) >=tol)
			return 0; 
	}
	return cause; 
}

int CGDSQMinimizer::do_run(CDoubleVector& x)
{
	TRACE_FUNCTION; 
	CDoubleVector g(x.size()); 
	CDoubleVector gt(x.size()); 
	CDoubleVector xwork(x.size()); 
	copy(x.begin(), x.end(), xwork.begin()); 
	
	double f_init = get_problem().fdf(xwork, g);
	double step = m_step; 
	
	double f_old = f_init; 
	unsigned int iter = 0; 
	int success = test_tol(g, m_xtol, SUCCESS_XTOLA);
	while (iter++ < m_maxiter && step > m_min_step && !success) {
		cblas_daxpy(g.size(), -step, &g[0], 1, &xwork[0], 1);
		double f = get_problem().fdf(xwork, gt);

		if (f < f_old) {
			cblas_dscal(g.size(), -step, &g[0], 1);
			double sum = cblas_ddot(g.size(), &g[0], 1, &gt[0], 1); 
			double aux = f_old - f + sum;
			if (aux <= 0.0) 
				step *= m_step_scale;
			else {
				step = 0.5 * cblas_dnrm2(g.size(), &g[0], 1) / aux; 
			}
			copy(xwork.begin(), xwork.end(), x.begin()); 
			copy(gt.begin(), gt.end(), g.begin()); 
			double frel = (f_old - f); 
			f_old = f; 

			success |= test_tol(gt, m_gtol, SUCCESS_GTOLA);

		
			if (frel < m_ftolr)
				success |= SUCCESS_FTOLR;  
			else
				cvinfo() << "improovement = " << frel << "\n"; 
			
		}else{
			// recover best solution 
			copy(x.begin(), x.end(), xwork.begin() ); 
			step /= m_step_scale;
		}
		success |= test_tol(g, m_xtol / step, SUCCESS_XTOLA); 
		cvinfo() << "[" << iter << "]: f=" << f << " step=" << step << "\n"; 
	}
	if (iter == m_maxiter) 
		cvwarn() << "Iteration stopped because maximum number of iterations was reached\n"; 
	
	if (success & SUCCESS_XTOLA) 
		cvmsg() << "Stop: XTOLA\n"; 

	if (success & SUCCESS_GTOLA) 
		cvmsg() << "Stop: GTOLA\n"; 
		
	if (success & SUCCESS_FTOLR) 
		cvmsg() << "Stop: FTOLR\n"; 

	cvmsg() << "Stop:" << success << " with " << iter << " of " << m_maxiter << "iterations\n"; 

	return CMinimizer::success; 
}

CGDSQMinimizerPlugin::CGDSQMinimizerPlugin():
	CMinimizerPlugin("gdsq"), 
	m_xtol(0.0), 
	m_gtol(0.0), 
	m_ftolr(0.0), 
	m_maxiter(100), 
	m_start_step(0.1), 
	m_step_scale(2.0), 
	m_min_step(1e-6)
{
	add_parameter("maxiter", new CUIBoundedParameter(m_maxiter, EParameterBounds::bf_min_closed, {1}, false,
							 "Stopping criterion: the maximum number of iterations")); 
	
	add_parameter("step", new CDBoundedParameter(m_start_step, EParameterBounds::bf_min_open, {0.0f},
						     false, "Initial step size"));
	
	add_parameter("scale", new CDBoundedParameter(m_step_scale, EParameterBounds::bf_min_open, {1.0f},
						      false, "Fallback fixed step size scaling"));
	
	add_parameter("xtola", new CDBoundedParameter(m_xtol, EParameterBounds::bf_min_closed, {0.0}, false, 
						      "Stop if the inf-norm of x-update is below this value.")); 
	add_parameter("gtola", new CDBoundedParameter(m_gtol, EParameterBounds::bf_min_closed, {0.0}, false, 
						      "Stop if the inf-norm of the gradient is below this value."));
	add_parameter("ftolr", new CDBoundedParameter(m_ftolr, EParameterBounds::bf_min_closed, {0.0}, false, 
						      "Stop if the relative change of the criterion is below."));
}

	

CMinimizer *CGDSQMinimizerPlugin::do_create() const
{
	TRACE_FUNCTION; 
	return new CGDSQMinimizer(m_start_step, m_step_scale, m_xtol, m_gtol, m_ftolr, m_maxiter, m_min_step); 
}

const std::string CGDSQMinimizerPlugin::do_get_descr() const
{
	return "Gradient descent with quadratic step estimation"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CGDSQMinimizerPlugin();
}




NS_END
