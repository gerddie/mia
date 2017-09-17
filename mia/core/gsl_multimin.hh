/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef GSLPP_MULTIMIN_HH
#define GSLPP_MULTIMIN_HH

#include <memory>
#include <mia/core/gsl_vector.hh>
#include <gsl/gsl_multimin.h>
#include <mia/core/gsl_defines.hh>

namespace gsl {

/**
   This class wraps the gradient based optimizers of the GSL 
 */
class EXPORT_GSL  CFDFMinimizer {
public: 
	/**
	   This is the base class for all optimization problems that provide 
	   gradient information for optimization. 
	 */
	class Problem {
	public:
		/**
		   Initialize the optimization problem with the given number of parameters
		 */
		Problem(size_t n_params); 

		/**
		   Callback to evaluate the value of the optimization criterion 
		   To derive a CFDFMinimizer::Problem, the do_f methods has to be implemented accordingly. 
		   \remark actually this function should be private and only visible CFDFMinimizer
		 */
		static double f(const gsl_vector * x, void * params); 
		
		/**
		   Callback to evaluate the gradient of the optimization criterion 
		   To derive a CFDFMinimizer::Problem, the do_df methods has to be implemented accordingly. 
		   \remark actually this function should be private and only visible CFDFMinimizer
		 */
		static void df(const gsl_vector * x, void * params, gsl_vector * g); 

		/**
		   Callback to evaluate the gradient and the value of the optimization criterion 
		   To derive a CFDFMinimizer::Problem, the do_fdf methods has to be implemented accordingly. 
		   \remark actually this function should be private and only visible to CFDFMinimizer
		 */
		static void fdf(const gsl_vector * x, void * params, double * f, gsl_vector * g); 

		operator gsl_multimin_function_fdf*(); 

		size_t size() const; 
	private: 
		virtual double  do_f(const Vector& x) = 0; 
		virtual void    do_df(const Vector& x, Vector&  g) = 0; 
		virtual double  do_fdf(const Vector& x, Vector&  g) = 0; 
		gsl_multimin_function_fdf m_func; 
	}; 
	typedef std::shared_ptr<Problem> PProblem; 

	/**
	   Construtor of the optimizer. 
	   \param p problem to be optimized 
	   \param ot optimizer type used 
	 */
	CFDFMinimizer(PProblem p, const gsl_multimin_fdfminimizer_type *ot); 
	
	
	~CFDFMinimizer(); 

	/**
	   Set the gradient tolerance stopping criterion. (See GSL documentation.) 
	 */
	void set_g_tol(double tol); 

	/**
	   Set the epsilon stopping criterion. (See GSL documentation.) 
	 */
	void set_stop_eps(double tol); 
	
	/**
	   Run the optimization 
	   \param[in,out] x at entry contains the start point of the optimization at exit the optimized value 
	   \returns returns a status whether the optimization succeeded or why it stopped 
	 */
	int run(Vector& x); 
	
private: 
	struct CFDFMinimizerImpl *impl; 
}; 


/**
   This class wraps the gradient free optimizers of the GSL 
 */
class EXPORT_GSL CFMinimizer {
public: 
	/**
	   This is the base class for all optimization problems that don't provide 
	   gradient information for optimization. 
	 */
	
	class Problem {
	public:
		/**
		   Initialize the optimization problem with the given number of parameters
		 */
		Problem(size_t n_params); 
		
		/**
		   Callback to evaluate the value of the optimization criterion 
		   To derive a CFMinimizer::Problem, the do_f methods has to be implemented accordingly. 
		   \remark actually this function should be private and only visible CFMinimizer
		 */

		static double f(const gsl_vector * x, void * params); 

		operator gsl_multimin_function*(); 

		size_t size() const; 
	private: 
		virtual double  do_f(const Vector& x) = 0; 
		gsl_multimin_function m_func; 
	}; 
	typedef std::shared_ptr<Problem> PProblem; 

	/**
	   Construtor of the optimizer. 
	   \param p problem to be optimized 
	   \param ot optimizer type used 
	 */
	CFMinimizer(PProblem p, const gsl_multimin_fminimizer_type *ot); 
	
	~CFMinimizer(); 
	
	/**
	   Run the optimization 
	   \param[in,out] x at entry contains the start point of the optimization at exit the optimized value 
	   \returns returns a status whether the optimization succeeded or why it stopped 
	 */
	int run(Vector& x); 
private: 
	struct CFMinimizerImpl *impl; 
}; 
	
}

#endif
