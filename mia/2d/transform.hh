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

#ifndef mia_2d_transform_hh
#define mia_2d_transform_hh

#include <iterator>
#include <memory>
#include <ostream>

#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/2d/matrix.hh>
#include <mia/2d/image.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN


/**
   @ingroup registration
   \brief This is the generic base class for 2D transformations.

   This class defines the generic interface for a 2D transformation.
   Most methods are pure abstract and need to be implemented by a "real" transformation.
   Actual implementations are provided as plug-ins.
*/
class EXPORT_2D C2DTransformation: public Transformation<C2DImage, C2DInterpolatorFactory>
{
public:
       /// generic name for the data used by this transformation
       typedef C2DImage Data;

       /// generic name for the size type used by this transformation
       typedef C2DBounds Size;

       /// generic name for the vector type used by this transformation
       typedef C2DFVector Vector;

       /// generic name for the interpolation factory used by this transformation
       typedef C2DInterpolatorFactory InterpolatorFactory;

       /// generic name for this transformation type
       typedef C2DTransformation type;

       /// pointer type of this transformation
       typedef std::shared_ptr<C2DTransformation > Pointer;

       /// plug-in search path element "type"
       static const char *data_descr;

       /// plug-in search path element "data"
       static const char *dim_descr;
protected:

       /**
          @brief Base of the implementation of an iterator over the transformation domain
          This iterator takes care of iterating over the transformation range. X is the fastest
          changing index.
          Some methods are abstract and must be defined in derived classes by using the
          specific transformation model.
       */
       class iterator_impl
       {
       public:
              iterator_impl();

              /**
                 Constructor to initialize the iterator at a certain point
                 @param pos current position of the iterator
                 @param size defines the grid of the domain of the iterator as
                    \f$[pos.x, size.x-1] \times  [pos.y, size.y-1]\f$

               */
              iterator_impl(const C2DBounds& pos, const C2DBounds& size);

              /// increment the position
              void increment();

              /// fast advance the position
              void advance(unsigned int delta);

              /// @returns the current value of the transformation iterated over
              const C2DFVector&  get_value() const;

              /// @returns a dynamically allocated copy of the iterator
              virtual iterator_impl *clone() const __attribute__((warn_unused_result))  = 0;

              /**
                 Compare the iterator to another one
                 @param other
                 @returns true if the positions are equal or both are at the end of the range
                 @remark no test is run whether both iterator belong to the same transformation
               */
              bool operator == (const iterator_impl& other) const;

              /// @returns the cutrrent grid position of the iterator
              const C2DBounds& get_pos()const;

              /// @returns the domain size of the underlying transformation
              const C2DBounds& get_size()const;

              /**
                 Print some information about the iteratorto an output stream,
                 mostely used for debugging purpouses
                 @param os
               */
              void print(std::ostream& os) const;
       private:
              virtual const C2DFVector& do_get_value()const = 0;
              virtual void do_y_increment() = 0;
              virtual void do_x_increment() = 0;

              C2DBounds m_pos;
              C2DBounds m_size;

       };
public:
       /**
          Iterator to iterator over the grid points of the supported range
          \todo this iterator shouldn't be here, but should be a separate
          class not bound to the 2D transformation
        */

       class const_iterator : public std::forward_iterator_tag
       {
       public:

              /// provide the STL with some typedef fro traits
              typedef std::forward_iterator_tag iterator_category;

              /// generic name for the value type of this iterator
              typedef C2DFVector value_type;

              /// generic name for the difference type of this iterator
              typedef size_t difference_type;

              /// generic name for the pointer type of this iterator
              typedef C2DFVector *pointer;

              /// generic name for the reference type of this iterator
              typedef C2DFVector& reference;

              /**
                 Standard constructor
                 \remark constructed like this the iterator is not usable.
               */
              const_iterator();

              /**
                 Constructor to be initialized with "a real implementation"
                 \param holder is the implementation that does all the real work
                 and depends on the transformation
               */
              const_iterator(iterator_impl *holder);


              /** Assignment operator implemnts the deep copy of the holder
                  The pointer to the holder is not shared but cloned.
              */
              const_iterator& operator = (const const_iterator& other);

              /** Copy Constructor impelemnts the deep copy of the holder
                  The pointer to the holder is not shared but cloned.
               */
              const_iterator(const const_iterator& other);


              /**
                 Prefix increment
               */
              const_iterator& operator ++();

              /**
                 Postfix increment
               */
              const_iterator operator ++(int);

              /**
                 Advance a certain amount of steps. This implementation
                 will ususlly be fatser than the generic std::advance function, since
                 for forward_iterators std::advance calls "++" delta times
                 @param delta
              */
              const_iterator& operator += (unsigned int delta);

              /// @returns the current value of the transformation
              const C2DFVector& operator *() const;

              /// @returns the pointer version of the current value of the transformation
              const C2DFVector  *operator ->() const;

              const C2DBounds& pos() const;

              const C2DBounds& get_size() const;

              /** Print the current position and value to an output stream
                  \param os
              */
              void print(std::ostream& os) const;
       private:
              std::unique_ptr<iterator_impl> m_holder;

              friend EXPORT_2D bool operator == (const C2DTransformation::const_iterator& a,
                                                 const C2DTransformation::const_iterator& b);

       };

       using Transformation<C2DImage, C2DInterpolatorFactory>::operator ();

       /**
          Standard constructor place holder
        */
       C2DTransformation(const C2DInterpolatorFactory& ipf);

       /**
          Set the descrition string that was used to create this transformstion
          @param s
        */
       void set_creator_string(const std::string& s);

       /// \returns the description string used to create this transformations
       const std::string& get_creator_string()const;

