/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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



/* 
   LatexBeginPluginDescription{Minimizers}
   
   \subsection{Minimizers provided by the NLOpt Library}
   \label{minimizer:nlopt}
   
   \begin{description}
   
   \item [Plugin:] nlopt
   \item [Description:] This plugin provides minimizers that are implemented in the NLopt Library
                        \cite{johnson11}. 
			A wide range of optimizers is supported - for details please see the original source.  
			The naming scheme for the algoritm identifier is xy-name with 
			\begin {itemize}
			\item x g=global, l=local
			\item y n=no derivative used, d=derivative used
			\item name the name of the algorithm including further specifications. 
			\end {itemize}
   \plugtabstart
   opt & string & main minimization algorithm &  not given\\
   local-opt & string & secondary minimization algorithm (some of the main algorithms need this, 
                       all parameters that are used for the main algorithm are currently also 
		       used for the local one &  not given\\
   stop & double & Stopping criterion: function value falls below this value & -HUGE \\
   xtola & double & Stopping criterion: the absolute change of all x-values is below this value & 0.0 \\
   xtolr & double & Stopping criterion: the relative change of all x-values is below this value & 0.0 \\
   ftola & double & Stopping criterion: the relative change of the objective value is below  this value & 0.0 \\
   ftolr & double & Stopping criterion: the relative change of all objective value  is below this value & 0.0 \\
   maxiter & int & Stopping criterion: the maximum number of iterations & 100 \\
   step & double & Initial step size & 0.0 \\
   \plugtabend
   
   \item [Remark:] The default values for all but the maxiter the stopping criterions correspond to 
                   "not used". Usually it is only required to set some of the stopping criterions. 
   \end{description}

   LatexEnd  
 */


#include <mia/core/minimizer/nlopt.hh>
#include <stdexcept>

NS_BEGIN(nlopt)

using namespace std; 
using namespace mia; 

const TDictMap<nlopt_algorithm>::Table optimizer_table[NLOPT_NUM_ALGORITHMS+1] = {
	{"gn-direct",                NLOPT_GN_DIRECT}, 
	{"gn-direct-l",              NLOPT_GN_DIRECT_L},
	{"gn-direct-l-rand",         NLOPT_GN_DIRECT_L_RAND},
	{"gn-direct-noscal",         NLOPT_GN_DIRECT_NOSCAL},
	{"gn-direct-l-noscal",       NLOPT_GN_DIRECT_L_NOSCAL},
	{"gn-direct-l-rand-noscale", NLOPT_GN_DIRECT_L_RAND_NOSCAL},
	
	{"gn-orig-direct",           NLOPT_GN_ORIG_DIRECT},
	{"gn-orig-direct-l",         NLOPT_GN_ORIG_DIRECT_L},
	
	{"gd-stogo",                 NLOPT_GD_STOGO},
	{"gd-stogo-rand",            NLOPT_GD_STOGO_RAND},
	
	{"ld-lbfgs-nocedal",         NLOPT_LD_LBFGS_NOCEDAL},
	
	{"ld-lbfgs",                 NLOPT_LD_LBFGS},
	
	{"ln-praxis",                NLOPT_LN_PRAXIS},
	
	{"ld-var1",                  NLOPT_LD_VAR1},
	{"ld-var2",                  NLOPT_LD_VAR2},
	
	{"ld-tnewton",               NLOPT_LD_TNEWTON},
	{"ld-tnewton-restart",       NLOPT_LD_TNEWTON_RESTART},
	{"ld-tnewton-precond",       NLOPT_LD_TNEWTON_PRECOND},
	{"ld-tnewton-precond-restart", NLOPT_LD_TNEWTON_PRECOND_RESTART},
	
	{"gn-crs2-lm",               NLOPT_GN_CRS2_LM},
	
	{"gn-mlsl",                  NLOPT_GN_MLSL},
	{"gd-mlsl",                  NLOPT_GD_MLSL},
	{"gn-mlsl-lds",              NLOPT_GN_MLSL_LDS},
	{"gd-mlsl-lds",              NLOPT_GD_MLSL_LDS},
	
	{"ld-mma",                   NLOPT_LD_MMA},
	
	{"ln-cobyla",                NLOPT_LN_COBYLA},
	
	{"ln-newuoa", NLOPT_LN_NEWUOA},
	{"ln-newuoa-bound", NLOPT_LN_NEWUOA_BOUND},
	
	{"ln-neldermead", NLOPT_LN_NELDERMEAD},
	{"ln-sbplx", NLOPT_LN_SBPLX},
	
	{"ln-auglag", NLOPT_LN_AUGLAG},
	{"ld-auglag", NLOPT_LD_AUGLAG},
	{"ln-auglag-eq", NLOPT_LN_AUGLAG_EQ},
	{"ld-auglag-eq", NLOPT_LD_AUGLAG_EQ},
	
	{"ln-bobyqa", NLOPT_LN_BOBYQA},
	
	{"gn-isres", NLOPT_GN_ISRES},
	
	/* new variants that require local_optimizer to be set},
	   not with older constants for backwards compatibility */
	{"auglag", NLOPT_AUGLAG},
	{"auglag-eq", NLOPT_AUGLAG_EQ},
	{"g-mlsl", NLOPT_G_MLSL},
	{"g-mlsl-lds", NLOPT_G_MLSL_LDS},
	
	{"ld-slsqp", NLOPT_LD_SLSQP},
	
	{NULL, NLOPT_NUM_ALGORITHMS}, 
}; 

