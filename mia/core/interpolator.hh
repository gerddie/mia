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

#ifndef mia_core_interpolator_hh
#define mia_core_interpolator_hh

#include <vector>
#include <cmath>
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>

#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN

/**
   \file core/interpolator.hh
   \todo - create B-spline kernels of size 0 and 1
   \todo move kernels into plugins 
 */

/// Table to define value-enum relations between interpolator types and strings 
extern EXPORT_CORE TDictMap<EInterpolation> GInterpolatorTable;

/**
   The kernel of spline based interpolations - base type
 */
class EXPORT_CORE CBSplineKernel {
public:
	/// @cond OBSOLETE 
	enum EIntegralType { integral_11, 
			     integral_20, 
			     integral_02, 
			     integral_unknown }; 
	/// @endcond 
	
	/**
	   A struture to cache B-spline weights and indices 
	 */
	struct SCache {
		/**
		   Initialize the case by setting the index and weight array size and 
		   mirror boundary sizes 
		   @param s support size of the kernel which equals the size of the index and weight arrays 
		   @param cs1 size of the 1D coefficient array accessed when interpolating with this spline 
		   @param cs2 2*cs1
		 */
		SCache(size_t s, int cs1, int cs2); 

		/** last location the B-spline was evaluated at. This  value is initialized to NaN
		    to make sure we 
		*/
		double x; 
		
		/// last start index the B-spline was evaluated for 
		int start_idx; 
		
		/// cached weights 
		std::vector<double> weights; 
		
		/// cached indices 
		std::vector<int> index; 

		/// coefficient index range 
		int csize1;
		
		/// double coefficient index range 
		int csize2;

		/// store whether indices were mirrored 
		bool is_mirrored; 
	}; 

	/**
	   @param degree of the spline
	   @param shift location shift of the input coordinate to obtain the proper support
	   @param type interpolation type 
	   @remark why to I give the type, it should alwas be bspline
	 */
	CBSplineKernel(size_t degree, double shift, EInterpolation type);

	/**
	   The virtual destructor is just here to avoid some warning
	 */
	virtual ~CBSplineKernel();

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
};

/// Pointer type for B-Spline kernels 
typedef std::shared_ptr<CBSplineKernel> PBSplineKernel;

/// convolution kernel type enums 
enum ci_type {ci_bspline, /**< B-Spline kernels */
	      ci_omoms};  /**< O-Moms kernels  */

template <typename T>
struct max_hold_type {
	typedef double type;
};

template <class T>
struct coeff_map {
	typedef T     value_type;
	typedef double coeff_type;
};

inline size_t CBSplineKernel::size()const
{
	return m_support_size;
}

/** implements a B-Spline kernel of degree 0 */
class EXPORT_CORE CBSplineKernel0: public  CBSplineKernel{
public:
	CBSplineKernel0();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a B-Spline kernel of degree 0 */
class EXPORT_CORE CBSplineKernel1: public  CBSplineKernel{
public:
	CBSplineKernel1();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};


/** implements a B-Spline kernel of degree 2 */
class EXPORT_CORE CBSplineKernel2: public  CBSplineKernel{
public:
	CBSplineKernel2();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a B-Spline kernel of degree 3 */
class EXPORT_CORE CBSplineKernel3: public  CBSplineKernel{
public:
	CBSplineKernel3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 4 */
class EXPORT_CORE CBSplineKernel4: public  CBSplineKernel{
public:
	CBSplineKernel4();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 5 */
class EXPORT_CORE CBSplineKernel5: public  CBSplineKernel{
public:
	CBSplineKernel5();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a o-Moms kernel of degree 3 */
class EXPORT_CORE CBSplineKernelOMoms3 : public  CBSplineKernel{
public:
	CBSplineKernelOMoms3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

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
double  EXPORT_CORE integrate2(const CBSplineKernel& spline, double s1, double s2, int d1, int d2, double n, double x0, double L);

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
void CBSplineKernel::filter_line(std::vector<C>& coeff)
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
	for_each(coeff.begin(), coeff.end(), FMultBy<C>(lambda));
	
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
C CBSplineKernel::initial_coeff(const std::vector<C>& coeff, double pole)
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
C CBSplineKernel::initial_anti_coeff(const std::vector<C>& coeff, double pole)
{
	return ((pole / (pole * pole - 1.0)) * 
		(pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
}


template <typename InterpolatorFactory> 
InterpolatorFactory *create_interpolator_factory(EInterpolation type) 
{
	std::shared_ptr<CBSplineKernel > kernel;
	EInterpolationFactory iptype = ipf_unknown; 
	switch (type) {
	case ip_nn:
	case ip_bspline0:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel0());
		break;
	case ip_linear:
	case ip_bspline1:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel1());
		break;
	case ip_bspline2:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel2());
		break;
	case ip_bspline3:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel3());
		break;
	case ip_bspline4:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel4());
		break;
	case ip_bspline5:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernel5());
		break;
	case ip_omoms3:
		iptype = ipf_spline;
		kernel.reset(new CBSplineKernelOMoms3());
		break;
	default:
		throw std::invalid_argument("unknown interpolation method");
	}
	return new InterpolatorFactory(iptype, kernel); 

}


NS_MIA_END

#endif
