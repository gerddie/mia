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
 *
 */

#ifndef mia_core_splinekernel_hh
#define mia_core_splinekernel_hh

#include <vector>
#include <memory>
#include <cmath>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/core/factory.hh>
#include <mia/core/product_base.hh>

NS_MIA_BEGIN

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

	/**
	   A struture to cache B-spline weights and indices 
	 */
	struct SCache {
		/**
		   Initialize the case by setting the index and weight array size and 
		   mirror boundary sizes 
		   @param s support size of the kernel which equals the size of the index and weight arrays 
		   @param bc Boundary conditions to be used 
		   @param am set to true if indices always need to be set 
		 */
		SCache(size_t s, PBoundaryCondition bc, bool am); 

		/** last location the B-spline was evaluated at. This  value is initialized to NaN
		    to make sure we 
		*/
		double x; 
		
		/// last start index the B-spline was evaluated for 
		int start_idx; 

		/// last possible start index 
		int index_limit; 
		
		/// cached weights 
		std::vector<double> weights; 
		
		/// cached indices 
		std::vector<int> index; 

		/// the boundary condition to be applied
		PBoundaryCondition boundary_condition; 
		
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
	CSplineKernel(size_t degree, double shift, EInterpolation type);

	/**
	   The virtual destructor is just here to avoid some warning
	 */
	virtual ~CSplineKernel();

	/**
	    This operator evaluates the weights and indices of the interpolation
	    \param x input coordinate
	    \param[out] weight weights of the interpolation
	    \param[out] index indices corresponding to the weights
	 */
	void operator () (double x, std::vector<double>& weight, std::vector<int>& index)const;

	/**
	   This operator evaluates the weights and indices of the interpolation at a given position. 
	   The result is stored in the return value cache and this cache is only updated if neccesary 
	   \param x location for which to evaluate weights and indices 
	   \param[in,out] cache storage for returned value
	   
	 */
	void operator () (double x, SCache& cache)const;

	/**
	   Evaluate the first order derivative weights of the B-Spline at the given position
	   @param x location where to evaluate the derivative 
	   @param[out] weight the interpolation weights are stored here 
	   @param[out] index the interpolation coefficient intices are stored here 
	*/
	
	void derivative(double x, std::vector<double>& weight, std::vector<int>& index)const;
	
        /**
	   Evaluate the derivative weights of the B-Spline at the given position
	   @param x location where to evaluate the derivative 
	   @param[out] weight the interpolation weights are stored here 
	   @param[out] index the interpolation coefficient intices are stored here 
	   @param order order of the derivative to be evaluated 
	 */
	
	void derivative(double x, std::vector<double>& weight, std::vector<int>& index, int order)const;
	
        /**
	   Evaluate the indices of the coefficients that would be used for interpolation 
	   @param x location where to evaluate
	   @param[out] index the interpolation coefficient indices are stored here 
	   @returns start index 
	 */
	int get_indices(double x, std::vector<int>& index) const;

	/** evaluate the weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \remark why is this not a private function? 
	 */
	virtual void get_weights(double x, std::vector<double>& weight) const = 0;

	/** evaluate the first order derivative weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \remark why is this not a private function? 
	 */
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const = 0;

	/** evaluate the first order derivative weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \param[out] weight the weights
	    \param order derivative order 
	    \remark why is this not a private function? 
	 */
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const = 0;


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
	int get_start_idx_and_value_weights(double x, std::vector<double>& weights) const; 

	/**
	   Evaluate the first coefficient index and the derivative weights vor B-spline interpolation
	   \param x location to evaluate the spline at 
	   \param[out] weights weights of the B-spline 
	   \returns first index into the coefficient field to be used - note this may be a negiative value 
	*/
	int get_start_idx_and_derivative_weights(double x, std::vector<double>& weights) const; 

	/**
	   Pre-filter a 1D line of coefficients 
	   \param coeff data to be filtered in-place 
	 */
	template <typename C>
	void filter_line(std::vector<C>& coeff);

protected:
	/** add a pole to the list of poles
	    \param x
	*/
	void add_pole(double x);

private:
	template <typename C>
	C initial_coeff(const std::vector<C>& coeff, double pole);
	
	template <typename C>
	C initial_anti_coeff(const std::vector<C>& coeff, double pole);

	/**
	   Helper function to fill the array index with consecutive values starting with i 
	 */
	void fill_index(int i, std::vector<int>& index) const; 

	
	size_t m_half_degree;
	
	double m_shift;
	
	std::vector<double> m_poles;

	size_t m_support_size;
	
	EInterpolation m_type; 
	std::vector<int> m_indices;
	
};

