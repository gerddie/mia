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
#include <mia/core/boundary_conditions.hh>
#include <mia/2d/image.hh>


NS_MIA_BEGIN

/**
   @cond INTERNAL
*/
template <typename T>
struct max_hold_type<T2DVector<T>> {
       typedef T2DVector<double> type;


};
/// @endcond

/**
    @cond INTERNAL
*/
template <class U>
struct coeff_map<T2DVector<U>> {
       typedef T2DVector<U> value_type;
       typedef C2DDVector   coeff_type;
};

/// @endcond


/**
   @cond INTERNAL
*/
struct C2DWeightCache {
       CSplineKernel::SCache x;
       CSplineKernel::SCache y;

       C2DWeightCache(int kernel_size,
                      const CSplineBoundaryCondition& xbc,
                      const CSplineBoundaryCondition& ybc);
};
/// @endcond

/**
   \ingroup interpol

   \tparam T data type to be interpolated

   \brief The base class for 2D interpolators that use some kind of spacial convolution

   This class provides the interface for 2D interpolation based on some kind of
   spacial convolution, like e.g. by using B-splines.
*/
template <class T>
class EXPORT_2D T2DInterpolator
{
public:
       /**
          Constructor for the interpolator. The input data is pre-filtered in order to
          ensure that the interpolation at grid points returns the original data values.
          \param data input data to base th einterpolation on
          \param kernel the B-spline kernel to be used.
       */
       T2DInterpolator(const T2DDatafield<T>& data, PSplineKernel kernel);

       /**
          Construtor to prefilter the input for proper interpolation
          \param data the data used for interpolation
          \param kernel the spline kernel used for interpolation
          \param xbc boundary conditions to be applied along the x-axis when interpolating
          \param ybc boundary conditions to be applied along the y-axis when interpolating
        */

       T2DInterpolator(const T2DDatafield<T>& data, PSplineKernel kernel,
                       const CSplineBoundaryCondition& xbc, const CSplineBoundaryCondition& ybc);

       ~T2DInterpolator();

       /**
          Create the cache structure needed to run the interpolation in a multi-threaded
          environment. This function must be called in each thread once.
          \returns the cache structure
       */
       C2DWeightCache create_cache() const;


       /**
          Interpolate at the given input point
          \param x input point
          \returns interpolated value
          \remark this method is not thread save
        */
       T  operator () (const C2DFVector& x) const;


       T  operator () (const C2DFVector& x, C2DWeightCache& cache) const;

       /**
          Evaluate the first order derivative on the given coordinate
          \param x location
          \returns teh drivatives in all coordinate directions as 2D vector
          \remark this method is not thread save
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
       const TCoeff2D& get_coefficients() const;

protected:
       /// helper class for the coefficient field
       typedef std::vector< typename TCoeff2D::value_type > coeff_vector;
private:

       void prefilter(const T2DDatafield<T>& image);

       typename TCoeff2D::value_type evaluate() const;

       TCoeff2D m_coeff;
       C2DBounds m_size2;
       PSplineKernel m_kernel;
       PSplineBoundaryCondition m_x_boundary;
       PSplineBoundaryCondition m_y_boundary;
       typename T2DDatafield<T>::value_type m_min;
       typename T2DDatafield<T>::value_type m_max;

       mutable CMutex m_cache_lock;
       mutable C2DWeightCache m_cache;


};

/**
   \ingroup interpol

   \brief The factory to create an interpolator from some input data
*/

class EXPORT_2D C2DInterpolatorFactory
{
public:
       /**
         Construct the factory the interpolation  kernel and according boundary conditions
         \param kernel description of the interpolation kernel
         \param boundary_conditions description of the boundary conditions
       */
       C2DInterpolatorFactory(const std::string& kernel, const std::string& boundary_conditions);


       /**
         Construct the factory the interpolation  kernel and according boundary conditions
         \param kernel  interpolation kernel
         \param boundary_conditions prototype boundary condition
       */
       C2DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& boundary_conditions);

       /**
         Construct the factory the interpolation  kernel and according boundary conditions
         \param kernel  interpolation kernel
         \param boundary_conditions description of the boundary conditions
       */
       C2DInterpolatorFactory(PSplineKernel kernel, const std::string& boundary_conditions);

       /**
          Construct the factory the interpolation  kernel and according boundary conditions
          \param kernel
          \param xbc boundary conditions along the x-axis
          \param ybc boundary conditions along the y-axis
        */
       C2DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& xbc, const CSplineBoundaryCondition& ybc);

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
       const CSplineKernel *get_kernel() const;

private:
       PSplineKernel m_kernel;
       PSplineBoundaryCondition m_xbc;
       PSplineBoundaryCondition m_ybc;
};

/// Pointer type for the 2D interpolationfactory
typedef std::shared_ptr<C2DInterpolatorFactory > P2DInterpolatorFactory;


// implementation

template <class T>
T2DInterpolator<T> *C2DInterpolatorFactory::create(const T2DDatafield<T>& src) const
{
       return new T2DInterpolator<T>(src, m_kernel, *m_xbc, *m_ybc);
}

NS_MIA_END

#endif
