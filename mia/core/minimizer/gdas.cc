/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/minimizer/gdas.hh>
#include <mia/core/errormacro.hh>

#include <gsl/gsl_cblas.h>


NS_BEGIN(gdas)
using namespace mia; 
using namespace std; 

#define SUCCESS_XTOLA 1
#define SUCCESS_FTOLR 2

CGDSAMinimizer::CGDSAMinimizer(double min_step, double max_step, 
			       double xtol, double ftolr, 
			       unsigned int maxiter):
	m_min_step(min_step), 
	m_max_step(max_step), 
        m_ftolr(ftolr),
	m_xtol( xtol ), 
	m_maxiter( maxiter )
{
        add(property_gradient); 
        if (max_step <= min_step) {
                throw create_exception<invalid_argument>("CGDSAMinimizer max_step(", max_step,
                                       ") must be larger than min_step (", min_step,")"); 
        }
}

void CGDSAMinimizer::do_set_problem()
{
}

double CGDSAMinimizer::get_gradmax(CDoubleVector& x) 
{
        double gmax = 0.0; 
        for( auto ix = x.begin(); ix != x.end(); ++ix) {
                const double ax = fabs(*ix);
                if (ax > gmax) 
                        gmax = ax;
        }
        return gmax; 
}

int CGDSAMinimizer::do_run(CDoubleVector& x)
{
	TRACE_FUNCTION; 
	CDoubleVector g(x.size()); 
        CDoubleVector xwork(x.size()); 
        
	copy(x.begin(), x.end(), xwork.begin()); 
	
	double f_init = get_problem().fdf(xwork, g);
	double step = 0.5 * (m_max_step - m_min_step); 
	
	double f_old = f_init; 
	unsigned int iter = 0; 

        double gmax = get_gradmax(g); 
        int success = gmax < m_xtol ? SUCCESS_XTOLA : 0;
        int tries = 0; 
        
        while (iter++ < m_maxiter && !success) {
		cblas_daxpy(g.size(), -step / gmax, &g[0], 1, &xwork[0], 1);
                double f = get_problem().fdf(xwork, g);
                gmax = get_gradmax(g);
                if (gmax < m_xtol) 
                        success |= SUCCESS_XTOLA; 
                
                
		if (f < f_old) {
                        cvinfo() << "Successfull step: [" << iter << "]: f=" << f 
                                 << ", gmax = " << gmax << ", step=" << step << "\n"; 

                        tries = 0; 
                        copy(xwork.begin(), xwork.end(), x.begin()); 
                        
                        if ( (f < 0.5 * f_old) && (step < m_max_step)) {
                                step *= 1.5; 
                                if (step > m_max_step) 
                                        step = m_max_step; 
                                cvdebug() << "Increase step size to " << step << " \n";
                        }

                        double dfrel = (f_old - f)/f_old; 
                        cvdebug() << "dfrel=" << dfrel << "\n"; 
                        if (dfrel < m_ftolr) {
                                success |= SUCCESS_FTOLR; 
                                break;
                        }
                        f_old = f; 
                } else {
                        cvinfo() << "Failed step: [" << iter << "]: f=" << f 
                                 << ", gmax = " << gmax << ", step=" << step << "\n"; 
                        
                        if (step > m_min_step) {
				
                                // restore last solution if current value larger 
				if (f > f_old) 
					copy(x.begin(), x.end(), xwork.begin()); 
				
                                step /= 2.0; 
                                if (step < m_min_step)  
                                        step = m_min_step; 
                                cvdebug() << "Decrease step size to " << step << " \n";
                        }else{
                                if (tries < 5) 
                                        ++tries;
                                else {
                                        cvmsg() << "At minimal stepsize and no improvent, stopping\n"; 
                                        break; 
                                }
                        }
                }
	}
	if (iter == m_maxiter) 
		cvmsg() << "Iteration stopped because maximum number of iterations was reached\n"; 
	
	if (success & SUCCESS_XTOLA) 
		cvmsg() << "Stop: dx below given limit\n"; 
		
	if (success & SUCCESS_FTOLR) 
		cvmsg() << "Stop: relative cost funtion value decrease below limit "<< m_ftolr << ".\n"; 

	return CMinimizer::success; 
}

CGDSAMinimizerPlugin::CGDSAMinimizerPlugin():
	CMinimizerPlugin("gdas"), 
	m_min_step(0.1), 
	m_max_step(2.0), 
        m_ftolr(0.0),
	m_xtol( 0.01 ), 
	m_maxiter( 200 )
{
	add_parameter("maxiter", new CUIBoundedParameter(m_maxiter, EParameterBounds::bf_min_closed, {1}, false, 
							 "Stopping criterion: the maximum number of iterations")); 
	
	add_parameter("min-step", new CDBoundedParameter(m_min_step, EParameterBounds::bf_min_open, {0.0}, false,
							 "Minimal absolute step size"));
	
	add_parameter("max-step", new CDBoundedParameter(m_max_step, EParameterBounds::bf_min_open, {0.0}, false,
							 "Maximal absolute step size"));
	
	add_parameter("xtola", new CDBoundedParameter(m_xtol, EParameterBounds::bf_min_closed, {0.0}, false, 
						      "Stop if the inf-norm of the change applied to x is below this value."));
	
	add_parameter("ftolr", new CDBoundedParameter(m_ftolr, EParameterBounds::bf_min_closed, {0.0},false, 
						      "Stop if the relative change of the criterion is below."));
}


CMinimizer *CGDSAMinimizerPlugin::do_create() const
{
	TRACE_FUNCTION; 
	return new CGDSAMinimizer(m_min_step, m_max_step, m_xtol, m_ftolr, m_maxiter);
}

const std::string CGDSAMinimizerPlugin::do_get_descr() const
{
	return "Gradient descent with automatic step size correction."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CGDSAMinimizerPlugin();
}


NS_END
