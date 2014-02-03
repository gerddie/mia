/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_transform_hh
#define mia_3d_transform_hh

#include <iterator>
#include <memory>

#include <mia/core/vector.hh>
#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/3d/matrix.hh>
#include <mia/3d/image.hh>
#include <mia/3d/interpolator.hh>

NS_MIA_BEGIN


class C3DTransformation;

/**
   \ingroup registration 
   \brief pointer type for a 3D transformation 
*/
typedef std::shared_ptr<C3DTransformation > P3DTransformation;

/**
   \ingroup registration 
   @brief This is the generic base class for 3D transformations.

   This class defines the interface for 3D transformations. Most methods are pure abstract 
   and need to be implemented by a "real" transformation. 
*/

class EXPORT_3D C3DTransformation: public Transformation<C3DImage, C3DInterpolatorFactory> {
public:
	/// @cond LAZY 
	typedef C3DBounds Size; 
	typedef C3DImage Data;
	typedef C3DFVector Vector;
	typedef C3DInterpolatorFactory InterpolatorFactory;
	typedef C3DTransformation type; 
	typedef std::shared_ptr<C3DTransformation> Pointer; 
	static const char *data_descr;
	static const char *dim_descr;
	/// @endcond 
protected: 
	/**
	   This is the abstract base class of the actual implementation of the transformation iterator.  
	*/
	class iterator_impl  {
	public: 
		iterator_impl(); 

		/**
		   Initialize the iterator with its current position and the size of the domain 
		   the actual domain is: 
		   \f$[0,size.x-1]\times [0,size.y-1]\times [0,size.z-1]\f$
		 */
		iterator_impl(const C3DBounds& pos, const C3DBounds& size); 

		/**
		   Initialize the iterator with its current position, 
		   the subarea to be traveresed, and 
		   the size of the domain the complete domain is: 
		   \f$[0,size.x-1]\times [0,size.y-1]\times [0,size.z-1]\f$
		   
		 */
		iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
			      const C3DBounds& end, const C3DBounds& size); 
		
		/// move to next position 
		void increment(); 

		/// @returns current value of the underlying transformation 
		const C3DFVector&  get_value() const;

		/// @returns a dynamic copy of the iterator 
		virtual iterator_impl * clone() const  __attribute__((warn_unused_result)) = 0; 
		
		/**
		   Compare this iterator to another one. Iterators are equal if they are at the same 
		   position or at the end.
		*/
		bool operator == (const iterator_impl& other) const; 
		
		/// @returns the current position on the support domain grid 
		const C3DBounds& get_pos()const; 

		/// @returns the size of the supported domain 
		const C3DBounds& get_size()const; 
	private:
		virtual const C3DFVector& do_get_value()const = 0; 
		virtual void do_z_increment() = 0; 
		virtual void do_y_increment() = 0; 
		virtual void do_x_increment() = 0; 
		
		C3DBounds m_pos; 
		C3DBounds m_size; 
		C3DBounds m_start; 
		C3DBounds m_end; 


	}; 
