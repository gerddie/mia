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

/*
  The weight evaluation of the b-splines is based on code by Philippe Thevenaz
  http://bigwww.epfl.ch/

*/

#include <cassert>

#include <mia/core/interpolator1d.hh>
#include <mia/2d/interpolator.hh>
#include <mia/core/unaryfunction.hh>
#include <mia/core/simpson.hh>

#include <mia/core/interpolator1d.cxx>
#include <mia/2d/interpolator.cxx>


NS_MIA_BEGIN

using namespace boost;

struct F2DKernelIntegrator: public FUnary {
	F2DKernelIntegrator(const CBSplineKernel& spline, double s1, double s2, int deg1, int deg2):
		_M_spline(spline), _M_s1(s1), _M_s2(s2), _M_deg1(deg1), _M_deg2(deg2)
		{
		}
	virtual double operator() (double x) const {
		return _M_spline.get_weight_at(x - _M_s1, _M_deg1) *
			_M_spline.get_weight_at(x - _M_s2, _M_deg2);
	}
private:
	const CBSplineKernel& _M_spline;
	double _M_s1, _M_s2, _M_deg1, _M_deg2;
};


double integrate2(const CBSplineKernel& spline, double s1, double s2, int deg1, int deg2, double n, double x0, double L)
{
	double sum = 0.0;
	x0 /= n;
	L  /= n;

	// evaluate interval to integrate over
	double start_int = s1 - spline.get_nonzero_radius();
	double end_int = s1 + spline.get_nonzero_radius();
	if (start_int > s2 - spline.get_nonzero_radius())
		start_int = s2 - spline.get_nonzero_radius();
	if (start_int < x0)
		start_int = x0;
	if (end_int > s2 + spline.get_nonzero_radius())
		end_int = s2 + spline.get_nonzero_radius();
	if (end_int > L)
		end_int = L;

	// Simpson formula
	if (end_int <= start_int)
		return sum;
	const size_t intervals = size_t(4 * (end_int - start_int));

	sum = simpson( start_int, end_int, intervals, F2DKernelIntegrator(spline, s1, s2, deg1, deg2));
	return sum * n;
}

C2DInterpolator::~C2DInterpolator()
{
}


C2DInterpolatorFactory::C2DInterpolatorFactory(EType type, SHARED_PTR(CBSplineKernel) kernel):
	_M_type(type),
	_M_kernel(kernel)
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(const C2DInterpolatorFactory& o):
	_M_type(o._M_type),
	_M_kernel(o._M_kernel)
{
}

C2DInterpolatorFactory& C2DInterpolatorFactory::operator = ( const C2DInterpolatorFactory& o)
{
	_M_type = o._M_type;
	_M_kernel = o._M_kernel;

	return *this;
}

C2DInterpolatorFactory::~C2DInterpolatorFactory()
{
}

const CBSplineKernel* C2DInterpolatorFactory::get_kernel() const
{
	return _M_kernel.get();
}

C2DInterpolatorFactory *create_2dinterpolation_factory(int type)
{
	SHARED_PTR(CBSplineKernel) kernel;
	C2DInterpolatorFactory::EType iptype = C2DInterpolatorFactory::ip_spline;

	switch (type) {
	case ip_nn:
		iptype = C2DInterpolatorFactory::ip_nn;
		break;
	case ip_linear:
		iptype = C2DInterpolatorFactory::ip_tri;
		break;
	case ip_bspline2:
		iptype = C2DInterpolatorFactory::ip_spline;
		kernel.reset(new CBSplineKernel2());
		break;
	case ip_bspline3:
		iptype = C2DInterpolatorFactory::ip_spline;
		kernel.reset(new CBSplineKernel3());
		break;
	case ip_bspline4:
		iptype = C2DInterpolatorFactory::ip_spline;
		kernel.reset(new CBSplineKernel4());
		break;
	case ip_bspline5:
		iptype = C2DInterpolatorFactory::ip_spline;
		kernel.reset(new CBSplineKernel5());
		break;
	case ip_omoms3:
		iptype = C2DInterpolatorFactory::ip_spline;
		kernel.reset(new CBSplineKernelOMoms3());
		break;
	default:
		throw std::invalid_argument("unknown interpolation method");
	}
	return new C2DInterpolatorFactory(iptype, kernel);
}


#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T2DInterpolator<TYPE>;		\
	template class T2DBilinearInterpolator<TYPE>;	\
	template class T2DNNInterpolator<TYPE>;		\
	template class T2DConvoluteInterpolator<TYPE>

INSTANCIATE_INTERPOLATORS(bool);
INSTANCIATE_INTERPOLATORS(unsigned char);
INSTANCIATE_INTERPOLATORS(signed char);
INSTANCIATE_INTERPOLATORS(unsigned short);
INSTANCIATE_INTERPOLATORS(signed short);
INSTANCIATE_INTERPOLATORS(unsigned int);
INSTANCIATE_INTERPOLATORS(signed int);
INSTANCIATE_INTERPOLATORS(float);
INSTANCIATE_INTERPOLATORS(double);
#ifdef HAVE_INT64
INSTANCIATE_INTERPOLATORS(mia_int64);
INSTANCIATE_INTERPOLATORS(mia_uint64);
#endif


INSTANCIATE_INTERPOLATORS(C2DFVector);

template class T1DInterpolator<C2DFVector>;
template class T1DLinearInterpolator<C2DFVector>;
template class T1DNNInterpolator<C2DFVector>;
template class T1DConvoluteInterpolator<C2DFVector>;



NS_MIA_END
