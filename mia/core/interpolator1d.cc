/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

C1DInterpolatorFactory *create_1dinterpolation_factory(EInterpolation type, EBoundaryConditions bc)
{
	PSplineKernel kernel; 
	switch (type) {
	case ip_nn: 
	case ip_bspline0: kernel = produce_spline_kernel("bspline:d=0"); break; 
	case ip_linear:
	case ip_bspline1: kernel = produce_spline_kernel("bspline:d=1"); break; 
	case ip_bspline2: kernel = produce_spline_kernel("bspline:d=2"); break; 
	case ip_bspline3: kernel = produce_spline_kernel("bspline:d=3"); break; 
	case ip_bspline4: kernel = produce_spline_kernel("bspline:d=4"); break; 
	case ip_bspline5: kernel = produce_spline_kernel("bspline:d=5"); break; 
	case ip_omoms3:   kernel = produce_spline_kernel("omoms:d=3"); break;
	default: 
		throw invalid_argument("create_interpolator_factory:Unknown interpolator type requested"); 
	}; 
	PSplineBoundaryCondition pbc; 
	switch (bc) {
	case bc_mirror_on_bounds: pbc = produce_spline_boundary_condition("mirror"); break; 
	case bc_repeat:           pbc = produce_spline_boundary_condition("repeat"); break; 
	case bc_zero:             pbc = produce_spline_boundary_condition("zero"); break;   
	default: 
		throw invalid_argument("create_interpolator_factory:Unknown boundary condition requested"); 
		
	}
	
	return new C1DInterpolatorFactory(kernel, *pbc); 

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
#ifdef LONG_64BIT
INSTANCIATE_INTERPOLATORS(unsigned long);
INSTANCIATE_INTERPOLATORS(signed long);
#endif

NS_MIA_END