public: 
	/**
	   This is an iterator that iterates over the grid of the 
	   domain where the transformation is defined. 
	   It implements the forward_iterator model. 
	   @todo make position readable from outside
	   @todo add iteration over sub-domains  
	 */
	
	class const_iterator : public std::forward_iterator_tag {
	public: 

		/// @cond some_STL_conform_typedefs
		typedef std::forward_iterator_tag iterator_category; 
		typedef C3DFVector value_type; 
		typedef size_t difference_type; 
		typedef C3DFVector *pointer; 
		typedef C3DFVector& reference; 
		/// @endcond 
		
		const_iterator(); 

		/// Copy constructor 
		const_iterator(const const_iterator& other); 		

		/// initialize the iterator with the actual worker object 
		const_iterator(iterator_impl * holder); 

		/// assignment operator 
		const_iterator& operator = (const const_iterator& other); 

		/// prefix increment 
		const_iterator& operator ++(); 
		
		/// postfix increment 
		const_iterator operator ++(int); 

		/// return current value of the transformation 
		const C3DFVector& operator *() const;

		/// return pointer to current value of the transformation 
		const C3DFVector  *operator ->() const;

		/// return the current position in 3D space 
 		const C3DBounds& pos()const; 


		/// @returns the size of the supported domain 
		const C3DBounds& get_size()const; 
		

	private: 
		std::unique_ptr<iterator_impl> m_holder;

		friend EXPORT_3D bool operator == (const C3DTransformation::const_iterator& a, 
						   const C3DTransformation::const_iterator& b); 

	}; 


	using Transformation<C3DImage, C3DInterpolatorFactory>::operator ();

	/**
	   Standard constructor place holder
	 */
	C3DTransformation(const C3DInterpolatorFactory& ipf);

	/**
	   Set the descrition string that was used to create this transformstion 
	   @param s
	 */
	void set_creator_string(const std::string& s); 

	/// @returns the description string used to create this transformations 
	const std::string& get_creator_string()const; 

	/**
	   @returns a newly allocated copy of the actual transformation
	 */
	virtual C3DTransformation *clone() const;

	/**
	   @returns a the inverse transform 
	 */
	virtual C3DTransformation *invert() const = 0;


	/**
	   @returns the start iterator of the transformation that iterates over the grid 
	   of the area the ransformation is defined on 
	 */
	virtual const_iterator begin() const = 0; 

	/**
	   @returns the end iterator of the transformation that iterates over the grid 
	   of the area the ransformation is defined on 
	 */
	
	virtual const_iterator end() const = 0; 


	/**
	   @returns the start iterator of the transformation that iterates over the grid 
	   of the area the ransformation is defined on 
	 */
	virtual const_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) const = 0; 

	/**
	   @returns the end iterator of the transformation that iterates over the grid 
	   of the area the ransformation is defined on 
	 */
	
	virtual const_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const = 0; 

	/**
	   Placeholder for transformations that might need special initializations
	   like the B-spline based transformation
	 */
	virtual void reinit() const;


	/**
	   Transforation upscaling to new image size
	   @param size new size of the transformation
	   @returns shared pointer to upscaled transformation
	 */
	P3DTransformation upscale(const C3DBounds& size) const;

	/**
	   update a transformation by using a vector field
	   \remark this is too specialized and should go away
	 */
	virtual void update(float step, const C3DFVectorfield& a) = 0;

	/**
	   @returns the number of free parameters this transformation provides
	 */
	virtual size_t degrees_of_freedom() const = 0;

	/**
	   set the transformation to be the identity transform
	 */
	virtual void set_identity() = 0;

	/**
	   evaluate the derivative (Jacobian matrix) of the transformation at the given
	   grid coordinates
	   @param x
	   @param y
	   @param z
	   @returns 3x3 matrix of the derivative
	 */
	virtual C3DFMatrix derivative_at(int x, int y, int z) const = 0;

	/**
	   evaluate the derivative (Jacobian matrix) of the transformation at the given
	   coordinates
	   @param x
	   @returns 3x3 matrix of the derivative
	 */
	virtual C3DFMatrix derivative_at(const C3DFVector& x) const = 0;

	/**
	   Translate the input gradient to a vector field in the space of the transformation field
	   \remark this is too specialized and needs to be replaced by something
	 */
	virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const = 0;

	/**
	   @returns the transformation parameters as a flat value array
	 */
	virtual CDoubleVector get_parameters() const = 0;

	/**
	   sets the transformation parameters from a flat value array
	 */
	virtual void set_parameters(const CDoubleVector& params) = 0;

	/**
	   @returns the (approximate) maximum absolute translation of the transformation over the whole domain
	 */
	virtual float get_max_transform() const = 0;

	/**
	   A transformation is defined on [0,X-1]x[0.Y-1]y[0,Z-1]. 
	   @returns the upper boundaries (X,Y,Z) of this range 
	 */
	virtual const C3DBounds& get_size() const = 0;

	/**
	   evaluate the pertuberation of a vectorfield combined with this transformation
	   \param[in,out] v vectorfield to be pertuberated
	   @returns maximum value of the pertuberation
	   \remark this makes only sense for fluid dynamics registration and should be handled elsewhere
	 */
	virtual float pertuberate(C3DFVectorfield& v) const = 0;

	/**
	   @returns the displacement at coordinate x
	   \remark rename the function to something that explains better whats going on
	 */
	virtual C3DFVector apply(const C3DFVector& x) const = 0;

        /**
	   apply the actual transformation to point x
	   @returns transformed point
	 */
	virtual C3DFVector operator () (const C3DFVector& x) const = 0;

	/**
	   Evaluate the Jacobian of the transformation when updated with vector field v by factor delta
	   @returns Jacobian
	   \remark this only is used for fluid dynamics registration and should probably be moved elsewhere
	 */
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const = 0;

	/**
	   Increase the number of coefficients along the axis according to the 
	   maximum given by the c-rate. This is the complemantary step to upscaling the 
	   transformation. 
	   \returns true if an actual refinment was done. 
	 */
	virtual bool refine(); 

	/**
	   Enable some additional debugging.  
	 */
	void set_debug(); 
protected: 
	/// @returns information about the debug state 
	bool get_debug()const; 

private: 
	P3DImage do_transform(const C3DImage& input, const C3DInterpolatorFactory& ipf) const; 

	virtual P3DTransformation do_upscale(const C3DBounds& size) const = 0;

	std::string m_creator_string;  
	virtual C3DTransformation *do_clone() const = 0;
	bool m_debug; 
};


/**
   \ingroup registration 
   \brief Compare two transformation iterators
   @param a
   @param b
   @returns \a true if iterators are not equal, \a false otherwise 
   
*/
EXPORT_3D bool operator != (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b); 


NS_MIA_END

#endif
