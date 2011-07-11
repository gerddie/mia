/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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
 */

/*
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

#ifndef mia_2d_interpolator_hh
#define mia_2d_interpolator_hh

#include <vector>
#include <mia/core/splinekernel.hh>
#include <mia/2d/2DImage.hh>


NS_MIA_BEGIN

template <typename T>
struct max_hold_type<T2DVector<T> > {
	typedef T2DVector<double> type;
};

class EXPORT_2D C2DInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~C2DInterpolator();
};

/**
   \brief The base class for 2D interpolators 

   Basic Interpolator type for 2D Data.
*/
template <typename T>
class  EXPORT_2D T2DInterpolator : public  C2DInterpolator {
public:

	/**
	   \param x location of data value to read
	   \returns interpolated value at location x
	 */
	virtual T operator () (const C2DFVector& x) const = 0;
	
	/**
	   \param x location of data value to read
	   \returns interpolated gradient at location x
	*/
	virtual T2DVector<T> derivative_at(const C2DFVector& x) const = 0;

};

template <class U>
struct coeff_map<T2DVector<U> > {
	typedef T2DVector<U> value_type;
	typedef C2DDVector   coeff_type;
};

/**
   \brief The base class for 2D interpolators that use some kind of spacial convolution 

   This class provides the interface for 2D interpolation based on some kind of 
   spacial convolution, like e.g. by using B-splines. 
*/
template <class T>
class EXPORT_2D T2DConvoluteInterpolator: public T2DInterpolator<T> {
public:
	/**
	   Constructor for the interpolator. The input data is pre-filtered in order to 
	   ensure that the interpolation at grid points returns the original data values. 
	   \param image input data to base th einterpolation on 
	   \param kernel the B-spline kernel to be used. 
	*/
	T2DConvoluteInterpolator(const T2DDatafield<T>& image, PSplineKernel kernel);

	T2DConvoluteInterpolator(const T2DDatafield<T>& image, PSplineKernel kernel, 
				 PBoundaryCondition xbc, PBoundaryCondition ybc);

	~T2DConvoluteInterpolator();

	
	T  operator () (const C2DFVector& x) const;

	/**
	   Evaluate the interolation based on the given weights and coefficient indices.
	   \param xweight B-spline weights in x-direction
	   \param yweight B-spline weights in y-direction
	   \param xindex indices into the coefficient field in x-direction
	   \param yindex indices into the coefficient field in y-direction
	*/
	T evaluate(const std::vector<double>& xweight, const std::vector<double>& yweight,
		   const std::vector<int>&    xindex,  const std::vector<int>&    yindex) const; 

	
	/**
	   Evaluate the first order derivative on the given coordinate 
	   \param x location 
	   \returns teh drivatives in all coordinate directions as 2D vector 
	 */
	T2DVector<T> derivative_at(const C2DFVector& x) const;

	/** Data type of the field that holds the cofficients. Essentially, it uses 
	    the coeff_map template to translate whatever T is composed of to something that 
	    is composed of double float values to provide the required accuracy for interpolation. 
	 */
	typedef T2DDatafield< typename coeff_map< T >::coeff_type > TCoeff2D;

	/**
	   \returns the current coefficient field 
	 */
	const TCoeff2D& get_coefficients() const __attribute__((deprecated)); 

protected:
	/// helper class for the coefficient field 
	typedef std::vector< typename TCoeff2D::value_type > coeff_vector;
private:
	
	void prefilter(const T2DDatafield<T>& image); 

	typename TCoeff2D::value_type evaluate() const;

	TCoeff2D m_coeff;
	C2DBounds m_size2;
	PSplineKernel m_kernel;
	PBoundaryCondition m_x_boundary; 
	PBoundaryCondition m_y_boundary; 
	T m_min;
	T m_max;

	mutable std::vector<int> m_x_index; 
	mutable std::vector<int> m_y_index; 
	mutable std::vector<double> m_x_weight; 
	mutable std::vector<double> m_y_weight; 
	mutable CSplineKernel::SCache m_x_cache; 
	mutable CSplineKernel::SCache m_y_cache; 

	

};

/**
   \brief The factory to create an interpolator from some input data 
*/

class EXPORT_2D C2DInterpolatorFactory {
public:

	/**
	   Construct the factory by giving the interpolator type and the 
	   kernel used for interpolation. 
	   \param type 
	   \param kernel
	 */
	C2DInterpolatorFactory(PSplineKernel kernel, PBoundaryCondition xbc, PBoundaryCondition ybc);

	/// Copy constructor 
	C2DInterpolatorFactory(const C2DInterpolatorFactory& o);

	/// Assignment operator 
	C2DInterpolatorFactory& operator = ( const C2DInterpolatorFactory& o);

	virtual ~C2DInterpolatorFactory();

	/**
	   Interpolator creation function 
	   \tparam pixel data type - can be anything that cann be added to itself and 
	   multiplied by a double scalar. The class T must also define the coeff_map trait. 
	   \param src input data to interpolate 
	   \returns the requested interpolator
	 */

	template <class T>
	T2DInterpolator<T> *create(const T2DDatafield<T>& src) const
		__attribute__ ((warn_unused_result));


	/**
	   \returns raw pointer to the interpolation kernel. 
	 */
	const CSplineKernel* get_kernel() const;

private:
	PSplineKernel m_kernel;
	PBoundaryCondition m_xbc;
	PBoundaryCondition m_ybc;
};

/// Pointer type for the 2D interpolationfactory 
typedef std::shared_ptr<C2DInterpolatorFactory > P2DInterpolatorFactory;


/**
   create a 2D interpolation factory of a certain interpolation type 
*/
C2DInterpolatorFactory EXPORT_2D  *create_2dinterpolation_factory(EInterpolation type, EBoundaryConditions bc)
	__attribute__ ((warn_unused_result));

// implementation

template <class T>
T2DInterpolator<T> *C2DInterpolatorFactory::create(const T2DDatafield<T>& src) const
{
	return new T2DConvoluteInterpolator<T>(src, m_kernel, m_xbc, m_ybc);
}

NS_MIA_END

#endif
