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

#ifndef mia_core_splinekernel_hh
#define mia_core_splinekernel_hh

#include <vector>
#include <memory>
#include <cmath>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/factory.hh>
#include <mia/core/product_base.hh>

NS_MIA_BEGIN


class CSplineBoundaryCondition; 

/**
   \ingroup interpol 
   \brief Base class for all spline based interpolation kernels.  

   The kernel of spline based interpolations that provides the interface to 
   evaluate weights and indices into the coefficient field. 
   
 */
class EXPORT_CORE CSplineKernel : public CProductBase{
public:

	/// helper typedef for plugin handling 
	typedef CSplineKernel plugin_data; 
	/// helper typedef for plugin handling 
	typedef CSplineKernel plugin_type; 

	/// plugin handling type description 
	static const char *type_descr; 

	/// plugin handling data description 
	static const char *data_descr; 

	/// type for the weight vector 
	typedef std::vector<double> VWeight; 

	/// type for the index vector 
	typedef std::vector<int> VIndex; 

	/**
	   A struture to cache B-spline weights and indices 
	 */
	struct SCache {
		/**
		   Initialize the case by setting the index and weight array size and 
		   mirror boundary sizes 
		   @param s support size of the kernel which equals the size of the index and weight arrays 
		   @param bc Boundary conditions to be used, this is a reference to the parent interpolator object 
		   @param am set to true if indices always need to be set 
		 */
		SCache(size_t s, const CSplineBoundaryCondition& bc, bool am); 

		/**
		   reset the parameters of the cache
		 */
		void reset(); 

		/** last location the B-spline was evaluated at. This  value is initialized to NaN
		    to make sure we 
		*/
		double x; 
		
		/// last start index the B-spline was evaluated for 
		int start_idx; 

		/// last possible start index 
		int index_limit; 
		
		/// cached weights 
		VWeight weights; 
		
		/// cached indices 
		VIndex index; 

		/// the boundary condition to be applied
		const CSplineBoundaryCondition& boundary_condition; 
		
		/// store whether indices were mirrored 
		bool is_flat; 

		/// always use mirror
		bool never_flat; 
	}; 

	/**
	   @param degree of the spline
	   @param shift location shift of the input coordinate to obtain the proper support
	   @param type interpolation type 
	   @remark why to I give the type, it should alwas be bspline
	 */
	CSplineKernel(int degree, double shift, EInterpolation type);

	/**
	   The virtual destructor is just here to avoid some warning
	 */
	virtual ~CSplineKernel();

	/**
	    This operator evaluates the weights and indices of the interpolation
	    \param x input coordinate
	    \param[out] weight weights of the interpolation
	    \param[out] index indices corresponding to the weights, No boundary conditions are applied. 
	 */
	void operator () (double x, VWeight& weight, VIndex& index)const;

	/**
	   This operator evaluates the weights and indices of the interpolation at a given position. 
	   The boundary conditions given in the value cache are applied.  
	   The result is stored in the return value cache and this cache is only updated if neccesary 
	   \param x location for which to evaluate weights and indices 
	   \param[in,out] cache storage for returned value
	   
	 */
	void operator () (double x, SCache& cache)const;

	/**
	   This operator evaluates the weights and indices of the interpolation at a given position. 
	   The boundary conditions given in the value cache are applied.  
	   The result is stored in the return value cache and this cache is only updated if neccesary. 
	   The index set is always fully set. 

	   \param x location for which to evaluate weights and indices 
	   \param[in,out] cache storage for returned value
	   
	 */
	void get_cached(double x, SCache& cache)const;

	/**
	   This operator evaluates the weights and indices of the interpolation at a given position. 
	   The boundary conditions given in the value cache are applied.  
	   The result is stored in the return value cache and this cache is always updated.
	   The index set will only be set correctly if the boundary conditions had to be applied.
	   \param x location for which to evaluate weights and indices 
	   \param[in,out] cache storage for returned value
	   
	 */
	void get_uncached(double x, SCache& cache)const;

	/**
	   Evaluate the first order derivative weights of the B-Spline at the given position
	   @param x location where to evaluate the derivative 
	   @param[out] weight the interpolation weights are stored here 
	   @param[out] index the interpolation coefficient intices are stored here 
	*/
	
	void derivative(double x, VWeight& weight, VIndex& index)const;
	
        /**
	   Evaluate the derivative weights of the B-Spline at the given position
	   @param x location where to evaluate the derivative 
	   @param[out] weight the interpolation weights are stored here 
	   @param[out] index the interpolation coefficient intices are stored here 
	   @param order order of the derivative to be evaluated 
	 */
	
	void derivative(double x, VWeight& weight, VIndex& index, int order)const;
	
        /**
	   Evaluate the indices of the coefficients that would be used for interpolation 
	   @param x location where to evaluate
	   @param[out] index the interpolation coefficient indices are stored here 
	   @returns start index 
	 */
	int get_indices(double x, VIndex& index) const;

