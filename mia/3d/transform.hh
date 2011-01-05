/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef mia_3d_transform_hh
#define mia_3d_transform_hh

#include <iterator>
#include <memory>

#include <gsl++/vector.hh>
#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/3d/matrix.hh>
#include <mia/3d/3DImage.hh>
#include <mia/3d/interpolator.hh>

NS_MIA_BEGIN


class C3DTransformation;
typedef std::shared_ptr<C3DTransformation > P3DTransformation;

/**
   This is the generic base class for 3D transformations.
   Most methods are pure abstract and need to be implemented by a "real" transformation. 
*/

class EXPORT_3D C3DTransformation: public Transformation<C3DImage, C3DInterpolatorFactory> {
public:
	typedef C3DImage Data;
	typedef C3DInterpolatorFactory Interpolator;
	typedef C3DTransformation type; 
	static const char *type_descr;
protected: 
	class iterator_impl  {
	public: 
		iterator_impl(); 
		iterator_impl(const C3DBounds& pos, const C3DBounds& size); 

		void increment(); 
		const C3DFVector&  get_value() const;
		virtual iterator_impl * clone() const = 0; 
		
		bool operator == (const iterator_impl& other) const; 
		
		const C3DBounds& get_pos()const; 
		const C3DBounds& get_size()const; 
	private:
		virtual const C3DFVector& do_get_value()const = 0; 
		virtual void do_z_increment() = 0; 
		virtual void do_y_increment() = 0; 
		virtual void do_x_increment() = 0; 
		
		C3DBounds _M_pos; 
		C3DBounds _M_size; 

	}; 
public: 
	class const_iterator : public std::forward_iterator_tag {
	public: 

		typedef std::forward_iterator_tag iterator_category; 
		typedef C3DFVector value_type; 
		typedef size_t difference_type; 
		typedef C3DFVector *pointer; 
		typedef C3DFVector& reference; 
		const_iterator(); 
		const_iterator(iterator_impl * holder); 

		const_iterator& operator = (const const_iterator& other); 
		const_iterator(const const_iterator& other); 

		const_iterator& operator ++(); 
		const_iterator operator ++(int); 

		const C3DFVector& operator *() const;
		const C3DFVector  *operator ->() const;

	private: 
		std::unique_ptr<iterator_impl> _M_holder;

		friend EXPORT_3D bool operator == (const C3DTransformation::const_iterator& a, 
						   const C3DTransformation::const_iterator& b); 

	}; 

	using Transformation<C3DImage, C3DInterpolatorFactory>::operator ();

	/**
	   Standard constructor place holder
	 */
	C3DTransformation();

	/**
	   Set the descrition string that was used to create this transformstion 
	   \params s
	 */
	void set_creator_string(const std::string& s); 

	/// \returns the description string used to create this transformations 
	const std::string& get_creator_string()const; 

	/**
	   \returns a newly allocated copy of the actual transformation
	 */
	virtual C3DTransformation *clone() const;

	/**
	   \returns a the inverse transform 
	 */
	virtual C3DTransformation *invert() const = 0;


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
	   Placeholder for transformations that might need special initializations
	   like the B-spline based transformation
	 */
	virtual void reinit() const;

	/**
	   Save the transformation to some file
	   \param filename name of the file to save to
	   \param type file type description
	   \returns \a true if saving was successfull and \a false if not
	 */
	virtual bool save(const std::string& filename, const std::string& type) const = 0;

	/**
	   Transforation upscaling to new image size
	   \param size new size of the transformation
	   \returns shared pointer to upscaled transformation
	 */
	virtual P3DTransformation upscale(const C3DBounds& size) const = 0;

	/**
	   concat a transformation,
	   \param a the transformation to be added
	 */
	virtual void add(const C3DTransformation& a) = 0;

	/**
	   update a transformation by using a vector field
	   \remark this is too specialized and should go away
	 */
	virtual void update(float step, const C3DFVectorfield& a) = 0;

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
	   grid coordinates
	   \param x
	   \param y
	   \returns 2x2 matrix of the derivative
	 */
	virtual C3DFMatrix derivative_at(int x, int y, int z) const = 0;

