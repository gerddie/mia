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

/*
  The weight evaluation of the b-splines is based on code by Philippe Thevenaz
  http://bigwww.epfl.ch/

*/

#include <cassert>

#include <mia/core/shared_ptr.hh>
#include <mia/core/interpolator1d.hh>
#include <mia/core/interpolator1d.cxx>

NS_MIA_BEGIN

C1DInterpolator::~C1DInterpolator()
{
}


C1DInterpolatorFactory::C1DInterpolatorFactory(EType type, PSplineKernel kernel):
	_M_type(type),
	_M_kernel(kernel)
{
	assert(_M_type == ipt_spline && _M_kernel);
}

C1DInterpolatorFactory::C1DInterpolatorFactory(EType type):
	_M_type(type)
{
	assert(_M_type == ipt_nn || _M_type == ipt_linear);
}

C1DInterpolatorFactory::C1DInterpolatorFactory(const C1DInterpolatorFactory& o):
	_M_type(o._M_type),
	_M_kernel(o._M_kernel)
{
}

C1DInterpolatorFactory& C1DInterpolatorFactory::operator = ( const C1DInterpolatorFactory& o)
{
	_M_type = o._M_type;
	_M_kernel = o._M_kernel;

	return *this;
}

PSplineKernel C1DInterpolatorFactory::get_kernel() const
{
	return _M_kernel;
}

C1DInterpolatorFactory::~C1DInterpolatorFactory()
{
}

C1DInterpolatorFactory *create_1dinterpolation_factory(EInterpolation type)
{
	SHARED_PTR(CBSplineKernel) kernel;
	C1DInterpolatorFactory::EType iptype = C1DInterpolatorFactory::ipt_spline;

	switch (type) {
	case ip_nn:
		return new C1DInterpolatorFactory(C1DInterpolatorFactory::ipt_nn);
		break;
	case ip_linear:
		return new C1DInterpolatorFactory(C1DInterpolatorFactory::ipt_linear);
		break;
	case ip_bspline2:
		kernel.reset(new CBSplineKernel2());
		break;
	case ip_bspline3:
		kernel.reset(new CBSplineKernel3());
		break;
	case ip_bspline4:
		kernel.reset(new CBSplineKernel4());
		break;
	case ip_bspline5:
		kernel.reset(new CBSplineKernel5());
		break;
	case ip_omoms3:
		kernel.reset(new CBSplineKernelOMoms3());
		break;
	default:
		throw std::invalid_argument("unknown interpolation method");
	}
	return new C1DInterpolatorFactory(iptype, kernel);
}


#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T1DInterpolator<TYPE>;		\
	template class T1DLinearInterpolator<TYPE>;	\
	template class T1DNNInterpolator<TYPE>;		\
	template class T1DConvoluteInterpolator<TYPE>

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

NS_MIA_END
