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


#include <cassert>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_vector.h>

#include <mia/core/minimizer.hh>

NS_BEGIN(gsl)


// The FDF minimizer of the GSL 
class CGSLFDFMinimizer : public mia::CMinimizer {
public: 
	CGSLFDFMinimizer(const gsl_multimin_fdfminimizer_type *ot, 
			 double gorth_tolerance, 
			 double stop_eps, 
			 unsigned int maxiter, 
			 float start_step);
	
	~CGSLFDFMinimizer(); 

	static double f(const gsl_vector * x, void * params); 
	static void df(const gsl_vector * x, void * params, gsl_vector * g); 
	static void fdf(const gsl_vector * x, void * params, double * f, gsl_vector * g); 
	
private: 
	virtual void do_set_problem();
	virtual int do_run(mia::CDoubleVector& x);
	
	const gsl_multimin_fdfminimizer_type *m_ot; 
	gsl_multimin_fdfminimizer *m_s; 
	double m_gorth_tolerance; 
	double m_stop_eps; 
	gsl_multimin_function_fdf m_func; 
	int m_maxiter;  
	double m_start_step;

}; 


// The F minimizer of the GSL 
class CGSLFMinimizer : public mia::CMinimizer {
public: 
	CGSLFMinimizer(const gsl_multimin_fminimizer_type *ot,
		       double stop_eps, 
		       unsigned int maxiter, 
		       double start_step);
	
	~CGSLFMinimizer(); 

	static double f(const gsl_vector * x, void * params); 
	
private: 
	virtual void do_set_problem();
	virtual int do_run(mia::CDoubleVector& x);
	
	const gsl_multimin_fminimizer_type *m_ot; 
	gsl_multimin_fminimizer *m_s; 
	double m_stop_eps; 
	gsl_multimin_function m_func; 
	int m_maxiter;  
	double m_start_step;
	gsl_vector *m_step_init; 	
}; 

class CGSLMinimizerPlugin: public mia::CMinimizerPlugin {
public: 
	CGSLMinimizerPlugin();

	enum EGSLOptimizer {
		opt_simplex2, 
		opt_cg_fr, 
		opt_cg_pr, 
		opt_bfgs, 
		opt_bfgs2, 
		opt_gd, 
		opt_unknown

	};
	
private:

	mia::CMinimizer *do_create() const;
	const std::string do_get_descr() const;
	
	EGSLOptimizer m_ot;
	double m_gorth_tolerance; 
	double m_stop_eps; 
	int m_maxiter;  
	double m_start_step;


}; 

NS_END
