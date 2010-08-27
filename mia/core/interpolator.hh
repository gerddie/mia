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
	enum EIntegralType { integral_11, 
			     integral_20, 
			     integral_02, 
			     integral_unknown }; 
			     

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
	   evaluate the integral 
	   /f[
	   * \int_0^\text{range}
	   \frac{\partial^{p}}{\partial x}\beta(x-s_1) \frac{\partial^{q}}{\partial x}\beta(x-s_2) dx
	   * /f]
	   with q1 and q2 defined by \a type 
	   \param s1
	   \param s2
	   \param range 
	   \param type 
	   \returns value of integral 
	   \remark a simpson based integration is implemented for all spline classes, but normally, a 
            spline class could provide precalculated values
	 */
	virtual double get_mult_int(int s1, int s2, int range, EIntegralType type) const;  

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

protected:

	/** add a pole to the list of poles
	    \param x
	*/
	void add_pole(double x);

	double mult_int_from_table(int s1, int s2, int range, EIntegralType type) const; 
private:
	void fill_index(int i, std::vector<int>& index) const; 
	virtual double read_table(int skip, int delta, bool swapped, EIntegralType type) const; 
	size_t _M_half_degree;
	double _M_shift;
	std::vector<double> _M_poles;
	size_t _M_support_size;
	EInterpolation _M_type; 
};

typedef std::shared_ptr<CBSplineKernel> PSplineKernel;

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
	double get_mult_int(int s1, int s2, int range, EIntegralType type) const;  
private: 
	virtual double read_table(int skip, int delta, bool swapped, EIntegralType type) const; 
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
	virtual double read_table(int skip, int delta, bool swapped, EIntegralType type) const; 
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


NS_MIA_END

#endif
