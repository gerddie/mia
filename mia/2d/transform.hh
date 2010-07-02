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

#ifndef mia_2d_transform_hh
#define mia_2d_transform_hh

#include <gsl++/vector.hh>
#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/2d/matrix.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/interpolator.hh>



NS_MIA_BEGIN


class C2DTransformation;
typedef SHARED_PTR(C2DTransformation) P2DTransformation;

/**
   generic base class for 2D transformations.
*/

class EXPORT_2D C2DTransformation: public Transformation<C2DImage, C2DInterpolatorFactory> {
public:
	typedef C2DImage Data;
	typedef C2DInterpolatorFactory Interpolator;

	using Transformation<C2DImage, C2DInterpolatorFactory>::operator ();

	/**
	   Standard constructor place holder
	 */
	C2DTransformation();

	/**
	   \returns a newly allocated copy of the actual transformation
	 */
	virtual C2DTransformation *clone() const = 0;

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
	virtual P2DTransformation upscale(const C2DBounds& size) const = 0;

	/**
	   concat a transformation,
	   \param a the transformation to be added
	 */
	virtual void add(const C2DTransformation& a) = 0;

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
	virtual void translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const = 0;

	/**
	   \returns the transformation parameters as a flat vector field
	 */
	virtual gsl::DoubleVector get_parameters() const = 0;

	/**
	   sets the transformation parameters from a flat vector field
	 */
	virtual void set_parameters(const gsl::DoubleVector& params) = 0;

	/**
	   \returns the (approximate) maximum absolute translation of the transformation over the whole domain
	 */
	virtual float get_max_transform() const = 0;

	/**
	   \returns the upper range of where the transformation is defined
	 */
	virtual const C2DBounds& get_size() const = 0;

	/**
	   evaluate the pertuberation of a vectorfield combined with this transformation
	   \retval v vectorfield to be pertuberated
	   \returns maximum value of the pertuberation
	   \remark this makes only sense for fluid dynamics registration and should be handled elsewhere
	 */
	virtual float pertuberate(C2DFVectorfield& v) const = 0;

	/**
	   \returns the displacement at coordinate x
	   \remark rename the function to something that explains better whats going on
	 */
	virtual C2DFVector apply(const C2DFVector& x) const = 0;

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
	   \remark placeholder
	 */
	virtual float divergence() const = 0;

	/**
	   \remark placeholder
	 */
	virtual float curl() const = 0;

};

/**
   Functor to evaluate a transformed image by applying a given transformation
   and using the providedinterpolator type
*/

template <typename Transform>
struct C2DTransform : public TFilter<P2DImage> {
	C2DTransform(const C2DInterpolatorFactory& ipf, const Transform& trans):
		_M_ipf(ipf),
		_M_trans(trans){
	}
	template <typename T>
	P2DImage operator ()(const T2DImage<T>& image) const {
		T2DImage<T> *timage = new T2DImage<T>(image.get_size());

		std::auto_ptr<T2DInterpolator<T> > interp(_M_ipf.create(image.data()));

		typename T2DImage<T>::iterator r = timage->begin();
		typename Transform::const_iterator v = _M_trans.begin();

		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(*v);
			}

		return P2DImage(timage);
	}
private:
	const C2DInterpolatorFactory& _M_ipf;
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
P2DImage transform2d(const C2DImage& image, const C2DInterpolatorFactory& ipf, const Transform& trans)
{
	return mia::filter(C2DTransform<Transform>(ipf, trans), image);
}

NS_MIA_END

#endif
