/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_2d_2dvectorfield_hh
#define mia_2d_2dvectorfield_hh

#include <mia/core/attributes.hh>
#include <mia/2d/datafield.hh>

NS_MIA_BEGIN

/**
   \ingroup basic 
   \brief 2D Vector field 
   
   This class provides an  interface to make the IO opf vector fields possible 
 */
template <typename T>
class EXPORT_2D T2DVectorfield: public T2DDatafield<T>, public CAttributedData {
public:
	/// plug.in related type description string 
	static const char *data_descr;

	T2DVectorfield(){};

	/**
	   Contruct a vector field of the given size 
	   @param size of vector field 
	 */
	T2DVectorfield(const C2DBounds& size):
		T2DDatafield<T>(size) {};


	/**
	   @param data meta data  
	   @param size of vector field 
	 */
	T2DVectorfield(const CAttributedData& data, const C2DBounds& size):
		T2DDatafield<T>(size),
		CAttributedData(data)
	{
	}
};

/// 2D vector field to store single precicion 2D vectors 
typedef T2DVectorfield<C2DFVector>  C2DFVectorfield;

/// 2D vector field to store double precicion 2D vectors 
typedef T2DVectorfield<C2DDVector>  C2DDVectorfield;

/**
   Concat two vector fields that descrive a transformation 
   T_a = x - a(x) and T_b = x - b(x) 
   @param[in,out] a ; output=a( x - b(x) ) + b(x); 
   @param b 
   @returns a( x - b(x) ) + b(x); 
*/
EXPORT_2D C2DFVectorfield& operator += (C2DFVectorfield& a, const C2DFVectorfield& b);

NS_MIA_END

#endif