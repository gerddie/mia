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

#ifndef mia_core_minimizer_hh
#define mia_core_minimizer_hh

#include <memory>
#include <vector>

#include <mia/core/problem.hh>
#include <mia/core/factory.hh>
#include <mia/core/handler.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN


/**
   This class defines a gradient based optimizer
 */
class EXPORT_CORE  CMinimizer : public CProductBase {
public: 
	typedef CMinimizer plugin_data; 
	typedef CMinimizer plugin_type; 
	typedef std::shared_ptr<CMinimizer> Pointer; 
	
	enum EMinimizerResult {failture, success}; 

	static const char *const value; 
	static const char *const type_descr; 
	
	/**
	   This is the base class for all optimization problems that provide 
	   gradient information for optimization. 
	 */
	class Problem : public CPropertyFlagHolder{
	public:
		/**
		   Initialize the optimization problem with the given number of parameters
		 */
		Problem(size_t n_params); 

		double  f(size_t n, const double *x); 
		void    df(size_t n, const double *x, double *g); 
		double  fdf(size_t n, const double *x, double *g); 

		double  f(const std::vector<double>& x); 
		void    df(const std::vector<double>& x, std::vector<double>& g); 
		double  fdf(const std::vector<double>& x, std::vector<double>& g); 


		/// \returns number of parameters to optimize
		size_t size() const; 
	private: 
		virtual double  do_f(const CDoubleVector& x) = 0; 
		virtual void    do_df(const CDoubleVector& x, CDoubleVector&  g) = 0; 
		virtual double  do_fdf(const CDoubleVector& x, CDoubleVector&  g) = 0; 
		size_t m_size; 
	}; 
	typedef std::shared_ptr<Problem> PProblem; 

	/**
	   Construtor of the optimizer. 
	 */
	CMinimizer(); 

	/**
	   \param p problem to be optimized 
	*/
	void set_problem(PProblem x);
	
	
	virtual ~CMinimizer(); 

	
	/**
	   Run the optimization 
	   \retval x at entry contains the start point of the optimization at exit the optimized value 
	   \returns returns a status whether the optimization succeeded or why it stopped 
	 */
	int run(CDoubleVector& x); 
	
protected: 
	Problem *get_problem_pointer();  
	size_t size() const; 
private: 
	virtual void do_set_problem();
	virtual int do_run(CDoubleVector& x) = 0;

	PProblem m_problem;
}; 

typedef CMinimizer::Pointer PMinimizer; 

typedef TFactory<CMinimizer> CMinimizerPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CMinimizerPlugin> > CMinimizerPluginHandler;
FACTORY_TRAIT(CMinimizerPluginHandler); 


	
NS_MIA_END

#endif
