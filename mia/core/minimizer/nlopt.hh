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


#ifndef mia_core_minimizer_nlopt_hh
#define mia_core_minimizer_nlopt_hh

#include <mia/core/minimizer.hh>
#include <mia/core/dictmap.hh>
#include <nlopt.h>

NS_BEGIN(nlopt)

extern const mia::TDictMap<nlopt_algorithm>::Table optimizer_table[];  

struct SOpt {
	SOpt(); 
	nlopt_algorithm algo; 
	nlopt_algorithm local_opt;
	double stopval; 
	double abs_xtol; 
	double rel_xtol; 
	double rel_ftol; 
	double abs_ftol; 
	double step; 
	int maxiter; 
}; 


// The FDF minimizer of the NOPT
class CNLOptFDFMinimizer : public mia::CMinimizer {
public: 

	CNLOptFDFMinimizer(const SOpt& options);
	
	~CNLOptFDFMinimizer(); 

	static double fdf(unsigned n, const double *x, double *grad, void *self); 
	
		

private: 
	virtual void do_set_problem();
	virtual int do_run(mia::CDoubleVector& x);

	bool require_gradient(nlopt_algorithm algo) const;

	nlopt_opt m_opt;
	SOpt m_options; 

}; 

class CNLOptMinimizerPlugin: public mia::CMinimizerPlugin {
public: 
	CNLOptMinimizerPlugin();
private:
	mia::CMinimizer *do_create() const;
	const std::string do_get_descr() const;

	SOpt m_options;

}; 

NS_END

#endif
