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

#include <memory>
#include <mia/core/minimizer/gsl.hh>

NS_BEGIN(gsl)

using namespace std;
using namespace mia; 


const TDictMap<CGSLMinimizerPlugin::EGSLOptimizer>::Table minimizer_table[] = {
	{"simplex", CGSLMinimizerPlugin::opt_simplex2},
	{"cg-fr",   CGSLMinimizerPlugin::opt_cg_fr},
	{"cg-pr",   CGSLMinimizerPlugin::opt_cg_pr},
	{"bfgs",    CGSLMinimizerPlugin::opt_bfgs},
	{"bfgs2",   CGSLMinimizerPlugin::opt_bfgs2},
	{"gd",      CGSLMinimizerPlugin::opt_gd},
	{NULL, CGSLMinimizerPlugin::opt_unknown}
};


CGSLFDFMinimizer::CGSLFDFMinimizer(const gsl_multimin_fdfminimizer_type *ot, 
				   double gorth_tolerance, 
				   double stop_eps, 
				   unsigned int maxiter, 
				   float start_step):
	m_ot(ot), 
	m_s(NULL), 
	m_gorth_tolerance(gorth_tolerance), 
	m_stop_eps(stop_eps), 
	m_maxiter(maxiter), 
	m_start_step(start_step)
{
	add(property_gradient); 
}

CGSLFDFMinimizer::~CGSLFDFMinimizer()
{
	gsl_multimin_fdfminimizer_free (m_s);	
}

double CGSLFDFMinimizer::f(const gsl_vector * x, void * obj)
{
	assert(x); 
	assert(obj); 
	
	CMinimizer::Problem *o = (Problem *)obj; 
	return o->f(x->size, x->data); 
}

void CGSLFDFMinimizer::df(const gsl_vector * x, void * obj, gsl_vector * g)
{
	assert(x); 
	assert(obj); 
	assert(g); 
	assert(x->size == g->size); 
	
	CMinimizer::Problem *o = (Problem *)obj; 
	o->df(x->size, x->data, g->data); 
}

void CGSLFDFMinimizer::fdf(const gsl_vector * x, void * obj, double * f, gsl_vector * g)
{
	assert(x); 
	assert(obj); 
	assert(g); 
	assert(x->size == g->size); 
	
	CMinimizer::Problem *o = (Problem *)obj; 
	*f = o->fdf(x->size, x->data, g->data); 
}


void CGSLFDFMinimizer::do_set_problem()
{
	m_func.n = size(); 
	m_func.f = CGSLFDFMinimizer::f; 
	m_func.df = CGSLFDFMinimizer::df; 
	m_func.fdf = CGSLFDFMinimizer::fdf; 
	m_func.params = (void *)get_problem_pointer();
	
	if (m_s) 
		gsl_multimin_fdfminimizer_free (m_s);
	m_s = gsl_multimin_fdfminimizer_alloc (m_ot, size()); 	
	if (!m_s) 
		throw std::runtime_error("CFDFMinimizer:Not enough memory to allocate the minimizer"); 
}

struct DeallocGSLVector {
	void operator ()(gsl_vector*v) {
		gsl_vector_free(v); 
	}
}; 


int CGSLFDFMinimizer::do_run(Parameters& x)
{
	int iter = 0; 
	int status = GSL_CONTINUE; 
	shared_ptr<gsl_vector> init_x(gsl_vector_alloc(x.size()),DeallocGSLVector()); 
	copy(x.begin(), x.end(), init_x->data);  
	
	gsl_multimin_fdfminimizer_set (m_s, &m_func, init_x.get(), m_start_step, m_gorth_tolerance);
	do {
		++iter; 
		status = gsl_multimin_fdfminimizer_iterate (m_s);
		if (status) 
			break; 
		
		status = gsl_multimin_test_gradient (m_s->gradient, m_stop_eps);
	} while (status == GSL_CONTINUE && iter < m_maxiter); 
	
	// copy best solution 
	gsl_vector * help = gsl_multimin_fdfminimizer_x (m_s); 
	std::copy(help->data, help->data + size(), x.begin()); 
	
	return status == GSL_SUCCESS ? CMinimizer::success : CMinimizer::failture;
	
}

CGSLFMinimizer::CGSLFMinimizer(const gsl_multimin_fminimizer_type *ot,
			       double stop_eps, 
			       unsigned int maxiter, double start_step):
	m_ot(ot), 
	m_s(NULL),
	m_stop_eps(stop_eps), 
	m_maxiter(maxiter), 
	m_start_step(start_step), 
	m_step_init(NULL)
{
}

CGSLFMinimizer::~CGSLFMinimizer()
{
	gsl_multimin_fminimizer_free (m_s);
	gsl_vector_free(m_step_init); 	
}

