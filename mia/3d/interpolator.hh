/* -*- mia-c++ -*- 
**
** Copyrigh (C) 2004-2011 Gert Wollny <gw.fossdev@gmail.com>
**                    
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
#include <mia/core/splinekernel.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/3d/3DImage.hh>


NS_MIA_BEGIN


class EXPORT_3D CInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~CInterpolator();
};

/**
   \brief Basic Interpolator type for 3D Data.
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
   \brief Interpolator that is based on convolution,like b-splines an o-moms.
*/

template <class T>
class EXPORT_3D T3DConvoluteInterpolator: public T3DInterpolator<T> {
public:
	/** type how the coefficients are stored  - this is done to use a higher accuracy 
	    if the input data is not of double floating point precicion */ 
	typedef T3DDatafield< typename coeff_map< T >::coeff_type > TCoeff3D;
	
	/**
	   Create the interpolator from the input data and a given kernel
	   \param data
	   \param kernel
	 */

	T3DConvoluteInterpolator(const T3DDatafield<T>& data, PSplineKernel kernel); 
	
	/**
	   Construtor to prefilter the input for proper interpolation 
	   \param data the data used for interpolation 
	   \param kernel the spline kernel used for interpolation 
	   \param xbc boundary conditions to be applied along the x-axis when interpolating  
	   \param ybc boundary conditions to be applied along the y-axis when interpolating  
	   \param zbc boundary conditions to be applied along the z-axis when interpolating  
	 */


	T3DConvoluteInterpolator(const T3DDatafield<T>& data, PSplineKernel kernel, 
				 const CSplineBoundaryCondition& xbc,  
				 const CSplineBoundaryCondition& ybc, 
				 const CSplineBoundaryCondition& zbc);
	
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
	/// helper class for filtering 
	typedef std::vector< typename TCoeff3D::value_type > coeff_vector;
private:

	void prefilter(const T3DDatafield<T>& image); 

	TCoeff3D m_coeff;
	C3DBounds m_size2;
	PSplineKernel m_kernel;
	PSplineBoundaryCondition m_xbc; 
	PSplineBoundaryCondition m_ybc; 
	PSplineBoundaryCondition m_zbc; 

	T m_min;
	T m_max;

 	mutable CSplineKernel::SCache m_x_cache; 
	mutable CSplineKernel::SCache m_y_cache; 
	mutable CSplineKernel::SCache m_z_cache; 
};


/**
   @brief A factory to create interpolators of a given type by providing input data.
   
 */

class EXPORT_3D C3DInterpolatorFactory {
public:
	

        /**
	   Construct the factory the interpolation  kernel and according boundary conditions 
	   \param kernel description of the interpolation kernel
	   \param boundary_conditions description of the boundary conditions 
	*/
	C3DInterpolatorFactory(const std::string& kernel, const std::string& boundary_conditions);

	/**
	   Construct the factory the interpolation  kernel and according boundary conditions 
	   \param kernel
	   \param xbc boundary conditions along the x-axis 
	   \param ybc boundary conditions along the y-axis 
	   \param zbc boundary conditions along the z-axis 
	 */

	C3DInterpolatorFactory(PSplineKernel kernel, 
			       const CSplineBoundaryCondition&xbc,  
			       const CSplineBoundaryCondition&ybc, 
			       const CSplineBoundaryCondition&zbc);

        /**
	   Construct the factory from an interpolation  kernel and according boundary conditions description
	   \param kernel interpolation kernel
	   \param bc description of the boundary conditions 
	*/

	C3DInterpolatorFactory(PSplineKernel kernel, const std::string& bc); 

	
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
	PSplineKernel get_kernel() const; 
private:
	PSplineKernel m_kernel;
	PSplineBoundaryCondition m_xbc; 
	PSplineBoundaryCondition m_ybc; 
	PSplineBoundaryCondition m_zbc; 
};


EXPORT_3D C3DInterpolatorFactory *create_3dinterpolation_factory(EInterpolation type, EBoundaryConditions bc)
	__attribute__ ((warn_unused_result));

// implementation

template <class T>
T3DInterpolator<T> *C3DInterpolatorFactory::create(const T3DDatafield<T>& src) const
{
	return new T3DConvoluteInterpolator<T>(src, m_kernel, *m_xbc, *m_ybc, *m_zbc);
}

/// Pointer type of the 3D interpolation factory 
typedef std::shared_ptr<C3DInterpolatorFactory> P3DInterpolatorFactory;

NS_MIA_END

#endif