       /**
          \returns a newly allocated copy of the actual transformation
        */
       virtual C2DTransformation *clone() const __attribute__((warn_unused_result));

       /**
          \returns a the inverse transform
        */
       virtual C2DTransformation *invert() const __attribute__((warn_unused_result))  = 0;

       /**
          \returns the start iterator of the transformation that iterates over the grid
          of the area the ransformation is defined on
        */
       virtual const_iterator begin() const = 0;

       /**
          \returns the end iterator of the transformation that iterates over the grid
          of the area the ransformation is defined on
        */

       virtual const_iterator end() const = 0;

       /**
          Transforation upscaling to new image size
          \param size new size of the transformation
          \returns shared pointer to upscaled transformation
        */
       Pointer upscale(const C2DBounds& size) const;

       /**
          update a transformation by using a vector field
          \remark this is too specialized and should go away
        */
       virtual void update(float step, const C2DFVectorfield& a) = 0;

       /**
          \returns the number of free parameters this transformation provides
        */
       virtual size_t degrees_of_freedom() const = 0;

       /**
          set the transformation to be the identity transform
        */
       virtual void set_identity() = 0;

       /**
          evaluate the derivative (Jacobian matrix) of the transformation at the given
          coordinate
          \param x
          \returns 2x2 matrix of the derivative
        */
       virtual C2DFMatrix derivative_at(const C2DFVector& x) const = 0;

       /**
          evaluate the derivative (Jacobian matrix) of the transformation at the given
          grid coordinates
          \param x
          \param y
          \returns 2x2 matrix of the derivative
        */
       virtual C2DFMatrix derivative_at(int x, int y) const = 0;

       /**
          Translate the input gradient to a vector field in the space of the transformation field
          \remark this is too specialized and needs to be replaced by something
        */
       virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const = 0;

       /**
          \returns the transformation parameters as a flat value array
        */
       virtual CDoubleVector get_parameters() const = 0;

       /**
          sets the transformation parameters from a flat value array
        */
       virtual void set_parameters(const CDoubleVector& params) = 0;

       /**
          \returns the (approximate) maximum absolute translation of the transformation over the whole domain
        */
       virtual float get_max_transform() const = 0;

       /**
          A transformation is defined on [0,X-1]x[0.Y-1].
          \returns the upper boundaries (X,Y) of this range
        */
       virtual const C2DBounds& get_size() const = 0;

       /**
          evaluate the pertuberation of a vectorfield combined with this transformation
          \param[in,out] v vectorfield to be pertuberated
          \returns maximum value of the pertuberation
          \remark this makes only sense for fluid dynamics registration and should be handled elsewhere
        */
       virtual float pertuberate(C2DFVectorfield& v) const = 0;

       /**
          \returns the displacement at coordinate x
          \remark rename the function to something that explains better whats going on
        */
       virtual C2DFVector apply(const C2DFVector& x) const __attribute__((deprecated))
       {
              return get_displacement_at(x);
       }

       /**
          \returns the displacement at coordinate x
       */
       virtual C2DFVector get_displacement_at(const C2DFVector& x) const = 0;
       /**
         apply the actual transformation to point x
         \returns transformed point
       */
       virtual C2DFVector operator () (const C2DFVector& x) const = 0;

       /**
          Evaluate the Jacobian of the transformation when updated with vector field v by factor delta
          \returns Jacobian
          \remark this only is used for fluid dynamics registration and should probably be moved elsewhere
        */
       virtual float get_jacobian(const C2DFVectorfield& v, float delta) const = 0;


       /**
          If applicaple the transformation model is refined (e.g. splines
          are converted to a denser coefficient distribution.
          \returns \a true if refinement was applied, and \a false otherwise
        */
       virtual bool refine();

       /* Attributes */
       /**
          This attribute defines the voxel spacing of the input data of this transform.
        */
       static constexpr const char *input_spacing_attr = "in-pixel-spacing";

       /**
          This attribute defines the output voxel spacing of this transform.
        */
       static constexpr const char *output_spacing_attr = "out-pixel-spacing";


       /**
          \returns the minimal image size that makes sense for the transformation.
          Usually this is (1,1), but for spline based transformation the image must
          be larger.
        */
       virtual C2DBounds get_minimal_supported_image_size() const;

private:

       virtual Pointer do_upscale(const C2DBounds& size) const = 0;

       std::string m_creator_string;
       virtual C2DTransformation *do_clone() const __attribute__((warn_unused_result)) = 0;


       P2DImage do_transform(const C2DImage& input, const C2DInterpolatorFactory& ipf) const;

};

/**
   @ingroup registration
   \brief  Pointer type for the 2D transformation
*/
typedef C2DTransformation::Pointer P2DTransformation;



/**
   @ingroup registration
   \brief Move an 2D transformation iterator forward by using its provided += operator
   don't use a reference to the iterator, because we use the created copy as result
*/
inline C2DTransformation::const_iterator operator + (C2DTransformation::const_iterator i, size_t delta)
{
       i += delta;
       return i;
}

/**
   Print information about the iterator i  to stream os (for debugging)
   \param os
   \param i
   \returns os
 */
inline std::ostream& operator << (std::ostream& os,
                                  const C2DTransformation::const_iterator& i)
{
       i.print(os);
       return os;
}

/**
   @ingroup registration
   \brief Compare two transformation iterators
   \param a
   \param b
   \returns \a true if iterators are not equal, \a false otherwise

*/
EXPORT_2D bool operator != (const C2DTransformation::const_iterator& a,
                            const C2DTransformation::const_iterator& b);


NS_MIA_END

#endif
