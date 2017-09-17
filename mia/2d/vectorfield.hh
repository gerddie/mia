/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
class EXPORT_2D  T2DVectorfield: public T2DDatafield<T>, public CAttributedData {
public:
	/// plug.in related type description string 
	static const char *data_descr;

	T2DVectorfield();

	/**
	   Contruct a vector field of the given size 
	   @param size of vector field 
	 */
	T2DVectorfield(const C2DBounds& size);


	/**
	   @param data meta data  
	   @param size of vector field 
	 */
	T2DVectorfield(const CAttributedData& data, const C2DBounds& size); 

	C2DFVector get_pixel_size() const; 

	void set_pixel_size(const C2DFVector& pixel); 

	/// This function should really go away 
	T get_interpol_val_at(const C2DFVector& p) const;

	
};

/// @cond never

extern template class EXPORT_2D T2DVectorfield<C2DFVector>;
extern template class EXPORT_2D T2DVectorfield<C2DDVector>;

/// @endcond never 

/**
   @ingroup basic 
   @brief a 2D field of floating point single accuracy 2D vectors 
*/
class EXPORT_2D C2DFVectorfield : public T2DVectorfield<C2DFVector> {
public: 
	
	using T2DVectorfield<C2DFVector>::T2DVectorfield; 

	/**
	   \brief evaluate this vector field as the inverse of another 
	   
	   This functions corrects the vector field to describe the inverse transformation 
	   of a given input vector field 

	   \param other the vector field this one should be inverse of
	   \param tol tolerance for inverse accuracy 
	   \param maxiter maximum number of interations for one vector to be optimized 
	 */
	void update_as_inverse_of(const C2DFVectorfield& other, float tol, int maxiter);

	/**
	   Update this vector field by using a velocity field
	   \param velocity_field the velocity field 
	   \param time_step the time step to be used for the update 
	 */
	void update_by_velocity(const C2DFVectorfield& velocity_field, float time_step); 
	
};


typedef std::shared_ptr<C2DFVectorfield > P2DFVectorfield;

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
