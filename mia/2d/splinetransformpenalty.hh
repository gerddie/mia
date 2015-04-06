/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_2d_splinetransformpenalty_hh
#define mia_2d_splinetransformpenalty_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/datafield.hh>
#include <mia/core/vector.hh>
#include <mia/core/product_base.hh>


NS_MIA_BEGIN

/**
   \brief Base class for transformation penalties in spline based deformations

   This class ist the base class for the possible penalties on spline based 
   transformations that provide an additional smoothness constrait. 
*/

class EXPORT_2D C2DSplineTransformPenalty : public CProductBase {

public: 
	typedef C2DSplineTransformPenalty plugin_data;
	static const char *data_descr;
	typedef C2DSplineTransformPenalty plugin_type;
	static const char *type_descr;
	
	/**
	   Constructor that sets the weight of the penalty term 
	   @param weight 
	   @param normalize 
	 */
	C2DSplineTransformPenalty(double weight, bool normalize); 

	C2DSplineTransformPenalty(const C2DSplineTransformPenalty& org) = delete; 
	C2DSplineTransformPenalty& operator = (const C2DSplineTransformPenalty& org) = delete; 

	/**
	   Provides a deep copy of the penalty 
	   \returns newly created copy of the spline penalty term. 
	 */
	C2DSplineTransformPenalty *clone() const __attribute__((warn_unused_result));

	virtual ~C2DSplineTransformPenalty(); 

	/**
	   The initializer of the spline transform penalty to set up required data structurs 
	   \param size size of the coefficient field 
	   \param range transformation range 
	   \param kernel kernel used to define the spline transformation 
	 */
	void initialize(const C2DBounds& size, const C2DFVector& range, PSplineKernel kernel); 

	/**
	   Evaluate the penalty of a spline transformation defined by its coefficients 
	   \param coefficients the spline transformation coefficients 
	   \returns the weighted penalty
	 */
	double value(const C2DFVectorfield&  coefficients) const; 

	/**
	   Evaluate the penalty and the gradient of a spline transformation defined by its coefficients 
	   \param coefficients the spline transformation coefficients 
	   \param[out] gradient the weighted gradient of the spline transformation penalty given so that 
	   coefficients + \f$\delta\f$ gradient moves toward the minimum of the penalty term.
	   \returns the weighted penalty
	 */

	double value_and_gradient(const C2DFVectorfield&  coefficients, CDoubleVector& gradient) const;

protected: 
	
	const C2DBounds& get_size() const;
	
	const C2DFVector& get_range() const; 
	
	PSplineKernel get_kernel() const;        

	double get_weight() const; 
	
	bool get_normalize() const; 

private:

 	virtual void do_initialize() = 0; 

	virtual double do_value(const C2DFVectorfield&  coefficients) const = 0; 

	virtual double do_value_and_gradient(const C2DFVectorfield&  coefficients, CDoubleVector& gradient) const = 0;
	
	virtual C2DSplineTransformPenalty *do_clone() const  = 0;

	double m_weight; 
	bool m_normalize; 

	C2DBounds m_size;
	C2DFVector m_range; 
	PSplineKernel m_kernel;
}; 

typedef std::shared_ptr<C2DSplineTransformPenalty> P2DSplineTransformPenalty; 


class EXPORT_2D C2DSplineTransformPenaltyPlugin: public TFactory<C2DSplineTransformPenalty> {
public: 
	C2DSplineTransformPenaltyPlugin(char const * const  name); 
private: 
	virtual Product *do_create() const __attribute__((warn_unused_result));
	virtual Product *do_create(float weight, bool normalize) const __attribute__((warn_unused_result)) = 0 ;

	float m_weight; 
	bool m_normalize; 
}; 


/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DSplineTransformPenaltyPlugin> > 
         C2DSplineTransformPenaltyPluginHandler;


FACTORY_TRAIT(C2DSplineTransformPenaltyPluginHandler); 


EXPORT_2D  C2DSplineTransformPenaltyPluginHandler::ProductPtr produce_2d_spline_transform_penalty(const std::string& descr); 


class EXPORT_2D C2DSplineTransformPenaltyPluginHandlerTest {
public: 
	C2DSplineTransformPenaltyPluginHandlerTest(); 
}; 

NS_MIA_END

#endif 
