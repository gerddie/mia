/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_sparse_solver_hh
#define mia_core_sparse_solver_hh


#include <mia/core/product_base.hh>
#include <mia/core/traits.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   \brief solver for sparse systems of equations 

   This is the templatex base class for solvers for systems of equations 
   Ax=b if A where the multiplication Ax can be expressed as a convolution 
   operation with a N dimansional operator. 
   \tparam F the field this solver works on. The Field type must provide the 
   types const_iterator and iterator that implement random access iterators. 
   \remark this class is currently not used at all and it may actually be better to 
   design it to interface with a library like LAPACK etc.
*/
template <typename F> 
class TSparseSolver : public CProductBase{
public: 
	/// typedef for the field the solver works on 
	typedef F Field;
	
	/// helper typedef for the plug-in handling 
	typedef F plugin_data; 
	
	/// typeef of the dimension type 
	typedef typename dim_traits<F>::dimsize_type dimsize_type; 

	/// helper for the plug-in handling 
	typedef TSparseSolver<F> plugin_type; 
	
	/// plugin type component of plugin path descriptor
	static const char * const type_descr; 

	/// type of the field random access iterator 
	typedef typename Field::iterator field_iterator;  
	
	/// type of the field random access const_iterator 
	typedef typename Field::const_iterator const_field_iterator;  
	
	/// type of the values stored in Field 
	typedef typename Field::value_type value_type; 
	
	/**
	   \brief A class that implements the multiplication of a cell of the 
	   matrix A with the field x.
	   
	   This class defines the (sparse) multiplication of the matrix A with a vector x
	 */
	
	class A_mult_x: public CProductBase {
	public: 
		/// helper typedef for the plug-in handling 
		typedef F plugin_data; 
		
		/// helper typedef for the plug-in handling 
		typedef A_mult_x plugin_type; 
		
		/// plugin type component of plugin path descriptor
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
		   \remark For efficiency no tests are done whether the boundary conditions are actually honoured. 
		   this is the job of the solver calling the operator 
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
	   \param[in,out] x at input an initial estimate of the solution, on output the solution 
	   \param mult the class implementing the multiplication with matrix A
	   \returns 0 if all went fine and an algorithm specific error value if something went wrong. 
	*/
	virtual int solve(const Field& rhs, Field& x, const A_mult_x& mult) const = 0; 
}; 

template <typename F> 
const char * const TSparseSolver<F>::type_descr = "sparsesolver"; 


template <typename F> 
const char * const TSparseSolver<F>::A_mult_x::type_descr = "amultx"; 


/**
   \brief Class template to implement a matrix - field multiplication. 
   
   This class implements the multiplication of a sparse structured matrix by a field representing 
   the input vector data. This structure is just a stub ans needs to be instanciated for 
   specific fields. 
   \tparam F the field that representes the data to be multiplied by the matrix 
*/

template <typename F>
struct multiply {
	/**
	   The function to apply the multiplication 
	   \param[out] result location to store the result, ususally it should be allocated and of the same size like x 
	   \param[in] A sparse Matrix 
	   \param[in] x the filed to multiply with the matrix 
	 */
	static void apply(F& MIA_PARAM_UNUSED(result), 
			  const typename TSparseSolver<F>::A_mult_x& MIA_PARAM_UNUSED(A), const F& MIA_PARAM_UNUSED(x)) {
		static_assert(sizeof(F)==0, "This operation needs to be specialized");  
	}
}; 


/**
   Implement the multiplication of a Matrix with an Image, 
   \param A the matrix 
   \param x the input image 
   \returns A * x, (works like a matrix filter) 
   \remark maybethis should go into the filter section 
 */

template <typename F>
F operator * (const typename TSparseSolver<F>::A_mult_x& A, const F& x)
{
	F result(x.get_size()); 
	multiply<F>::apply(result, A, x); 
	return result;
}


NS_MIA_END

#endif
