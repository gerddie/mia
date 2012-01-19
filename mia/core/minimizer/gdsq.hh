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


#include <mia/core/minimizer.hh>

NS_BEGIN(minimizer_gdsq)

#define SUCCESS_FTOLR 1
#define SUCCESS_XTOLA 2
#define SUCCESS_GTOLA 4

// The FDF minimizer of the GSL 
class CGDSQMinimizer : public mia::CMinimizer {
public: 
	CGDSQMinimizer(double start_step, double step_scale, 
		       double xtol, double gtol, double ftolr, 
		       unsigned int maxiter, double min_step);
	
private: 
	virtual void do_set_problem();
	virtual int do_run(mia::CDoubleVector& x);
	int test_tol(mia::CDoubleVector& dx, double tol, int cause)const; 
	
	double m_xtol; 
	double m_gtol; 
	unsigned int m_maxiter; 
	double m_step; 
	double m_step_scale; 
	double m_ftolr; 
	double m_min_step; 
}; 



class CGDSQMinimizerPlugin: public mia::CMinimizerPlugin {
public: 
	CGDSQMinimizerPlugin();

private:

	mia::CMinimizer *do_create() const;
	const std::string do_get_descr() const;
	
	double m_xtol; 
	double m_gtol; 
	double m_ftolr; 
	unsigned int m_maxiter; 
	double m_start_step; 
	double m_step_scale; 
	double m_min_step; 


}; 

NS_END