const nlopt_algorithm g_require_gradient[] = {
	NLOPT_GD_STOGO, 
	NLOPT_GD_STOGO_RAND,
	NLOPT_LD_LBFGS_NOCEDAL,
	NLOPT_LD_LBFGS,
	NLOPT_LD_VAR1,
	NLOPT_LD_VAR2,
	NLOPT_LD_TNEWTON,
	NLOPT_LD_TNEWTON_RESTART,
	NLOPT_LD_TNEWTON_PRECOND,
	NLOPT_LD_TNEWTON_PRECOND_RESTART,
	NLOPT_GD_MLSL,
	NLOPT_GD_MLSL_LDS,
	NLOPT_LD_MMA,
	NLOPT_LD_AUGLAG,
	NLOPT_LD_AUGLAG_EQ,
	NLOPT_G_MLSL,
	NLOPT_G_MLSL_LDS,
	NLOPT_LD_SLSQP,
	NLOPT_NUM_ALGORITHMS}; 
	


SOpt::SOpt():
	algo(NLOPT_NUM_ALGORITHMS), 
	local_opt(NLOPT_NUM_ALGORITHMS), 
	stopval(-HUGE_VAL), 
	abs_xtol(0.0), 
	rel_xtol( 0.0), 
	rel_ftol(0.0),  
	abs_ftol(0.0), 
	step(0),
	maxiter(100)
{
}

CNLOptFDFMinimizer::CNLOptFDFMinimizer(const SOpt& options):
	m_opt(NULL),
	m_options(options)
{
	if (require_gradient(m_options.algo)  || 
	    require_gradient(m_options.local_opt)) 
		add(property_gradient); 
}

bool CNLOptFDFMinimizer::require_gradient(nlopt_algorithm algo) const
{
	bool result = false; 
	const nlopt_algorithm * meth = g_require_gradient; 
	while (!result && *meth != NLOPT_NUM_ALGORITHMS) 
		result = (algo == *meth++); 
	return result; 
}

CNLOptFDFMinimizer::~CNLOptFDFMinimizer()
{
	if (m_opt) 
		nlopt_destroy(m_opt);
}

double CNLOptFDFMinimizer::fdf(unsigned n, const double *x, double *grad, void *problem)
{
	assert(problem); 
	assert(x); 
	
	CMinimizer::Problem *o = reinterpret_cast<CMinimizer::Problem *>(problem); 
	assert(n == o->size()); 

	if (grad)
		return o->fdf(n, x, grad); 
	else 
		return o->f(n, x); 
}
	
	
void CNLOptFDFMinimizer::do_set_problem()
{
	

	if (m_opt) 
		nlopt_destroy(m_opt);
	
	m_opt = nlopt_create(m_options.algo, size()); /* algorithm and dimensionality */
	if (!m_opt) 
		throw runtime_error("CNLOptFDFMinimizer: unable to create minimizer"); 
	
	nlopt_set_min_objective(m_opt, CNLOptFDFMinimizer::fdf, (void *)get_problem_pointer());
	
	if (m_options.local_opt != NLOPT_NUM_ALGORITHMS) {
		nlopt_opt local_opt  = nlopt_create(m_options.algo, size()); /* algorithm and dimensionality */

		nlopt_set_xtol_rel(local_opt, m_options.rel_xtol);
		nlopt_set_ftol_rel(local_opt, m_options.rel_ftol);
		
		nlopt_set_xtol_abs1(local_opt, m_options.abs_xtol);
		nlopt_set_ftol_abs(local_opt, m_options.abs_ftol);
		
		nlopt_set_stopval(local_opt, m_options.stopval);
		nlopt_set_maxeval(local_opt, m_options.maxiter);
		
		if (m_options.step > 0.0)
			nlopt_set_initial_step1(local_opt, m_options.step);
		
			
		nlopt_set_local_optimizer(m_opt, local_opt); 
		nlopt_destroy(local_opt); 


	}
	
	nlopt_set_xtol_rel(m_opt, m_options.rel_xtol);
	nlopt_set_ftol_rel(m_opt, m_options.rel_ftol);

	nlopt_set_xtol_abs1(m_opt, m_options.abs_xtol);
	nlopt_set_ftol_abs(m_opt, m_options.abs_ftol);
	
	nlopt_set_stopval(m_opt, m_options.stopval);
	nlopt_set_maxeval(m_opt, m_options.maxiter);
	
	if (m_options.step > 0.0)
		nlopt_set_initial_step1(m_opt, m_options.step);
}

