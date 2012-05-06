/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/export_handler.hh>
#include <mia/2d/splinetransformpenalty.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>


NS_MIA_BEGIN


C2DSplineTransformPenalty::~C2DSplineTransformPenalty()
{
}

void C2DSplineTransformPenalty::initialize(const C2DBounds& size, const C2DFVector& range, PSplineKernel kernel)
{
	m_size = size;
	m_range = range; 
	m_kernel = kernel; 
	do_initialize(); 
}


double C2DSplineTransformPenalty::value(const C2DFVectorfield&  coefficients) const
{
	return do_value(coefficients); 
}


double C2DSplineTransformPenalty::value_and_gradient(const C2DFVectorfield&  coefficients, CDoubleVector& gradient) const
{
	return do_value_and_gradient(coefficients, gradient); 
}

const C2DBounds& C2DSplineTransformPenalty::get_size() const
{
	return m_size; 
}

const C2DFVector& C2DSplineTransformPenalty::get_range() const
{
	return m_range; 
}

const CSplineKernel& C2DSplineTransformPenalty::get_kernel() const
{
	return *m_kernel; 
}

C2DSplineTransformPenaltyPlugin::ProductPtr produce_2d_spline_transform_penalty(const string& descr)
{
	return C2DSplineTransformPenaltyPluginHandler::instance().produce(descr); 
}

EXPLICIT_INSTANCE_HANDLER(C2DSplineTransformPenalty);

template<> const  char * const 
TPluginHandler<TFactory<C2DSplineTransformPenalty>>::m_help = 
    "These plug-ins provides penalty terms to enforce smoothness of spline based 2D transformation.";



NS_MIA_END
