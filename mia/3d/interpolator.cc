/*
** Copyrigh (C) 2004 MPI of Human Cognitive and Brain Sience
**                    Gert Wollny <wollny@cbs.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  The weight evaluation of the b-splines is based on code by Philippe Thevenaz 
  http://bigwww.epfl.ch/
  
*/

#include <cassert>

#include <mia/3d/interpolator.hh>
#include <mia/3d/interpolator.cxx>


NS_MIA_BEGIN

using namespace boost; 

CInterpolator::~CInterpolator()
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(EType type, SHARED_PTR(CBSplineKernel) kernel):
	_M_type(type),
	_M_kernel(kernel)
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(const C3DInterpolatorFactory& o):
	_M_type(o._M_type),
	_M_kernel(o._M_kernel)
{
}

C3DInterpolatorFactory& C3DInterpolatorFactory::operator = ( const C3DInterpolatorFactory& o)
{
	_M_type = o._M_type; 
	_M_kernel = o._M_kernel; 
	
	return *this; 
}

C3DInterpolatorFactory::~C3DInterpolatorFactory()
{
}


EXPORT_3D C3DInterpolatorFactory *create_3dinterpolation_factory(int type) 
{
	SHARED_PTR(CBSplineKernel) kernel; 
	C3DInterpolatorFactory::EType iptype = C3DInterpolatorFactory::ip_spline; 
	
	switch (type) {
	case ip_nn: 
		iptype = C3DInterpolatorFactory::ip_nn; 
		break;  
	case ip_linear: 
		iptype = C3DInterpolatorFactory::ip_tri; 
		break;  
	case ip_bspline2: 
		iptype = C3DInterpolatorFactory::ip_spline; 
		kernel.reset(new CBSplineKernel2()); 
		break; 
	case ip_bspline3:
		iptype = C3DInterpolatorFactory::ip_spline; 
		kernel.reset(new CBSplineKernel3()); 
		break; 
	case ip_bspline4:
		iptype = C3DInterpolatorFactory::ip_spline; 
		kernel.reset(new CBSplineKernel4()); 
		break; 
	case ip_bspline5:		
		iptype = C3DInterpolatorFactory::ip_spline; 
		kernel.reset(new CBSplineKernel5()); 
		break; 
	case ip_omoms3:
		iptype = C3DInterpolatorFactory::ip_spline; 
		kernel.reset(new CBSplineKernelOMoms3()); 
		break; 
	default:
		throw std::invalid_argument("unknown interpolation method");
	}
	return new C3DInterpolatorFactory(iptype, kernel); 
}

#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T3DInterpolator<TYPE>;		\
	template class T3DTrilinearInterpolator<TYPE>;	\
	template class T3DNNInterpolator<TYPE>;		\
	template class T3DConvoluteInterpolator<TYPE>

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
