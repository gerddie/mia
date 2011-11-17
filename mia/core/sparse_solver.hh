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


#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \brief solver for sparse systems of equations 

   This is the templatex base class for solvers for systems of equations 
   Ax=b if A where the multiplication Ax can be expressed as a convolution 
   operation with a N dimansional operator. 
   \tparam the field this solver works on. 
 */
template <typename Field> 
class TSparseSolver {
public: 
	typedef typename Field::iterator field_iterator;  
	typedef typename Field::const_iterator const_field_iterator;  

	class A_mult_x {
	public: 
		virtual T operator (const_field_iterator ix) const = 0;
	}; 

	virtual ~TSparseSolver() {}; 

	virtual int solve(const Field& b, Field& x, const A_mult_x& mult) const = 0; 
}; 

NS_MIA_END

#endif
