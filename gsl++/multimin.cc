/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <gsl++/multimin.hh>

#include <stdexcept>
#include <algorithm>

using std::transform; 

namespace gsl {


struct CFDFMinimizerImpl {
	CFDFMinimizerImpl(CFDFMinimizer::PProblem problem, const gsl_multimin_fdfminimizer_type *ot); 
	~CFDFMinimizerImpl(); 
	
	int run(DoubleVector& x); 

	CFDFMinimizer::PProblem m_problem; 
	const gsl_multimin_fdfminimizer_type *m_optimizer_type; 

	gsl_multimin_fdfminimizer *m_s; 
	double m_gorth_tolerance; 
	double m_stop_eps; 

}; 

CFDFMinimizer::CFDFMinimizer(PProblem problem, const gsl_multimin_fdfminimizer_type *ot):
	impl(new CFDFMinimizerImpl(problem, ot))
{
}

int CFDFMinimizer::run(DoubleVector& x) 
{
	return impl->run(x); 
}

CFDFMinimizer::~CFDFMinimizer()
{
	delete impl; 
}

CFDFMinimizer::Problem::Problem(size_t n)
{
	m_func.n = n; 
	m_func.f = Problem::f; 
	m_func.df = Problem::df; 
	m_func.fdf = Problem::fdf; 
	m_func.params = (void *)this; 
}

double CFDFMinimizer::Problem::f(const gsl_vector * x, void * params)
{
	Problem *p = (Problem *)params; 
	return p->do_f(DoubleVector(x)); 
}

void CFDFMinimizer::Problem::df(const gsl_vector * x, void * params, gsl_vector * g)
{
	Problem *p = (Problem *)params; 
	const DoubleVector vx(x); 
	DoubleVector gx(g); 
	p->do_df(vx,gx); 
}

void CFDFMinimizer::Problem::fdf(const gsl_vector * x, void * params, double * f, gsl_vector * g)
{
	Problem *p = (Problem *)params; 
	const DoubleVector vx(x); 
	DoubleVector gx(g); 
	*f = p->do_fdf(vx,gx); 
}

size_t CFDFMinimizer::Problem::size() const
{
	return 	m_func.n; 
}

CFDFMinimizer::Problem::operator gsl_multimin_function_fdf*() 
{
	return &m_func; 
}

CFDFMinimizerImpl::CFDFMinimizerImpl(CFDFMinimizer::PProblem problem, const gsl_multimin_fdfminimizer_type *ot):
	m_problem(problem), 
	m_optimizer_type(ot), 
	m_gorth_tolerance(0.1),
	m_stop_eps(0.01)
{
	m_s = gsl_multimin_fdfminimizer_alloc (m_optimizer_type, m_problem->size()); 	
	if (!m_s) 
		throw std::runtime_error("CFDFMinimizer:Not enough memory to allocate the minimizer"); 
}

CFDFMinimizerImpl::~CFDFMinimizerImpl()
{
	gsl_multimin_fdfminimizer_free (m_s);
}


void CFDFMinimizer::set_g_tol(double tol)
{
	impl->m_gorth_tolerance = tol;
}

void CFDFMinimizer::set_stop_eps(double tol)
{
	impl->m_stop_eps = tol; 
}


int CFDFMinimizerImpl::run(DoubleVector& x)
{
	int iter = 0; 
	int status = GSL_CONTINUE; 
	gsl_multimin_fdfminimizer_set (m_s, *m_problem, x, 0.001, m_gorth_tolerance);
	do {
		++iter; 
		status = gsl_multimin_fdfminimizer_iterate (m_s);
		if (status) 
			break; 
		
		status = gsl_multimin_test_gradient (m_s->gradient, m_stop_eps);
	} while (status == GSL_CONTINUE && iter < 100); 
	
	// copy best solution 
	gsl_vector * help = gsl_multimin_fdfminimizer_x (m_s); 
	std::copy(help->data, help->data + m_problem->size(), x.begin()); 

	return status; 
}

CFMinimizer::Problem::Problem(size_t n)
{
	m_func.n = n; 
	m_func.f = Problem::f; 
	m_func.params = (void *)this; 
}

double CFMinimizer::Problem::f(const gsl_vector * x, void * params)
{
	Problem *p = (Problem *)params; 
	return p->do_f(DoubleVector(x)); 
}

size_t CFMinimizer::Problem::size() const
{
	return 	m_func.n; 
}


struct CFMinimizerImpl {
	CFMinimizerImpl(CFMinimizer::PProblem problem, const gsl_multimin_fminimizer_type *ot); 
	~CFMinimizerImpl(); 
	
	int run(DoubleVector& x); 

	CFMinimizer::PProblem m_problem; 
	const gsl_multimin_fminimizer_type *m_optimizer_type; 

	gsl_multimin_fminimizer *m_s; 	
	gsl_vector *m_step_size; 
}; 

CFMinimizer::Problem::operator gsl_multimin_function*() 
{
	return &m_func; 
}


CFMinimizer::CFMinimizer(PProblem p, const gsl_multimin_fminimizer_type *ot):
	impl(new CFMinimizerImpl(p,ot))
{
}

CFMinimizer::~CFMinimizer()
{
	delete impl; 
}
	
int CFMinimizer::run(DoubleVector& x)
{
	return impl->run(x); 
}


CFMinimizerImpl::CFMinimizerImpl(CFMinimizer::PProblem problem, const gsl_multimin_fminimizer_type *ot):
	m_problem(problem), 
	m_optimizer_type(ot)
{
	m_s = gsl_multimin_fminimizer_alloc (m_optimizer_type, m_problem->size()); 	
	if (!m_s) 
		throw std::runtime_error("CFMinimizer:Not enough memory to allocate the minimizer"); 
	m_step_size = gsl_vector_alloc(m_problem->size());
	
}

CFMinimizerImpl::~CFMinimizerImpl()
{
	gsl_multimin_fminimizer_free (m_s);	
	gsl_vector_free(m_step_size); 
}

int CFMinimizerImpl::run(DoubleVector& x)
{
	int iter = 0; 
	int status;  
	
	gsl_vector_set_all (m_step_size, 1.0);
	gsl_multimin_fminimizer_set (m_s, *m_problem, x, m_step_size);
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
	std::copy(help->data, help->data + m_problem->size(), x.begin()); 
	return status; 
}

} // end namespace gsl 