	/** evaluate the weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \remark why is this not a private function? 
	 */
	virtual void get_weights(double x, VWeight& weight) const = 0;

	/** evaluate the first order derivative weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \remark why is this not a private function? 
	 */
	virtual void get_derivative_weights(double x, VWeight& weight) const = 0;

	/** evaluate the first order derivative weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \param order derivative order 
	    \remark why is this not a private function? 
	 */
	virtual void get_derivative_weights(double x, VWeight& weight, int order) const = 0;


	/**
	   Evaluate the weight at a single position 
	   \param x location to evaluate B-spline at 
	   \param order order of derivative to be evaluated (0 = value) 
	   \returns B-spline weight 
	 */
	virtual double get_weight_at(double x, int order) const;
	
	/// @returns the type of this interpolator 
	EInterpolation get_type() const; 

	/**
	   \returns the poles of the spline used for pre-filtering 
	 */
	const std::vector<double>& get_poles() const;

	/**
	   \returns the size of the support of this kernel
	*/
	size_t size() const;


	/// \returns the radius around zero where the B-spline does not evaluate to zero 
	double get_nonzero_radius() const;

	///\returns the number of the neighboring grind points used on each side of the center 
	int get_active_halfrange()const; 

	/**
	   Evaluate the first coefficient index and the weights vor B-spline interpolation
	   \param x location to evaluate the spline at 
	   \param[out] weights weights of the B-spline 
	   \returns first index into the coefficient field to be used - note this may be a negiative value 
	*/
	int get_start_idx_and_value_weights(double x, VWeight& weights) const; 

	/**
	   Evaluate the first coefficient index and the derivative weights vor B-spline interpolation
	   \param x location to evaluate the spline at 
	   \param[out] weights weights of the B-spline 
	   \returns first index into the coefficient field to be used - note this may be a negiative value 
	*/
	int get_start_idx_and_derivative_weights(double x, VWeight& weights) const; 

protected:
	/** add a pole to the list of poles
	    \param x
	*/
	void add_pole(double x);

private:
	int get_start_idx(double x) const; 

	/**
	   Helper function to fill the array index with consecutive values starting with i 
	 */
	void fill_index(short i, VIndex& index) const; 

	
	size_t m_half_degree;
	
	double m_shift;
	
	std::vector<double> m_poles;

	size_t m_support_size;
	
	EInterpolation m_type; 
	std::vector<short> m_indices;
	
};

/**
   \ingroup interpol 
   Pointer type for spline kernels 
*/
typedef std::shared_ptr<CSplineKernel> PSplineKernel;

/// base plugin for spline kernels
typedef TFactory<CSplineKernel> CSplineKernelPlugin; 


template<>  const char * const 
TPluginHandler<TFactory<CSplineKernel>>::m_help; 
/**
   \ingroup interpol 
   Plugin handler for the creation of spline kernels 
*/
typedef THandlerSingleton<TFactoryPluginHandler<CSplineKernelPlugin> > CSplineKernelPluginHandler;

/**
   \ingroup interpol 
   Create a spline kernel by using the provided plug-ins 
   \param descr the spline kernel description (e.g. "bspline:d=3" for a B-spline kernel of degree 3)
   \returns the spline kernel stored in a shared pointer PSplineKernel. 
*/

inline PSplineKernel produce_spline_kernel(const std::string& descr) 
{
	return CSplineKernelPluginHandler::instance().produce(descr); 
}

FACTORY_TRAIT(CSplineKernelPluginHandler); 

/**
   @cond INTERNAL 
   @ingroup traits 
   @brief helper to determain the besr accuracy scalar type that type T can be converted to 
*/
template <>
struct __cache_policy<CSplineKernelPlugin> {
	static  bool apply() {
		return true; 
	}
}; 

template <typename T>
struct max_hold_type {
	typedef double type;
};

/**
   @ingroup traits 
   @brief trait to obtaine the type the spline coefficients are best represented in
*/
template <class T>
struct coeff_map {
	typedef T     value_type;
	typedef double coeff_type;
};
/**
   @ingroup traits 
   @brief specialization that marks that float should not be translated to double float
*/
template <>
struct coeff_map<float> {
	typedef float value_type;
	typedef float coeff_type;
};

/// @endcond 

inline size_t CSplineKernel::size()const
{
	return m_support_size;
}


/**
   Approximate integration of a B-Spline kernel product 
   \f[ 
   \int_{x_0}^L \frac{\partial^{d_1}}{\partial x^{d_1}} \beta(x - s_1)  
        \frac{\partial^{d_2}}{\partial x^{d_2}} \beta(x - s_2) dx
   \f]
   using the Simpson integration.  
   @param spline kernel \f$\beta\f$  
   @param s1 
   @param s2
   @param d1 
   @param d2
   @param n number of integration intervals 
   @param x0 start of interval
   @param L end of interval 
   @returns value of integral 
   
*/
double  EXPORT_CORE integrate2(const CSplineKernel& spline, double s1, double s2, int d1, int d2, double n, double x0, double L);


NS_MIA_END

#endif