/**
   \ingroup interpol 
   Pointer type for spline kernels 
*/
typedef std::shared_ptr<CSplineKernel> PSplineKernel;

/// base plugin for spline kernels
typedef TFactory<CSplineKernel> CSplineKernelPlugin; 

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

struct CSplineKernelTestPath {
	CSplineKernelTestPath(); 
}; 


template <typename T>
struct max_hold_type {
	typedef double type;
};

template <class T>
struct coeff_map {
	typedef T     value_type;
	typedef double coeff_type;
};

template <>
struct coeff_map<float> {
	typedef float value_type;
	typedef float coeff_type;
};


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

/**
   Function to apply mirrored boundary conditions so that the indices fit into [0,width)
   @param[in,out] index index array to be adjusted
   @param width width of the supported index range 
   @param width2 2*width to allow repitition over the range
   @returns true if mirroring was applied 
*/
inline bool mirror_boundary_conditions(std::vector<int>& index, int width, 
				       int width2)
{
	// skip the cases where nothing happens
	if (index[0] >= 0 && index[index.size()-1] < width)
		return false; 
	for (size_t k = 0; k < index.size(); k++) {
		int idx = (index[k] < 0) ? -index[k] : index[k]; 
		
		idx = (width == 1) ? (0) : ((idx < width2) ? idx : idx % width2);
		if (width <= idx) {
			idx = width2 - idx;
		}
		index[k] = idx; 
	}
	return true; 
}

template <typename A>
struct FMultBy {
	FMultBy(double f):
		m_f(f)
	{
	}
	void operator()(A& value)
	{
		value *= m_f; 
	}
private: 
	double m_f; 
};


template <typename C>
void CSplineKernel::filter_line(std::vector<C>& coeff)
{
	/* special case required by mirror boundaries */
	if (coeff.size() < 2) {
		return;
	}
	/* compute the overall gain */
	double	lambda = 1.0;
	for (size_t k = 0; k < m_poles.size() ; ++k) {
		lambda  *=  2 - m_poles[k] - 1.0 / m_poles[k];
	}
	
	/* apply the gain */
	for_each(coeff.begin(), coeff.end(), [lambda](C& x) { x *= lambda;});
	
	/* loop over all poles */
	for (size_t k = 0; k < m_poles.size(); ++k) {
		/* causal initialization */
		coeff[0] = initial_coeff(coeff, m_poles[k]);
		
		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			coeff[n] += m_poles[k] * coeff[n - 1];
		}
		
		/* anticausal initialization */
		coeff[coeff.size() - 1] = initial_anti_coeff(coeff, m_poles[k]);
		/* anticausal recursion */
		for (int n = coeff.size() - 2; 0 <= n; n--) {
			coeff[n] = m_poles[k] * (coeff[n + 1] - coeff[n]);
		}
	}
}

template <typename C>
C CSplineKernel::initial_coeff(const std::vector<C>& coeff, double pole)
{
	
	/* full loop */
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	C sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n <= coeff.size()  - 2L; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	
	return(sum / (1.0 - zn * zn));
}

template <typename C>
C CSplineKernel::initial_anti_coeff(const std::vector<C>& coeff, double pole)
{
	return ((pole / (pole * pole - 1.0)) * 
		(pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
}

NS_MIA_END

#endif
