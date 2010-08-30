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

#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>
#include <mia/core/interpolator.hh>


NS_MIA_BEGIN

/**
   Basic Interpolator type for 1D Data.
 */

class EXPORT_CORE C1DInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~C1DInterpolator();
};


/**
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
	virtual typename coeff_map<T>::coeff_type derivative_at (const double& x) const = 0;


};


/** Base type for interpolators that work directly on the image data */
template <typename T>
class EXPORT_CORE T1DDirectInterpolator: public T1DInterpolator<T> {
public:

	/** Constructor
	    \param data the source data
	 */
	T1DDirectInterpolator(const std::vector<T>& data);
protected:
	const std::vector<T>& data()const {
		return _M_data;
	}
private:
	const std::vector<T>& _M_data;
};

/**
    Nearest Neighbor interpolation.
 */

template <class T>
class EXPORT_CORE T1DNNInterpolator: public T1DDirectInterpolator<T> {
public:
	T1DNNInterpolator(const std::vector<T>& image);
	T operator () (const double& x) const;
	virtual  typename coeff_map<T>::coeff_type
		derivative_at (const double& x) const;
};

/**
   Tri-linear interpolation
*/

template <class T>
class EXPORT_CORE T1DLinearInterpolator: public T1DDirectInterpolator<T> {
public:
	T1DLinearInterpolator(const std::vector<T>& image);
	T operator () (const double& x) const;
	virtual typename coeff_map<T>::coeff_type derivative_at (const double& x) const;
private:
	size_t _M_xy;
	double _M_size;
	double _M_sizeb;
};

/** Base type for interpolators that work with some kind of convolution  */

template <class T>
class EXPORT_CORE T1DConvoluteInterpolator: public T1DInterpolator<T> {
public:
	T1DConvoluteInterpolator(const std::vector<T>& image, PSplineKernel kernel);
	~T1DConvoluteInterpolator();
	T  operator () (const double& x) const;
	virtual typename coeff_map<T>::coeff_type derivative_at (const double& x) const;

protected:
	typedef std::vector< typename coeff_map< T >::coeff_type > TCoeff1D;

	typedef std::vector< typename TCoeff1D::value_type > coeff_vector;
private:

	void filter_line(coeff_vector& coeff, const std::vector<double>& poles);
	typename TCoeff1D::value_type initial_coeff(const coeff_vector& coeff, double pole);
	typename TCoeff1D::value_type initial_anti_coeff(const coeff_vector& coeff, double pole);

	TCoeff1D _M_coeff;
	size_t _M_size2;
	PSplineKernel _M_kernel;
	T _M_min;
	T _M_max;

	mutable std::vector<int> _M_x_index;
	mutable std::vector<int> _M_y_index;
	mutable std::vector<double> _M_x_weight;
	mutable std::vector<double> _M_y_weight;
};


/** Factory to create 1D interpolators of a give type using the given input data */
class EXPORT_CORE C1DInterpolatorFactory {
public:
	enum EType {ipt_nn, ipt_linear, ipt_spline, ipt_unknown};

	C1DInterpolatorFactory(EType type);
	C1DInterpolatorFactory(EType type, std::shared_ptr<CBSplineKernel > kernel);

	C1DInterpolatorFactory(const C1DInterpolatorFactory& o);

	C1DInterpolatorFactory& operator = ( const C1DInterpolatorFactory& o);

	virtual ~C1DInterpolatorFactory();

	template <class T>
	T1DInterpolator<T> *create(const std::vector<T>& src) const;

	PSplineKernel get_kernel() const;

private:
	EType _M_type;
	PSplineKernel _M_kernel;
};
typedef std::shared_ptr<const C1DInterpolatorFactory > P1DInterpolatorFactory;



C1DInterpolatorFactory EXPORT_CORE  *create_1dinterpolation_factory(EInterpolation type);

// implementation

template <class T>
T1DInterpolator<T> *C1DInterpolatorFactory::create(const std::vector<T>& src) const
{
	switch (_M_type) {
	case ipt_nn:  return new T1DNNInterpolator<T>(src);
	case ipt_linear: return new T1DLinearInterpolator<T>(src);
	case ipt_spline: return new T1DConvoluteInterpolator<T>(src, _M_kernel);
	default: throw "CInterpolatorFactory::create: Unknown interpolator requested";
	}
	return NULL;
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
