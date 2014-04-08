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

#include <addons/nlopt/nlopt.hh>
#include <stdexcept>

NS_BEGIN(nlopt)

using namespace std; 
using namespace mia; 

const TDictMap<nlopt_algorithm>::Table optimizer_table[] = {
	{"gn-direct",                NLOPT_GN_DIRECT, "Dividing Rectangles"}, 
	{"gn-direct-l",              NLOPT_GN_DIRECT_L, "Dividing Rectangles (locally biased)"},
	{"gn-direct-l-rand",         NLOPT_GN_DIRECT_L_RAND, "Dividing Rectangles (locally biased, randomized)"},
	{"gn-direct-noscal",         NLOPT_GN_DIRECT_NOSCAL, "Dividing Rectangles (unscaled)"},
	{"gn-direct-l-noscal",       NLOPT_GN_DIRECT_L_NOSCAL, "Dividing Rectangles (unscaled, locally biased)"},
	{"gn-direct-l-rand-noscale", NLOPT_GN_DIRECT_L_RAND_NOSCAL, "Dividing Rectangles (unscaled, locally biased, randomized)"},
	
	{"gn-orig-direct",           NLOPT_GN_ORIG_DIRECT, "Dividing Rectangles (original implementation)"},
	{"gn-orig-direct-l",         NLOPT_GN_ORIG_DIRECT_L, "Dividing Rectangles (original implementation, locally biased)"},
	
	{"ld-lbfgs-nocedal",         NLOPT_LD_LBFGS_NOCEDAL, ""},
	
	{"ld-lbfgs",                 NLOPT_LD_LBFGS, "Low-storage BFGS"},
	
	{"ln-praxis",                NLOPT_LN_PRAXIS, "Gradient-free Local Optimization via the Principal-Axis Method"},
	
	{"ld-var1",                  NLOPT_LD_VAR1, "Shifted Limited-Memory Variable-Metric, Rank 1"},
	{"ld-var2",                  NLOPT_LD_VAR2, "Shifted Limited-Memory Variable-Metric, Rank 2"},
	
	{"ld-tnewton",               NLOPT_LD_TNEWTON, "Truncated Newton"},
	{"ld-tnewton-restart",       NLOPT_LD_TNEWTON_RESTART, "Truncated Newton with steepest-descent restarting"},
	{"ld-tnewton-precond",       NLOPT_LD_TNEWTON_PRECOND, "Preconditioned Truncated Newton"},
	{"ld-tnewton-precond-restart", NLOPT_LD_TNEWTON_PRECOND_RESTART, "Preconditioned Truncated Newton with steepest-descent restarting"},
	
	{"gn-crs2-lm",               NLOPT_GN_CRS2_LM, "Controlled Random Search with Local Mutation"},
	
	{"ld-mma",                   NLOPT_LD_MMA, "Method of Moving Asymptotes"},
	
	{"ln-cobyla",                NLOPT_LN_COBYLA, "Constrained Optimization BY Linear Approximation"},
	
	{"ln-newuoa", NLOPT_LN_NEWUOA, "Derivative-free Unconstrained Optimization by Iteratively Constructed Quadratic Approximation"},
	{"ln-newuoa-bound", NLOPT_LN_NEWUOA_BOUND, "Derivative-free Bound-constrained Optimization by Iteratively Constructed Quadratic Approximation"},
	
	{"ln-neldermead", NLOPT_LN_NELDERMEAD,  "Nelder-Mead simplex algorithm"},
	{"ln-sbplx", NLOPT_LN_SBPLX, "Subplex variant of Nelder-Mead"},
	
	{"ln-bobyqa", NLOPT_LN_BOBYQA, "Derivative-free Bound-constrained Optimization"},
	{"gn-isres", NLOPT_GN_ISRES, "Improved Stochastic Ranking Evolution Strategy"},
	
	/* new variants that require local_optimizer to be set},
	   not with older constants for backwards compatibility */
	{"auglag", NLOPT_AUGLAG, "Augmented Lagrangian algorithm"},
	{"auglag-eq", NLOPT_AUGLAG_EQ, "Augmented Lagrangian algorithm with equality constraints only"},
	{"g-mlsl", NLOPT_G_MLSL, "Multi-Level Single-Linkage (require local optimization and bounds)"},
	{"g-mlsl-lds", NLOPT_G_MLSL_LDS, "Multi-Level Single-Linkage (low-discrepancy-sequence, require local gradient based optimization and bounds)"},
	
	{"ld-slsqp", NLOPT_LD_SLSQP, "Sequential Least-Squares Quadratic Programming"},
	
	{NULL, NLOPT_NUM_ALGORITHMS, ""}, 
}; 


