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


#ifndef mia_core_sparse_solver_hh
#define mia_core_sparse_solver_hh


#include <mia/core/product_base.hh>
#include <mia/core/traits.hh>

NS_MIA_BEGIN

/**
   \brief solver for sparse systems of equations 

   This is the templatex base class for solvers for systems of equations 
   Ax=b if A where the multiplication Ax can be expressed as a convolution 
   operation with a N dimansional operator. 
   \tparam Field the field this solver works on. The Field type must provide the 
           types const_iterator and iterator that implement random access iterators. 
 */
template <typename F> 
class TSparseSolver : public CProductBase{
public: 
	typedef F Field;
	
	typedef F plugin_data; 
	
	typedef typename dim_traits<F>::dimsize_type dimsize_type; 

	typedef TSparseSolver<F> plugin_type; 

	static const char * const type_descr; 

	/// type of the field random access iterator 
	typedef typename Field::iterator field_iterator;  
	
	/// type of the field random access const_iterator 
	typedef typename Field::const_iterator const_field_iterator;  
	
	/// type of the values stored in Field 
	typedef typename Field::value_type value_type; 
	
	/**
	   A class that implements the multiplication of a cell of the 
	   matrix A with the field x.
	 */
	
	class A_mult_x: public CProductBase {
	public: 
		
		typedef F plugin_data; 
		
		typedef A_mult_x plugin_type; 
		
		static const char * const type_descr; 
		
		/** standard constructor sets the expected fiels size to zero */
		A_mult_x():m_size(dimsize_type()){}; 

		/** constructor sets the expected fiels size to given value */
		A_mult_x(const dimsize_type& size):m_size(size){};

		/** destructor to ensure virtual distruction */
		virtual ~A_mult_x() {}; 

		/**
		   Operator to execute the multiplication 
		   \param ix is the random access iterator to the 
                          central value of the input vector that would be 
		          multiplied by the element on the main diagonal of A 
		   \returns the result for the cell multiplication 
		 */
		virtual value_type operator () (const_field_iterator ix) const = 0;

		/**
		   Returns the size of the boundary that one has to take into account 
		 */
		virtual int get_boundary_size() const = 0; 
		
		/**
		   \returns the sizes of the (multidimensional) array that is expected 
		            by the operator 
		 */
		dimsize_type get_size() const {
			return m_size; 
		}
	private: 
		dimsize_type m_size; 
		
	}; 

	virtual ~TSparseSolver() {}; 


	/**
	   Run the solver 
	   \param rhs the right hand side of teh system of linear equations to be solved 
	   \param[in,out] at input an initila estimate of the solution, on output the solution 
	   \param mult the class implementing the multiplication with matrix A
	   \returns 0 if all went fine and an algorithm specific error value if something went wrong. 
	*/
	virtual int solve(const Field& rhs, Field& x, const A_mult_x& mult) const = 0; 
}; 

template <typename F> 
const char * const TSparseSolver<F>::type_descr = "sparsesolver"; 


template <typename F> 
const char * const TSparseSolver<F>::A_mult_x::type_descr = "amultx"; 

NS_MIA_END

#endif
