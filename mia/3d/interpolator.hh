/*
** Copyrigh (C) 2004 MPI of Human Cognitive and Brain Sience
**                    Gert Wollny <wollny@cbs.mpg.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  The filter routines for splines and omoms is based on code by
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/
  see also:

  [1] M. Unser,
 	"Splines: A Perfect Fit for Signal and Image Processing,"
 	IEEE Signal Processing Magazine, vol. 16, no. 6, pp. 22-38,
 	November 1999.
  [2] M. Unser, A. Aldroubi and M. Eden,
 	"B-Spline Signal Processing: Part I--Theory,"
 	IEEE Transactions on Signal Processing, vol. 41, no. 2, pp. 821-832,
 	February 1993.
  [3]  M. Unser, A. Aldroubi and M. Eden,
 	"B-Spline Signal Processing: Part II--Efficient Design and Applications,"
 	IEEE Transactions on Signal Processing, vol. 41, no. 2, pp. 834-848,
 	February 1993.

*/

#ifndef mia_3d_interpolator_hh
#define mia_3d_interpolator_hh


#include <vector>
#include <mia/core/shared_ptr.hh>
#include <mia/core/interpolator.hh>
#include <mia/3d/3DImage.hh>


NS_MIA_BEGIN


class EXPORT_3D CInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~CInterpolator();
};

/**
   Basic Interpolator type for 3D Data.
 */

template <typename T>
class  EXPORT_3D T3DInterpolator : public  CInterpolator {
public:

	/**
	   \param x location of data value to read
	   \returns interpolated value at location x
	 */
	virtual T operator () (const C3DFVector& x) const = 0;

};

template <class U>
struct coeff_map<T3DVector<U> > {
	typedef T3DVector<U> value_type;
	typedef C3DDVector   coeff_type;
};


/**
   Interpolator that is based on convolution,like b-splines an o-moms.
*/

template <class T>
class EXPORT_3D T3DConvoluteInterpolator: public T3DInterpolator<T> {
public:
	typedef T3DDatafield< typename coeff_map< T >::coeff_type > TCoeff3D;
	/**
	   Create the interpolator from the input data and a given kernel
	   \param data
	   \param kernel
	 */
	T3DConvoluteInterpolator(const T3DDatafield<T>& data, std::shared_ptr<CBSplineKernel > kernel);

	/// Standart constructor for factory prototyping
	~T3DConvoluteInterpolator();

	/**
	   get the interpolated value at a given location \a x
	   \param x
	   \returns the interpolated value
	 */
	T  operator () (const C3DFVector& x) const;

	/// \returns the coefficients 
	const TCoeff3D& get_coefficients() const {
		return m_coeff; 
	}

protected:

	typedef std::vector< typename TCoeff3D::value_type > coeff_vector;
private:

	TCoeff3D m_coeff;
	C3DBounds m_size2;
	std::shared_ptr<CBSplineKernel > m_kernel;
	T m_min;
	T m_max;

 	mutable CBSplineKernel::SCache m_x_cache; 
	mutable CBSplineKernel::SCache m_y_cache; 
	mutable CBSplineKernel::SCache m_z_cache; 
};


/**
   @brief A factory to create interpolators of a given type by providing input data.
   
 */

class EXPORT_3D C3DInterpolatorFactory {
public:

	/**
	   Initialise the factory by providing a interpolator type id and a kernel (if needed)
	   \param type interpolator type id
	   \param kernel spline kernel
	*/
	C3DInterpolatorFactory(EInterpolationFactory type, PBSplineKernel kernel);

	/// Copy constructor
	C3DInterpolatorFactory(const C3DInterpolatorFactory& o);

	/// Assignment operator
	C3DInterpolatorFactory& operator = ( const C3DInterpolatorFactory& o);

	/// \remark why do I need a virtual destructor here?
	virtual ~C3DInterpolatorFactory();

	/**
	   \param src input data
	   \returns an interpolator with the given input data and the predefined interpolation type
	 */
	template <class T>
	T3DInterpolator<T> *create(const T3DDatafield<T>& src) const
		__attribute__ ((warn_unused_result));

	/// @returns the B-spline kernel used for interpolator creation 
	PBSplineKernel get_kernel() const; 
private:
	EInterpolationFactory m_type;
	PBSplineKernel m_kernel;
};


EXPORT_3D C3DInterpolatorFactory *create_3dinterpolation_factory(EInterpolation type)
	__attribute__ ((warn_unused_result));

// implementation

template <class T>
T3DInterpolator<T> *C3DInterpolatorFactory::create(const T3DDatafield<T>& src) const
{
	switch (m_type) {
	case ipf_spline: return new T3DConvoluteInterpolator<T>(src, m_kernel);
	default: throw "C3DInterpolatorFactory::create: Unknown interpolator requested";
	}
	return NULL;
}

/// Pointer type of the 3D interpolation factory 
typedef std::shared_ptr<C3DInterpolatorFactory> P3DInterpolatorFactory;

NS_MIA_END

#endif
