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

/*
  The weight evaluation of the b-splines is based on code by Philippe Thevenaz
  http://bigwww.epfl.ch/

*/

#include <cassert>

#include <mia/core/interpolator1d.hh>
#include <mia/core/interpolator1d.cxx>

NS_MIA_BEGIN

C1DInterpolator::~C1DInterpolator()
{
}


C1DInterpolatorFactory::C1DInterpolatorFactory(PSplineKernel kernel, PBoundaryCondition bc):
	m_kernel(kernel),
	m_bc(bc)
{
	assert(m_kernel);
	assert(m_bc); 
}

C1DInterpolatorFactory::C1DInterpolatorFactory(const C1DInterpolatorFactory& o):
	m_kernel(o.m_kernel), 
	m_bc(o.m_bc)
{
}

C1DInterpolatorFactory& C1DInterpolatorFactory::operator = ( const C1DInterpolatorFactory& o)
{
	m_kernel = o.m_kernel;
	m_bc = o.m_bc; 

	return *this;
}

PSplineKernel C1DInterpolatorFactory::get_kernel() const
{
	return m_kernel;
}

C1DInterpolatorFactory::~C1DInterpolatorFactory()
{
}

C1DInterpolatorFactory *create_1dinterpolation_factory(EInterpolation type, EBoundaryConditions bc)
{
	return create_interpolator_factory<C1DInterpolatorFactory>(type, bc); 
}


#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T1DInterpolator<TYPE>;		\
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