	/**
	   Translate the input gradient to a vector field in the space of the transformation field
	   \remark this is too specialized and needs to be replaced by something
	 */
	virtual void translate(const C3DFVectorfield& gradient, gsl::DoubleVector& params) const = 0;

	/**
	   \returns the transformation parameters as a flat value array
	 */
	virtual gsl::DoubleVector get_parameters() const = 0;

	/**
	   sets the transformation parameters from a flat value array
	 */
	virtual void set_parameters(const gsl::DoubleVector& params) = 0;

	/**
	   \returns the (approximate) maximum absolute translation of the transformation over the whole domain
	 */
	virtual float get_max_transform() const = 0;

	/**
	   A transformation is defined on [0,X-1]x[0.Y-1]. 
	   \returns the upper boundaries (X,Y) of this range 
	 */
	virtual const C3DBounds& get_size() const = 0;

	/**
	   evaluate the pertuberation of a vectorfield combined with this transformation
	   \retval v vectorfield to be pertuberated
	   \returns maximum value of the pertuberation
	   \remark this makes only sense for fluid dynamics registration and should be handled elsewhere
	 */
	virtual float pertuberate(C3DFVectorfield& v) const = 0;

	/**
	   \returns the displacement at coordinate x
	   \remark rename the function to something that explains better whats going on
	 */
	virtual C3DFVector apply(const C3DFVector& x) const = 0;

        /**
	   apply the actual transformation to point x
	   \returns transformed point
	 */
	virtual C3DFVector operator () (const C3DFVector& x) const = 0;

	/**
	   Evaluate the Jacobian of the transformation when updated with vector field v by factor delta
	   \returns Jacobian
	   \remark this only is used for fluid dynamics registration and should probably be moved elsewhere
	 */
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const = 0;


	/**
	   Evaluate the grad div ^2 + grad rot ^2 value and its gradient for the 
	   transformtion 
	   \param wd weight of the divergence
	   \param wr weight of the rotation 
	   \retval gradient vector to hold the resulting gradient 
	   \returns cost function value 
	 */
	virtual double get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const = 0; 

	/**
	   Evaluate the grad div ^2 + grad rot ^2 value for the transformtion 
	   \param wd weight of the divergence
	   \param wr weight of the rotation 
	   \returns cost function value 
	 */

	virtual double get_divcurl_cost(double wd, double wr) const = 0; 

	/**
	   If applicaple the transformation model is refined (e.g. splines 
	   are converted to a denser coefficient distribution. 
	   \returns \a true if refinement was applied, and \a false otherwise
	 */
	virtual bool refine(); 

private: 


	std::string _M_creator_string;  
	virtual C3DTransformation *do_clone() const = 0;

};


/**
   Compare two transformation iterators
   \param a
   \param b
   \returns \a true if iterators are not equal, \a false otherwise 
   
*/
EXPORT_3D bool operator != (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b); 


/**
   Helper Functor to evaluate a transformed image by applying a given 
   transformation and using the provided interpolator type
*/

template <typename Transform>
struct C3DTransform : public TFilter<P3DImage> {
	C3DTransform(const C3DInterpolatorFactory& ipf, const Transform& trans):
		_M_ipf(ipf),
		_M_trans(trans){
	}
	template <typename T>
	P3DImage operator ()(const T3DImage<T>& image) const {
		T3DImage<T> *timage = new T3DImage<T>(image.get_size());

		std::auto_ptr<T3DInterpolator<T> > interp(_M_ipf.create(image.data()));

		typename T3DImage<T>::iterator r = timage->begin();
		typename Transform::const_iterator v = _M_trans.begin();

		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(*v);
			}

		return P3DImage(timage);
	}
private:
	const C3DInterpolatorFactory& _M_ipf;
	const Transform& _M_trans;
};


/**
   Transform an image by a given transform using the provided interpolation method.
   \param image the image to be transformed
   \param ipf interpolator factory holding the information which interpolation method will be used
   \param trans transformation
   \returns transformed image
 */

template <typename Transform>
P3DImage transform3d(const C3DImage& image, const C3DInterpolatorFactory& ipf, const Transform& trans)
{
	return mia::filter(C3DTransform<Transform>(ipf, trans), image);
}

NS_MIA_END

#endif