const TDictMap<nlopt_algorithm>::Table local_optimizer_table[] = {
	{"gn-direct",                NLOPT_GN_DIRECT, "Dividing Rectangles"}, 
	{"gn-direct-l",              NLOPT_GN_DIRECT_L, "Dividing Rectangles (locally biased)"},
	{"gn-direct-l-rand",         NLOPT_GN_DIRECT_L_RAND, "Dividing Rectangles (locally biased, randomized)"},
	{"gn-direct-noscal",         NLOPT_GN_DIRECT_NOSCAL, "Dividing Rectangles (unscaled)"},
	{"gn-direct-l-noscal",       NLOPT_GN_DIRECT_L_NOSCAL, "Dividing Rectangles (unscaled, locally biased)"},
	{"gn-direct-l-rand-noscale", NLOPT_GN_DIRECT_L_RAND_NOSCAL, "Dividing Rectangles (unscaled, locally biased, randomized)"},
	
	{"gn-orig-direct",           NLOPT_GN_ORIG_DIRECT, "Dividing Rectangles (original implementation)"},
	{"gn-orig-direct-l",         NLOPT_GN_ORIG_DIRECT_L, "Dividing Rectangles (original implementation, locally biased)"},
	
	{"ld-lbfgs-nocedal",         NLOPT_LD_LBFGS_NOCEDAL, ""},
	
	{"ld-lbfgs",                 NLOPT_LD_LBFGS, "Low-storage BFGS"},
	
	{"ln-praxis",                NLOPT_LN_PRAXIS, "Gradient-free Local Optimization via the Principal-Axis Method"},
	
	{"ld-var1",                  NLOPT_LD_VAR1, "Shifted Limited-Memory Variable-Metric, Rank 1"},
	{"ld-var2",                  NLOPT_LD_VAR2, "Shifted Limited-Memory Variable-Metric, Rank 2"},
	
	{"ld-tnewton",               NLOPT_LD_TNEWTON, "Truncated Newton"},
	{"ld-tnewton-restart",       NLOPT_LD_TNEWTON_RESTART, "Truncated Newton with steepest-descent restarting"},
	{"ld-tnewton-precond",       NLOPT_LD_TNEWTON_PRECOND, "Preconditioned Truncated Newton"},
	{"ld-tnewton-precond-restart", NLOPT_LD_TNEWTON_PRECOND_RESTART, "Preconditioned Truncated Newton with steepest-descent restarting"},
	
	{"gn-crs2-lm",               NLOPT_GN_CRS2_LM, "Controlled Random Search with Local Mutation"},
	
	{"ld-mma",                   NLOPT_LD_MMA, "Method of Moving Asymptotes"},
	
	{"ln-cobyla",                NLOPT_LN_COBYLA, "Constrained Optimization BY Linear Approximation"},
	
	{"ln-newuoa", NLOPT_LN_NEWUOA, "Derivative-free Unconstrained Optimization by Iteratively Constructed Quadratic Approximation"},
	{"ln-newuoa-bound", NLOPT_LN_NEWUOA_BOUND, "Derivative-free Bound-constrained Optimization by Iteratively Constructed Quadratic Approximation"},
	
	{"ln-neldermead", NLOPT_LN_NELDERMEAD,  "Nelder-Mead simplex algorithm"},
	{"ln-sbplx", NLOPT_LN_SBPLX, "Subplex variant of Nelder-Mead"},
	
	{"ln-bobyqa", NLOPT_LN_BOBYQA, "Derivative-free Bound-constrained Optimization"},
	{"gn-isres", NLOPT_GN_ISRES, "Improved Stochastic Ranking Evolution Strategy"},
	{"none", NLOPT_NUM_ALGORITHMS, "don't specify algorithm"}, 
	{NULL, NLOPT_NUM_ALGORITHMS, ""}, 
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
	algo(NLOPT_LD_LBFGS), 
	local_opt(NLOPT_NUM_ALGORITHMS), 
	stopval(-HUGE_VAL), 
	abs_xtol(0.0), 
	rel_xtol( 0.0), 
	rel_ftol(0.0),  
	abs_ftol(0.0), 
	step(0),
	min_boundary(-HUGE_VAL), 
	max_boundary(HUGE_VAL), 
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

	if (m_options.min_boundary != -HUGE_VAL) {
		cvinfo() << "Set lower boundary to " << m_options.min_boundary << "\n"; 
		nlopt_set_lower_bounds1(m_opt, m_options.min_boundary); 
	}

	if (m_options.max_boundary != HUGE_VAL) {
		cvinfo() << "Set higher boundary to " << m_options.max_boundary << "\n"; 
		nlopt_set_upper_bounds1(m_opt, m_options.max_boundary); 
	}
	
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
		case NLOPT_INVALID_ARGS: throw invalid_argument("CNLOptFDFMinimizer: invalid arguments given.");
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
CNLOptMinimizerDict local_dict(local_optimizer_table); 


CNLOptMinimizerPlugin::CNLOptMinimizerPlugin():
	CMinimizerPlugin("nlopt")
{
	add_parameter("opt", new CNLOptMinimizerParam(m_options.algo, dict, "main minimization algorithm")); 
	// todo this should be done by another plugin 
	add_parameter("local-opt", new CNLOptMinimizerParam(m_options.local_opt, local_dict, 
						  "local minimization algorithm that may be required for the"
						  " main minimization algorithm."));
	
	add_parameter("stop", new CDoubleParameter(m_options.stopval, -HUGE_VAL, HUGE_VAL, false, 
				  "Stopping criterion: function value falls below this value")); 
	add_parameter("xtola", new CDoubleParameter(m_options.abs_xtol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the absolute change of all x-values is below "
				  " this value")); 
	add_parameter("xtolr", new CDoubleParameter(m_options.rel_xtol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the relative change of all x-values is below "
				  " this value")); 

	add_parameter("ftola", new CDoubleParameter(m_options.abs_ftol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the absolute change of the objective value is below "
						    " this value")); 
	add_parameter("ftolr", new CDoubleParameter(m_options.rel_ftol, 0.0, HUGE_VAL, false, 
				  "Stopping criterion: the relative change of the objective value is below "
						    " this value")); 

	add_parameter("maxiter", new CIntParameter(m_options.maxiter, 1, numeric_limits<int>::max(), false, 
						   "Stopping criterion: the maximum number of iterations")); 
	
	add_parameter("step", new CDoubleParameter(m_options.step, 0.0, HUGE_VAL, false, 
					 "Initial step size for gradient free methods")); 

	add_parameter("lower", new CDoubleParameter(m_options.min_boundary, -HUGE_VAL, HUGE_VAL, false, 
					 "Lower boundary (equal for all parameters)")); 

	add_parameter("higher", new CDoubleParameter(m_options.max_boundary, -HUGE_VAL, HUGE_VAL, false, 
					 "Higher boundary (equal for all parameters)")); 

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
