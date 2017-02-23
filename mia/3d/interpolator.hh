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

#ifndef mia_3d_interpolator_hh
#define mia_3d_interpolator_hh


#include <vector>
#include <mia/core/splinekernel.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/3d/image.hh>
#include <mia/core/parallel.hh>

NS_MIA_BEGIN


/**
   \ingroup interpol 
   \tparam T data type to be interpolated 
   \brief Basic Interpolator type for 3D Data.
*/
template <typename T>
class  EXPORT_3D T3DInterpolator  {
public:
	
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~T3DInterpolator(){}

	/**
	   \param x location of data value to read
	   \returns interpolated value at location x
	 */
	virtual T operator () (const C3DFVector& x) const = 0;

};

/**
   @cond INTERNAL 
*/
template <class U>
struct coeff_map<T3DVector<U> > {
	typedef T3DVector<U> value_type;
	typedef C3DDVector   coeff_type;
};

struct C3DWeightCache {
	CSplineKernel::SCache x; 
	CSplineKernel::SCache y; 
	CSplineKernel::SCache z; 
	
	C3DWeightCache(int kernel_size, 
		       const CSplineBoundaryCondition& xbc, 
		       const CSplineBoundaryCondition& ybc, 
		       const CSplineBoundaryCondition& zbc); 
}; 
/// @endcond 

/**
   \ingroup interpol 
   \tparam T data type to be interpolated 

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
	   Create the cache structure needed to run the interpolation in a multi-threaded 
	   environment. This function must be called in each thread once. 
	   \returns the cache structure 
	*/
	C3DWeightCache create_cache() const; 

	/**
	   get the interpolated value at a given location \a x
	   \param x
	   \param cache the cache structure created by calling create_cache()- 
	   \returns the interpolated value
	   \remark This method is thread save if the cache structure is thread local 
	*/
	T  operator () (const C3DFVector& x, C3DWeightCache& cache) const;

	/**
	   get the interpolated value at a given location \a x
	   \param x
	   \returns the interpolated value
	   \remark This method is not thread save
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

	typename T3DDatafield<T>::value_type m_min;
	typename T3DDatafield<T>::value_type m_max;
	
	mutable CMutex m_cache_lock; 
 	mutable C3DWeightCache m_cache; 
};


/**
   \ingroup interpol 
      

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
	   \param kernel description of the interpolation kernel
	   \param boundary_conditions boundary condition proto type 
	*/
	C3DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& boundary_conditions);

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
	T3DConvoluteInterpolator<T> *create(const T3DDatafield<T>& src) const
		__attribute__ ((warn_unused_result));
	
	/// @returns the B-spline kernel used for interpolator creation 
	PSplineKernel get_kernel() const; 
private:
	PSplineKernel m_kernel;
	PSplineBoundaryCondition m_xbc; 
	PSplineBoundaryCondition m_ybc; 
	PSplineBoundaryCondition m_zbc; 
};

// implementation

template <class T>
T3DConvoluteInterpolator<T> *C3DInterpolatorFactory::create(const T3DDatafield<T>& src) const
{
	return new T3DConvoluteInterpolator<T>(src, m_kernel, *m_xbc, *m_ybc, *m_zbc);
}

/// Pointer type of the 3D interpolation factory 
typedef std::shared_ptr<C3DInterpolatorFactory> P3DInterpolatorFactory;

NS_MIA_END

#endif
