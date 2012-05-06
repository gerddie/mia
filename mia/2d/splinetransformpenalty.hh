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


#ifndef mia_2d_splinetransformpenalty_hh
#define mia_2d_splinetransformpenalty_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/core/vector.hh>
#include <mia/core/product_base.hh>


NS_MIA_BEGIN

class C2DSplineTransformPenalty : public CProductBase {

public: 
	virtual ~C2DSplineTransformPenalty(); 

	void initialize(const C2DBounds& size, const C2DFVector& range, PSplineKernel kernel); 

	double value(const C2DFVectorfield&  m_coefficients) const; 

	double value_and_gradient(const C2DFVectorfield&  m_coefficients, CDoubleVector& gradient) const;

protected: 

	const C2DBounds& get_size() const;
	
	const C2DFVector& get_range() const; 
	
	const CSplineKernel& get_kernel() const;        
private:

 	virtual void do_initialize() = 0; 

	virtual double value(const C2DFVectorfield&  m_coefficients) const = 0; 

	virtual double value_and_gradient(const C2DFVectorfield&  m_coefficients, CDoubleVector& gradient) const = 0;

	C2DBounds m_size;
	C2DFVector m_range; 
	PSplineKernel m_kernel
}; 


typedef TFactory<C2DSplineTransformPenalty> C2DSplineTransformPenaltyPlugin;

/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DSplineTransformPenaltyPlugin> > 
         C2DSplineTransformPenaltyPluginHandler;

FACTORY_TRAIT(C2DSplineTransformPenaltyPluginHandler); 


C2DSplineTransformPenaltyPlugin::ProductPtr produce_2d_spline_transform_penalty(const string& descr); 

NS_MIA_END

#endif 
