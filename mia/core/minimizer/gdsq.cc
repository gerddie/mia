/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/core/minimizer/gdsq.hh>
extern "C" {
#include <cblas.h>
}

NS_BEGIN(minimizer_gdsq)
using namespace mia; 

CGDSQMinimizer::CGDSQMinimizer(double start_step, double step_scale, 
			       double xtol, double gtol, unsigned int maxiter):
	
	m_xtol( xtol ), 
	m_gtol( gtol ), 
	m_maxiter( maxiter ), 
	m_step( start_step ),
	m_step_scale(step_scale)
{
	add(property_gradient); 
}

	
void CGDSQMinimizer::do_set_problem()
{
}

int CGDSQMinimizer::test_tol(CDoubleVector& dx, double tol)const
{
	for (auto ix = dx.begin(); ix != dx.end(); ++ix) {
		if (fabs(*ix) >=tol)
			return false; 
	}
	return true; 
}

int CGDSQMinimizer::do_run(CDoubleVector& x)
{
	TRACE_FUNCTION; 
	CDoubleVector g(x.size()); 
	CDoubleVector gt(x.size()); 
	CDoubleVector xwork(x.size()); 
	copy(x.begin(), x.end(), xwork.begin()); 
	
	double f_init = get_problem().fdf(xwork, g);
	
	double f_old = f_init; 
	int iter = 0; 
	bool success = false;
	while (iter++ < m_maxiter && !success) {

		cblas_daxpy(g.size(), -m_step, &g[0], 1, &xwork[0], 1);
		double f = get_problem().fdf(xwork, gt);

		if (f < f_old) {
			cblas_dscal(g.size(), -m_step, &g[0], 1);
			double sum = cblas_ddot(g.size(), &g[0], 1, &gt[0], 1); 
			double aux = f_old - f + sum;
			if (aux <= 0.0) 
				m_step *= m_step_scale;
			else {
				m_step = 0.5 * cblas_dnrm2(g.size(), &g[0], 1) / aux; 
			}
			copy(xwork.begin(), xwork.end(), x.begin()); 
			copy(gt.begin(), gt.end(), g.begin()); 
			f_old = f; 

			success |= test_tol(g, m_xtol); 
			success |= test_tol(gt, m_gtol);
	
		}else{
			// recover best solution 
			copy(x.begin(), x.end(), xwork.begin() ); 
			m_step /= m_step_scale;
		}
		cvmsg() << "[" << iter << "]: f=" << f << " step=" << m_step << "\n"; 
	}
	if (iter == m_maxiter) 
		cvwarn() << "Iteration stopped because maximum number of iterations was reached\n"; 
		
	return CMinimizer::success; 
}

CGDSQMinimizerPlugin::CGDSQMinimizerPlugin():
	CMinimizerPlugin("gdsq"), 
	m_xtol(0.0), 
	m_gtol(0.0), 
	m_maxiter(100), 
	m_start_step(0.1), 
	m_step_scale(2.0)
{
	add_parameter("maxiter", new CUIntParameter(m_maxiter, 1, numeric_limits<int>::max(), false, 
						   "Stopping criterion: the maximum number of iterations")); 
	
	add_parameter("step", new CDoubleParameter(m_start_step, 0.0, HUGE_VAL, false, "Initial step size")); 
	add_parameter("scale", new CDoubleParameter(m_step_scale, 1.0, HUGE_VAL, false, "Fallback fixed step size scaling")); 
	add_parameter("xtola", new CDoubleParameter(m_xtol, 0.0, HUGE_VAL, false, 
						    "Stop if the inf-norm of x-update is below this value.")); 
	add_parameter("gtola", new CDoubleParameter(m_gtol, 0.0, HUGE_VAL, false, 
						    "Stop if the inf-norm of the gradient is below this value."));
}

	

CGDSQMinimizerPlugin::ProductPtr CGDSQMinimizerPlugin::do_create() const
{
	return ProductPtr(new CGDSQMinimizer(m_start_step, m_step_scale, m_xtol, m_gtol, m_maxiter)); 
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
