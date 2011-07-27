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

#ifndef mia_1d_interpolator_hh
#define mia_1d_interpolator_hh


#include <vector>

#include <mia/core/defines.hh>
#include <mia/core/splinekernel.hh>
#include <mia/core/boundary_conditions.hh>


NS_MIA_BEGIN

/**
   \ingroup interpol 
   \brief Basic Interpolator type for 1D Data.
   
   \remark Why do we need this? 
*/

class EXPORT_CORE C1DInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~C1DInterpolator();
};


/**
   \ingroup interpol 

   \brief Interpolator base class providing the full interface 
   
   Basic Interpolator type for 1D Data.
   \tparam T data type to be interpolated over 
 */

template <typename T>
class  EXPORT_CORE T1DInterpolator : public  C1DInterpolator {
public:

	/**
	   \param x location of data value to read
	   \returns interpolated value at location x
	 */
	virtual T operator () (const double& x) const = 0;

	/**
	   interface to evaluate the derivative of the spline defined function 
	   @param x 
	   @returns interpolated approximate derivative at x 
	 */
	virtual typename coeff_map<T>::coeff_type derivative_at (const double& x) const = 0;


};

/** 
   \ingroup interpol 

    \brief Interpolator that uses some kind of spaciel kernel. 
    
    Base type for interpolators that work with some kind of convolution  
    \remark currently all interpolators are like this ... 
*/
template <class T>
class EXPORT_CORE T1DConvoluteInterpolator: public T1DInterpolator<T> {
public:
	/**
	   Construtor to prefilter the input for proper interpolation 
	   \param data the data used for interpolation 
	   \param kernel the spline kernel used for interpolation 
	   \param boundary_conditions boundary conditions to be applied when interpolating 
	 */
	
	T1DConvoluteInterpolator(const std::vector<T>& data, PSplineKernel kernel, 
				 const CSplineBoundaryCondition& boundary_conditions);
	
	~T1DConvoluteInterpolator();
	
	/**
	   The actual interpolation operator 
	   \param x location to interpolate at 
	   \returns the interpolated value 
	 */
	T  operator () (const double& x) const;

	/**
	   The interpolation funtion for the first order derivative
	   \param x location to interpolate at 
	   \returns the interpolated derivative value 
	 */
	virtual typename coeff_map<T>::coeff_type derivative_at (const double& x) const;

protected:
	/// Type of the coefficients after filtering 
	typedef std::vector< typename coeff_map< T >::coeff_type > TCoeff1D;

	/// vector to hold coefficients 
	typedef TCoeff1D coeff_vector;
private:

	TCoeff1D m_coeff;
	PSplineKernel m_kernel;
	PSplineBoundaryCondition m_boundary_conditions; 
	T m_min;
	T m_max;

	// not thread save!!!
	mutable CSplineKernel::VIndex m_x_index;
	mutable CSplineKernel::VWeight m_x_weight;
};


/** 
   \ingroup interpol 
   \brief Factory class for 1D interpolators 
   
   Factory to create 1D interpolators of a give data type using the given input data.  
*/
class EXPORT_CORE C1DInterpolatorFactory {
public:

	/** Initialize the factory according B-Spline kernel and a boundary condition  
	    @param kernel 
	    @param bc 
	 */
	C1DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& bc);

	/// Copy constructor 
	C1DInterpolatorFactory(const C1DInterpolatorFactory& o);

	/// assignment operator 
	C1DInterpolatorFactory& operator = ( const C1DInterpolatorFactory& o);

	virtual ~C1DInterpolatorFactory();

	/**
	   Create a 1D interpolator from a set of sampes that 
	   returns the same values as the original at grid points 
	   @tparam data type to be interpolated 
	   @param src input data 
	   @returns the interpolator 
	 */
	template <class T>
	T1DInterpolator<T> *create(const std::vector<T>& src) const
		__attribute__ ((warn_unused_result));

	/// @returns the B-spline kernel 
	PSplineKernel get_kernel() const;

private:
	PSplineKernel m_kernel;
	PSplineBoundaryCondition m_bc; 
};

/** 
   \ingroup interpol 
    Pointer type for C1DInterpolatorFactory. 
 */
typedef std::shared_ptr<const C1DInterpolatorFactory > P1DInterpolatorFactory;

/**
   Create an interpolation factory from a type by also allocating the B-spline kernel if 
   neccessary. 
   @todo this should become the work of a plug-in handler 
 */

C1DInterpolatorFactory EXPORT_CORE  *create_1dinterpolation_factory(EInterpolation type, EBoundaryConditions bc) 
	__attribute__ ((warn_unused_result));

// implementation
template <class T>
T1DInterpolator<T> *C1DInterpolatorFactory::create(const std::vector<T>& src) const
{
	return new T1DConvoluteInterpolator<T>(src, m_kernel, *m_bc);
}



template <typename T>
struct __dispatch_min_max {
	static void apply(const T i, T& min, T &max);
};

template <typename I, typename O>
struct __dispatch_copy {
	static void apply(const I& input, O& output);
};

NS_MIA_END

#endif
