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


#ifndef mia_core_minimizer_hh
#define mia_core_minimizer_hh

#include <memory>
#include <vector>

#include <mia/core/factory.hh>
#include <mia/core/handler.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN


/**
   \ingroup classes
   \brief A class for generalized minimization problems 
   
   This class defined the interface for a generalized minimizer that may use 
   gradient information. Specific implementations are provided as plug-ins. 
   
 */
class EXPORT_CORE  CMinimizer : public CProductBase {
public: 

	/// plug-in searchpath typedef helper
	typedef CMinimizer plugin_data; 
	/// plug-in searchpath typedef helper
	typedef CMinimizer plugin_type; 

	/// Pointer type of this minimizer 
	typedef std::shared_ptr<CMinimizer> Pointer; 
	
	/// enum to describe whether optimization succeeded 
	enum EMinimizerResult {failure, /**< optimization failed */ 
			       success  /**< optimization succeeded */ 
	}; 

	/// plug-in searchpath helper
	static const char *const type_descr; 
	
	/// plug-in searchpath helper
	static const char *const data_descr; 
	
	/**
	   \brief Base class for all optimization problems that can be run 
	   by CMinimizer
	   
	   This is the abstract base class for all optimization problems that can be run by
	   CMinimizer. In order to implement a real optimizable problem, the abstract functions 
		- virtual double  do_f(const CDoubleVector& x) = 0; 
		- virtual void    do_df(const CDoubleVector& x, CDoubleVector&  g) = 0; 
		- virtual double  do_fdf(const CDoubleVector& x, CDoubleVector&  g) = 0; 

   	   have to be implemented in the derived class.
	   These functions must provide the functionality as documented for the public member functions 
	        - double  f(size_t n, const double *x); 
		- void    df(size_t n, const double *x, double *g); 
		- double  fdf(size_t n, const double *x, double *g); 
	*/
	class Problem : public CPropertyFlagHolder{
	public:
		virtual ~Problem(); 

		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function value.  
		   @param n number of parameters 
		   @param x vector of parameters 
		   @returns function value 
		*/
		double  f(size_t n, const double *x); 

		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient.  
		   @param n number of parameters 
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   
		*/
		
		void    df(size_t n, const double *x, double *g); 
		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient and the function value.  
		   @param n number of parameters 
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   @returns objective function value 
		*/
		double  fdf(size_t n, const double *x, double *g); 

		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function value.  
		   @param x vector of parameters 
		   @returns function value 
		   
		*/
		double  f(const std::vector<double>& x); 
		
		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient.  
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   
		*/
		void    df(const std::vector<double>& x, std::vector<double>& g); 
		
                /**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient and the function value.  
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   @returns objective function value 
		*/
		double  fdf(const std::vector<double>& x, std::vector<double>& g); 

		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function value.  
		   @param x vector of parameters 
		   @returns function value 
		   
		*/
		double  f(const CDoubleVector& x); 
		
		/**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient.  
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   
		*/
		void    df(const CDoubleVector& x, CDoubleVector& g); 
		
                /**
		   The function that is called by the optimizer to evaluate the 
		   objective function gradient and the function value.  
		   @param x vector of parameters 
		   @param[out] g vector of gradient  of the objective function 
		   @returns objective function value 
		*/
		double  fdf(const CDoubleVector& x, CDoubleVector& g); 


		/// \returns number of parameters to optimize
		size_t size() const; 
	private: 
		virtual double  do_f(const CDoubleVector& x) = 0; 
		virtual void    do_df(const CDoubleVector& x, CDoubleVector&  g) = 0; 
		virtual double  do_fdf(const CDoubleVector& x, CDoubleVector&  g) = 0; 
		virtual size_t do_size() const = 0; 
	}; 

	/// pointer type for the optimization problem 
	typedef std::shared_ptr<Problem> PProblem; 

	/**
	   Construtor of the optimizer. 
	 */
	CMinimizer(); 

	/**
	   Set the optimization problem 
	   \param x problem to be optimized 
	*/
	void set_problem(PProblem x);
	
	
	virtual ~CMinimizer(); 

	
	/**
	   Run the optimization 
	   \param[in,out] x at entry contains the start point of the optimization at exit the optimized value 
	   \returns returns a status whether the optimization succeeded or why it stopped 
	 */
	int run(CDoubleVector& x); 
	
protected: 
	/// @returns a raw pointer to the optimization problem 
	Problem *get_problem_pointer();  
	
	/// @returns the size  (degrees of freedom) of the optimization problem
	size_t size() const; 

	/// \returns a read/write reference to the current optimization problem
	Problem& get_problem(); 
private: 
	virtual void do_set_problem();
	virtual int do_run(CDoubleVector& x) = 0;

	PProblem m_problem;
}; 

/// Pointer type for the CMinimizer class 
typedef CMinimizer::Pointer PMinimizer; 

/// Base class for the CMinimizer creator plugins 
typedef TFactory<CMinimizer> CMinimizerPlugin;

/// The minimizer plugin handler 
typedef THandlerSingleton<TFactoryPluginHandler<CMinimizerPlugin> > CMinimizerPluginHandler;

/// Trait to make the minimizer definition parsable on the command line  
FACTORY_TRAIT(CMinimizerPluginHandler); 

inline CMinimizer::Problem& CMinimizer::get_problem()
{
	return *m_problem;
}

inline 
PMinimizer produce_minimizer(const std::string& descr) 
{
	return CMinimizerPluginHandler::instance().produce(descr); 
}

	
NS_MIA_END

#endif
