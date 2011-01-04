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

#ifndef mia_2d_interpolator_hh
#define mia_2d_interpolator_hh


#include <vector>
#include <mia/core/shared_ptr.hh>

#include <mia/core/defines.hh>
#include <mia/core/interpolator.hh>
#include <mia/2d/2DImage.hh>


NS_MIA_BEGIN

template <typename T>
struct max_hold_type<T2DVector<T> > {
	typedef T2DVector<double> type;
};


class EXPORT_2D C2DInterpolator {
public:
	/** a virtual destructor is neccessary for some of the interpolators */
	virtual  ~C2DInterpolator();
};

/**
   Basic Interpolator type for 2D Data.
 */

template <typename T>
class  EXPORT_2D T2DInterpolator : public  C2DInterpolator {
public:

	/**
	   \param x location of data value to read
	   \returns interpolated value at location x
	 */
	virtual T operator () (const C2DFVector& x) const = 0;
	virtual T2DVector<T> derivative_at(const C2DFVector& x) const = 0;

};


/** Base type for interpolators that work directly on the image data */
template <typename T>
class EXPORT_2D T2DDirectInterpolator: public T2DInterpolator<T> {
public:

	/** Constructor
	    \param data the source data
	 */
	T2DDirectInterpolator(const T2DDatafield<T>& data);
protected:
	const T2DDatafield<T>& data()const {
		return _M_data;
	}
private:
	const T2DDatafield<T>& _M_data;
};

/**
    Nearest Neighbor interpolation.
 */

template <class T>
class EXPORT_2D T2DNNInterpolator: public T2DDirectInterpolator<T> {
public:
	T2DNNInterpolator(const T2DDatafield<T>& image);
	T operator () (const C2DFVector& x) const;
	T2DVector<T> derivative_at(const C2DFVector& x) const;

};

/**
   Tri-linear interpolation
*/

template <class T>
class EXPORT_2D T2DBilinearInterpolator: public T2DDirectInterpolator<T> {
public:
	T2DBilinearInterpolator(const T2DDatafield<T>& image);
	T operator () (const C2DFVector& x) const;
	T2DVector<T> derivative_at(const C2DFVector& x) const;
private:
	size_t _M_xy;
	C2DFVector _M_size;
	C2DFVector _M_sizeb;
};

template <class U>
struct coeff_map<T2DVector<U> > {
	typedef T2DVector<U> value_type;
	typedef C2DDVector   coeff_type;
};

template <class T>
class EXPORT_2D T2DConvoluteInterpolator: public T2DInterpolator<T> {
public:
	T2DConvoluteInterpolator(const T2DDatafield<T>& image, std::shared_ptr<CBSplineKernel > kernel);
	~T2DConvoluteInterpolator();
	T  operator () (const C2DFVector& x) const;

	T evaluate(const std::vector<double>& xweight, const std::vector<double>& yweight,
		   const std::vector<int>&    xindex,  const std::vector<int>&    yindex) const; 

	T2DVector<T> derivative_at(const C2DFVector& x) const;

	typedef T2DDatafield< typename coeff_map< T >::coeff_type > TCoeff2D;

	const TCoeff2D& get_coefficients() const; 

protected:
	typedef std::vector< typename TCoeff2D::value_type > coeff_vector;
private:

	typename TCoeff2D::value_type evaluate() const;

	TCoeff2D _M_coeff;
	C2DBounds _M_size2;
	std::shared_ptr<CBSplineKernel > _M_kernel;
	T _M_min;
	T _M_max;

	mutable std::vector<int> _M_x_index; 
	mutable std::vector<int> _M_y_index; 
	mutable std::vector<double> _M_x_weight; 
	mutable std::vector<double> _M_y_weight; 

};

class EXPORT_2D C2DInterpolatorFactory {
public:
	enum EType {ip_nn, ip_tri, ip_spline, ip_unknown};

	C2DInterpolatorFactory(EType type, std::shared_ptr<CBSplineKernel > kernel);

	C2DInterpolatorFactory(const C2DInterpolatorFactory& o);

	C2DInterpolatorFactory& operator = ( const C2DInterpolatorFactory& o);

	virtual ~C2DInterpolatorFactory();

	template <class T>
	T2DInterpolator<T> *create(const T2DDatafield<T>& src) const
		__attribute__ ((warn_unused_result));

	const CBSplineKernel* get_kernel() const;

private:
	EType _M_type;
	std::shared_ptr<CBSplineKernel > _M_kernel;
};
typedef std::shared_ptr<C2DInterpolatorFactory > P2DInterpolatorFactory;


C2DInterpolatorFactory EXPORT_2D  *create_2dinterpolation_factory(int type)
	__attribute__ ((warn_unused_result));

// implementation

template <class T>
T2DInterpolator<T> *C2DInterpolatorFactory::create(const T2DDatafield<T>& src) const
{
	switch (_M_type) {
	case ip_nn:  return new T2DNNInterpolator<T>(src);
	case ip_tri: return new T2DBilinearInterpolator<T>(src);
	case ip_spline: return new T2DConvoluteInterpolator<T>(src, _M_kernel);
	default: throw "CInterpolatorFactory::create: Unknown interpolator requested";
	}
	return NULL;
}

NS_MIA_END

#endif
