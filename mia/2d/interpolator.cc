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

#include <mia/core/interpolator1d.cxx>
#include <mia/2d/interpolator.cxx>


NS_MIA_BEGIN

using namespace boost;

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