int CNLOptFDFMinimizer::do_run(CDoubleVector& x)
{
	double minf = 0.0; 
	
	cvinfo() << "Starting optimization with '" << nlopt_algorithm_name(m_options.algo) << "'\n"; 
	auto retval = nlopt_optimize(m_opt, &x[0], &minf); 
	cvinfo() << "\n"; 
	if (retval < 0) {
		switch (retval) {
		case NLOPT_FAILURE: cvinfo() << "CNLOptFDFMinimizer: optimization failed for ungiven reasons\n"; 
			// okay, it's a lie
			return CMinimizer::success; 
		case NLOPT_INVALID_ARGS: throw invalid_argument("CNLOptFDFMinimizer: invalid arguments given");
		case NLOPT_OUT_OF_MEMORY: throw runtime_error("CNLOptFDFMinimizer: out of memory"); 
		case NLOPT_FORCED_STOP: throw runtime_error("CNLOptFDFMinimizer: optimization was forced to stop"); 
		case NLOPT_ROUNDOFF_LIMITED:  
			cvinfo() << "CNLOptFDFMinimizer: optimization stopped due to roundoff errors\n"; 
			return CMinimizer::success; 
		default: 
			throw runtime_error("CNLOptFDFMinimizer: optimization failed with unknown error code"); 
		}
	}else {
		switch (retval) {
		case NLOPT_SUCCESS: cvinfo() << "SUCCESS\n"; 
			break; 
		case NLOPT_STOPVAL_REACHED: cvinfo() << "SUCCESS: objective value below stopvalue.\n"; 
			break; 
		case NLOPT_FTOL_REACHED:cvinfo() << "SUCCESS: objective value changes below given tolerance.\n"; 
			break; 
		case NLOPT_XTOL_REACHED:cvinfo() << "SUCCESS: X changes below given tolerance.\n"; 
			break; 
		case NLOPT_MAXEVAL_REACHED: cvinfo() << "maximum number of iterations reached.\n"; 
			break; 
		case NLOPT_MAXTIME_REACHED: cvinfo() << "maximum time exeeded.\n"; 
			break; 
		default: 
			cvmsg() << "NLOpt SUCCESS: with unknown return value.\n"; 
		}
		return CMinimizer::success; 
	}
}


typedef CDictParameter<nlopt_algorithm> CNLOptMinimizerParam; 
typedef TDictMap<nlopt_algorithm> CNLOptMinimizerDict;  

CNLOptMinimizerDict dict(optimizer_table); 


CNLOptMinimizerPlugin::CNLOptMinimizerPlugin():
	CMinimizerPlugin("nlopt")
{
	add_parameter("opt", new CNLOptMinimizerParam(m_options.algo, dict, "main minimization algorithm")); 
	add_parameter("local-opt", new CNLOptMinimizerParam(m_options.local_opt, dict, 
						  "local minimization algorithm that may be required for the"
						  " main minimization algorithm. Currently, it uses the same"
						  " stopping criteria as the main algorithm"));
	
	add_parameter("stop", new CDoubleParameter(m_options.stopval, -HUGE_VAL, HUGE_VAL, false, 
				  "Stopping criterion: function value falls below this value")); 
	add_parameter("xtola", new CDoubleParameter(m_options.abs_xtol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the absolute change of all x-values is below "
				  " this value")); 
	add_parameter("xtolr", new CDoubleParameter(m_options.rel_xtol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the relative change of all x-values is below "
				  " this value")); 

	add_parameter("ftola", new CDoubleParameter(m_options.abs_ftol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the relative change of the objective value is below "
				  " this value")); 
	add_parameter("ftolr", new CDoubleParameter(m_options.rel_ftol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the relative change of all objective value  is below "
				  " this value")); 

	add_parameter("maxiter", new CIntParameter(m_options.maxiter, 1, numeric_limits<int>::max(), false, 
						   "Stopping criterion: the maximum number of iterations")); 
	
	add_parameter("step", new CDoubleParameter(m_options.step, 0.0, HUGE_VAL, false, 
					 "Initial step size for gradient free methods")); 
}
	
CMinimizer *CNLOptMinimizerPlugin::do_create() const
{
	return new CNLOptFDFMinimizer(m_options); 
}

const std::string CNLOptMinimizerPlugin::do_get_descr() const
{
	return "Minimizer algorithms using the NLOPT library, for a "
		"description of the optimizers please see" 
		" 'http://ab-initio.mit.edu/wiki/index.php/NLopt_Algorithms'"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CNLOptMinimizerPlugin();
}

NS_END
