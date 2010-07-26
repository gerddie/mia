/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010 Gert Wollny 
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

#ifndef GSLPP_MULTIMIN_HH
#define GSLPP_MULTIMIN_HH

#include <memory>
#include <gsl++/vector.hh>
#include <gsl/gsl_multimin.h>

namespace gsl {
class CFDFMinimizer {
public: 
	class Problem {
	public:
		Problem(size_t n); 
		static double f(const gsl_vector * x, void * params); 
		static void df(const gsl_vector * x, void * params, gsl_vector * g); 
		static void fdf(const gsl_vector * x, void * params, double * f, gsl_vector * g); 

		operator gsl_multimin_function_fdf*(); 

		size_t size() const; 
	private: 
		virtual double  do_f(const DoubleVector& x) = 0; 
		virtual void    do_df(const DoubleVector& x, DoubleVector&  g) = 0; 
		virtual double  do_fdf(const DoubleVector& x, DoubleVector&  g) = 0; 
		gsl_multimin_function_fdf m_func; 
	}; 
	typedef std::shared_ptr<Problem> PProblem; 

	CFDFMinimizer(PProblem p, const gsl_multimin_fdfminimizer_type *ot); 
	~CFDFMinimizer(); 
	
	int run(DoubleVector& x); 
	
	
private: 
	struct CFDFMinimizerImpl *impl; 
}; 



class CFMinimizer {
public: 
	class Problem {
	public:
		Problem(size_t n); 
		static double f(const gsl_vector * x, void * params); 

		operator gsl_multimin_function*(); 

		size_t size() const; 
	private: 
		virtual double  do_f(const DoubleVector& x) = 0; 
		gsl_multimin_function m_func; 
	}; 
	typedef std::shared_ptr<Problem> PProblem; 

	CFMinimizer(PProblem p, const gsl_multimin_fminimizer_type *ot); 
	~CFMinimizer(); 
	
	int run(DoubleVector& x); 
private: 
	struct CFMinimizerImpl *impl; 
}; 
	
}

#endif
