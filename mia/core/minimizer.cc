/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
  LatexBeginPluginSection{Minimizers}
  \label{sec:minimizers}
  
  These minimizers are used to optimize functions of the type $F:\Omega \rightarrow \R$. 
  
  Minimizers are created by the CMinimizerPluginHandler singleton.

  LatexEnd
*/


#include <stdexcept>
#include <sstream>
#include <boost/lambda/lambda.hpp>
#include <algorithm>
#include <mia/core/minimizer.hh>
#include <mia/core/errormacro.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>


NS_MIA_BEGIN

const char *const CMinimizer::type_descr = "singlecost"; 
const char *const CMinimizer::data_descr = "minimizer"; 


size_t CMinimizer::Problem::size() const
{
	return do_size(); 
}

CMinimizer::CMinimizer()
{
}

void CMinimizer::set_problem(PProblem p)
{
	m_problem = p; 
	do_set_problem(); 
}

size_t CMinimizer::size() const
{
	return m_problem ? m_problem->size() : 0; 
}

void CMinimizer::do_set_problem()
{
}

double  CMinimizer::Problem::f(const std::vector<double>& x)
{
	assert(x.size() == size()); 
	const CDoubleVector params(x.size(), &x[0]); 
	return do_f(params); 	
}
void    CMinimizer::Problem::df(const std::vector<double>& x, std::vector<double>& g)
{
	assert(x.size() == size()); 
	assert(g.size() == size()); 
	const CDoubleVector params_x(x.size(), &x[0]); 
	CDoubleVector params_g(g.size(), &g[0]); 
	do_df(params_x, params_g); 	
}

double  CMinimizer::Problem::fdf(const std::vector<double>& x, std::vector<double>& g)
{
	assert(x.size() == size()); 
	assert(g.size() == size()); 
	const CDoubleVector params_x(x.size(), &x[0]); 
	CDoubleVector params_g(g.size(), &g[0]); 
	return do_fdf(params_x, params_g); 	
}

double  CMinimizer::Problem::f(const CDoubleVector& x)
{
	assert(x.size() == size()); 
	return do_f(x); 	
}

void    CMinimizer::Problem::df(const CDoubleVector& x, CDoubleVector& g)
{
	assert(x.size() == size()); 
	assert(g.size() == size()); 
	do_df(x, g); 	
}

double  CMinimizer::Problem::fdf(const CDoubleVector& x, CDoubleVector& g)
{
	assert(x.size() == size()); 
	assert(g.size() == size()); 
	return do_fdf(x, g); 	
}


CMinimizer::Problem::~Problem()
{
}

double  CMinimizer::Problem::f(size_t n, const double *x)
{
	assert(n == size()); 
	const CDoubleVector params(n, x); 
	return do_f(params); 	
}

void    CMinimizer::Problem::df(size_t n, const double *x, double *g)
{
	assert(n == size()); 
	const CDoubleVector params_x(n, x); 
	CDoubleVector params_g(n, g); 
	do_df(params_x, params_g); 
}

double  CMinimizer::Problem::fdf(size_t n, const double *x, double *g)
{
	assert(n == size()); 
	const CDoubleVector params_x(n, x); 
	CDoubleVector params_g(n, g); 
	return do_fdf(params_x, params_g); 
}

CMinimizer::~CMinimizer()
{
}

CMinimizer::Problem *CMinimizer::get_problem_pointer()
{
	return m_problem.get(); 
}
	
int CMinimizer::run(CDoubleVector& x)
{
	DEBUG_ASSERT_RELEASE_THROW(m_problem, "CMinimizer::run: no minimization problem given");
	if (!m_problem->has_all_in(*this)) {
		stringstream msg; 
		msg << "The optimizer requests some properties that the minimization problem"
			" doesn't provide:"; 
		auto missing = m_problem->get_missing_properties(*this); 
		for (auto i = missing.begin(); i != missing.end(); ++i) 
			msg << "'" <<  *i << "' ";
		throw invalid_argument(msg.str()); 
	}
	return do_run(x); 
}

template<> const char * const 
TPluginHandler<TFactory<CMinimizer>>::m_help = "These plug-ins provide optimizers of many-to-one functions"; 

EXPLICIT_INSTANCE_HANDLER(CMinimizer);

NS_MIA_END
