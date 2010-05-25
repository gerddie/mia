/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>

NS_MIA_BEGIN
/**
   The kernel of spline based interpolations - base type
 */

extern EXPORT_CORE TDictMap<EInterpolation> GInterpolatorTable;

class EXPORT_CORE CBSplineKernel {
public:

	/**
	   \param degree of the spline
	   \param shift location shift of the input coordinate to obtain the proper support
	 */
	CBSplineKernel(size_t degree, double shift);

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

	/**
	   \returns the poles of the spline
	 */
	const std::vector<double>& get_poles() const;

	/**
	   \returns the size of the support of this kernel
	*/
	size_t size() const;

	double get_nonzero_radius() const; 
	
protected:



	/** add a pole to the list of poles
	    \param x
	*/
	void add_pole(double x);
private:
	size_t _M_half_degree;
	double _M_shift;
	std::vector<double> _M_poles;
	size_t _M_support_size;
};

typedef SHARED_PTR(CBSplineKernel) PSplineKernel;

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
};

/** implements a B-Spline kernel of degree 4 */
class EXPORT_CORE CBSplineKernel4: public  CBSplineKernel{
public:
	CBSplineKernel4();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int degree) const; 
	void get_derivative_weights(double x, std::vector<double>& weight, int degree) const; 
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

NS_MIA_END

#endif