double CGSLFMinimizer::f(const gsl_vector * x, void *obj)
{
	assert(x); 
	assert(obj); 
	
	CMinimizer::Problem *o = (Problem *)obj; 
	return o->f(x->size, x->data); 
}


void CGSLFMinimizer::do_set_problem()
{
	m_func.n = size(); 
	m_func.f = CGSLFDFMinimizer::f; 
	m_func.params = (void *)get_problem_pointer();
	
	if (m_s) 
		gsl_multimin_fminimizer_free (m_s);
	m_s = gsl_multimin_fminimizer_alloc (m_ot, size()); 	
	if (!m_s) 
		throw std::runtime_error("CFMinimizer:Not enough memory to allocate the minimizer"); 
	
	if (m_step_init && m_step_init->size != size()) 
		gsl_vector_free(m_step_init); 
	
	m_step_init= gsl_vector_alloc(size());
}

int CGSLFMinimizer::do_run(Parameters& x)
{
	int iter = 0; 
	int status;  
	shared_ptr<gsl_vector> init_x(gsl_vector_alloc(x.size()), DeallocGSLVector()); 
	copy(x.begin(), x.end(), init_x->data);  
	
	
	gsl_vector_set_all (m_step_init, m_start_step);
	gsl_multimin_fminimizer_set (m_s, &m_func, init_x.get(), m_step_init);
	do {
		++iter; 
		status = gsl_multimin_fminimizer_iterate (m_s);
		if (status) 
			break; 
		const double size = gsl_multimin_fminimizer_size (m_s);
		status = gsl_multimin_test_size (size, 1e-3);
		
	} while (status == GSL_CONTINUE && iter < 100); 
	
	// copy best solution 
	gsl_vector * help = gsl_multimin_fminimizer_x (m_s); 
	std::copy(help->data, help->data + size(), x.begin()); 
	return status == GSL_SUCCESS ? CMinimizer::success : CMinimizer::failture;
}



typedef CDictParameter<CGSLMinimizerPlugin::EGSLOptimizer> CGSLMinimizerParam; 
typedef TDictMap<CGSLMinimizerPlugin::EGSLOptimizer> CGSLMinimizerDict;  

CGSLMinimizerPlugin::CGSLMinimizerPlugin():
	CMinimizerPlugin("gsl"), 
	m_ot(opt_gd), 
	m_gorth_tolerance(0.1),  
	m_stop_eps(0.01),  
	m_maxiter(100), 
	m_start_step(0.001)
	
{
	add_parameter("opt", new CGSLMinimizerParam(m_ot, CGSLMinimizerDict(minimizer_table), 
						    "Specific optimizer to be used."));
	
	add_parameter("tol", new CDoubleParameter(m_gorth_tolerance, 0.001, 10.0, false, 
						"some tolerance parameter"));
	
	add_parameter("eps", new CDoubleParameter(m_stop_eps, 1e-10, 10.0, false, 
						"gradient norm stopping criterion (stop when |g| < eps)"));
	
	add_parameter("iter", new CIntParameter(m_maxiter, 1, numeric_limits<int>::max(), 
						false, "maximum number of iterations"));
	add_parameter("step", new CDoubleParameter(m_start_step, 0, 10, false, 
						 "initial step size"));
}						

const std::string CGSLMinimizerPlugin::do_get_descr() const
{
	return "optimizer plugin based on the multimin optimizers of"
		"the GNU Scientific Library (GSL)"; 
}

CGSLMinimizerPlugin::ProductPtr CGSLMinimizerPlugin::do_create() const
{
	const gsl_multimin_fdfminimizer_type *ot; 
	
	switch (m_ot) {
	case opt_simplex2 : return ProductPtr(new CGSLFMinimizer(gsl_multimin_fminimizer_nmsimplex2, 
							     m_stop_eps, m_maxiter, m_start_step));
	case opt_cg_fr: ot = gsl_multimin_fdfminimizer_conjugate_fr; break;   
	case opt_cg_pr: ot = gsl_multimin_fdfminimizer_conjugate_pr; break;   
	case opt_bfgs: ot = gsl_multimin_fdfminimizer_vector_bfgs; break;   
	case opt_bfgs2: ot = gsl_multimin_fdfminimizer_vector_bfgs2; break;   
	case opt_gd: ot = gsl_multimin_fdfminimizer_steepest_descent; break;  
	default:
		throw invalid_argument("Unknown GSL optimizer given"); 
	}
	return ProductPtr(new CGSLFDFMinimizer(ot, m_gorth_tolerance, 
					       m_stop_eps, m_maxiter, 
					       m_start_step));
}

NS_END