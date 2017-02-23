/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cassert>

#include <mia/core/interpolator1d.hh>
#include <mia/core/interpolator1d.cxx>

NS_MIA_BEGIN

using std::invalid_argument; 

C1DInterpolator::~C1DInterpolator()
{
}


C1DInterpolatorFactory::C1DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& bc):
	m_kernel(kernel),
	m_bc(bc.clone())
{
	assert(m_kernel);
	assert(m_bc); 
}

C1DInterpolatorFactory::C1DInterpolatorFactory(const std::string& kernel_descr, const std::string& boundary_descr):
	m_kernel(produce_spline_kernel(kernel_descr)),
	m_bc(produce_spline_boundary_condition(boundary_descr))
{
}


C1DInterpolatorFactory::C1DInterpolatorFactory(const C1DInterpolatorFactory& o):
	m_kernel(o.m_kernel), 
	m_bc(o.m_bc->clone())
{
}

C1DInterpolatorFactory& C1DInterpolatorFactory::operator = ( const C1DInterpolatorFactory& o)
{
	m_kernel = o.m_kernel;
	m_bc.reset(o.m_bc->clone()); 

	return *this;
}

PSplineKernel C1DInterpolatorFactory::get_kernel() const
{
	return m_kernel;
}

C1DInterpolatorFactory::~C1DInterpolatorFactory()
{
}

#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T1DInterpolator<TYPE>;		\
	template class T1DConvoluteInterpolator<TYPE>

INSTANCIATE_INTERPOLATORS(bool);
INSTANCIATE_INTERPOLATORS(uint8_t);
INSTANCIATE_INTERPOLATORS(int8_t);
INSTANCIATE_INTERPOLATORS(uint16_t);
INSTANCIATE_INTERPOLATORS(int16_t);
INSTANCIATE_INTERPOLATORS(int32_t);
INSTANCIATE_INTERPOLATORS(uint32_t);
INSTANCIATE_INTERPOLATORS(float);
INSTANCIATE_INTERPOLATORS(double);
INSTANCIATE_INTERPOLATORS(uint64_t);
INSTANCIATE_INTERPOLATORS(int64_t);

NS_MIA_END
