/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
   \file interpolator.hh
   \todo - create B-spline kernels of size 0 and 1
   \todo - add a index/weight type that can reuse old indices, 
           and maybe even the old weights 
   \todo move kernels into plugins 
 */

extern EXPORT_CORE TDictMap<EInterpolation> GInterpolatorTable;

/**
   The kernel of spline based interpolations - base type
 */
class EXPORT_CORE CBSplineKernel {
public:
	enum EIntegralType { integral_11, 
			     integral_20, 
			     integral_02, 
			     integral_unknown }; 
			     

	struct SCache {
		SCache(size_t s, int cs1, int cs2); 
		double x; 
		int start_idx; 
		std::vector<double> weights; 
		std::vector<int> index; 
		int csize1;
		int csize2;
	}; 

	/**
	   \param degree of the spline
	   \param shift location shift of the input coordinate to obtain the proper support
	 */
	CBSplineKernel(size_t degree, double shift, EInterpolation type);

	/**
	   The virtual destructor is just here to avoid some warning
	 */
	virtual ~CBSplineKernel();

	/**
	    This operator evaluates the weights and indices of the interpolation
	    \param x input coordinate
	    \retval weight weights of the interpolation
	    \retval index indices corresponding to the weights
	 */
	void operator () (double x, std::vector<double>& weight, std::vector<int>& index)const;
	void operator () (double x, SCache& cache)const;
	void derivative(double x, std::vector<double>& weight, std::vector<int>& index)const;
	void derivative(double x, std::vector<double>& weight, std::vector<int>& index, int degree)const;
	int get_indices(double x, std::vector<int>& index) const;

	/** evaluate the weights, this needs to be implemented for a specific spline
	    \param x coordinate
	    \retval weight the weights
	 */
	virtual void get_weights(double x, std::vector<double>& weight) const = 0;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const = 0;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int degree) const = 0;

	virtual double get_weight_at(double x, int degree) const;

	EInterpolation get_type() const; 

	/**
	   \returns the poles of the spline
	 */
	const std::vector<double>& get_poles() const;

	/**
	   \returns the size of the support of this kernel
	*/
	size_t size() const;

	double get_nonzero_radius() const;

	///\returns the number of the neighboring grind points used on one side of the center 
	int get_active_halfrange()const; 


	int get_start_idx_and_value_weights(double x, std::vector<double>& weights) const; 
	int get_start_idx_and_derivative_weights(double x, std::vector<double>& weights) const; 

	template <typename C>
	void filter_line(std::vector<C>& coeff);

protected:
	template <typename C>
	C initial_coeff(const std::vector<C>& coeff, double pole);
	template <typename C>
	C initial_anti_coeff(const std::vector<C>& coeff, double pole);

	/** add a pole to the list of poles
	    \param x
	*/
	void add_pole(double x);

private:
	void fill_index(int i, std::vector<int>& index) const; 

	size_t _M_half_degree;
	double _M_shift;
	std::vector<double> _M_poles;
	size_t _M_support_size;
	EInterpolation _M_type; 
};

typedef std::shared_ptr<CBSplineKernel> PSplineKernel;
typedef std::shared_ptr<CBSplineKernel> PBSplineKernel;

enum ci_type {ci_bspline, ci_omoms};

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
	return _M_support_size;
}

/** implements a B-Spline kernel of degree 2 */
class EXPORT_CORE CBSplineKernel0: public  CBSplineKernel{
 public:
	CBSplineKernel0();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
};


/** implements a B-Spline kernel of degree 2 */
class EXPORT_CORE CBSplineKernel2: public  CBSplineKernel{
 public:
	CBSplineKernel2();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
};

/** implements a B-Spline kernel of degree 3 */
class EXPORT_CORE CBSplineKernel3: public  CBSplineKernel{
 public:
	CBSplineKernel3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
	double get_mult_int(int s1, int s2, int range, EIntegralType type) const;  
private: 
};

/** implements a B-Spline kernel of degree 4 */
class EXPORT_CORE CBSplineKernel4: public  CBSplineKernel{
public:
	CBSplineKernel4();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
	double get_mult_int(int s1, int s2, int range, EIntegralType type) const;  
private: 
};

/** implements a B-Spline kernel of degree 5 */
class EXPORT_CORE CBSplineKernel5: public  CBSplineKernel{
public:
	CBSplineKernel5();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
};

/** implements a o-Moms kernel of degree 3 */
class EXPORT_CORE CBSplineKernelOMoms3 : public  CBSplineKernel{
public:
	CBSplineKernelOMoms3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int degree) const;
};

double  EXPORT_CORE integrate2(const CBSplineKernel& spline, double s1, double s2, int deg1, int deg2, double n, double x0, double L);

inline void mirror_boundary_conditions(std::vector<int>& index, int width, 
				       int width2)
{
	// skip the cases where nothing happens
	if (index[0] >= 0 && index[index.size()-1] < width)
		return; 
	for (size_t k = 0; k < index.size(); k++) {
		int idx = (index[k] < 0) ? -index[k] : index[k]; 
		
		idx = (width == 1) ? (0) : ((idx < width2) ? idx : idx % width2);
		if (width <= idx) {
			idx = width2 - idx;
		}
		index[k] = idx; 
	}
}

template <typename A>
struct FMultBy {
	FMultBy(double f):
		_M_f(f)
	{
	}
	void operator()(A& value)
	{
		value *= _M_f; 
	}
private: 
	double _M_f; 
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
	for (size_t k = 0; k < _M_poles.size() ; ++k) {
		lambda  *=  2 - _M_poles[k] - 1.0 / _M_poles[k];
	}
	
	/* apply the gain */
	for_each(coeff.begin(), coeff.end(), FMultBy<C>(lambda));
	
	/* loop over all poles */
	for (size_t k = 0; k < _M_poles.size(); ++k) {
		/* causal initialization */
		coeff[0] = initial_coeff(coeff, _M_poles[k]);
		
		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			coeff[n] += _M_poles[k] * coeff[n - 1];
		}
		
		/* anticausal initialization */
		coeff[coeff.size() - 1] = initial_anti_coeff(coeff, _M_poles[k]);
		/* anticausal recursion */
		for (int n = coeff.size() - 2; 0 <= n; n--) {
			coeff[n] = _M_poles[k] * (coeff[n + 1] - coeff[n]);
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



NS_MIA_END

#endif
