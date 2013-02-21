/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#ifndef mia_3d_splinetransformpenalty_hh
#define mia_3d_splinetransformpenalty_hh

#include <mia/3d/interpolator.hh>
#include <mia/3d/datafield.hh>
#include <mia/core/vector.hh>
#include <mia/core/product_base.hh>


NS_MIA_BEGIN

/**
   \brief Base class for transformation penalties in spline based deformations

   This class ist the base class for the possible penalties on spline based 
   transformations that provide an additional smoothness constrait. 
*/

class EXPORT_3D C3DSplineTransformPenalty : public CProductBase {

public: 
	typedef C3DSplineTransformPenalty plugin_data;
	static const char *data_descr;
	typedef C3DSplineTransformPenalty plugin_type;
	static const char *type_descr;
	
	/**
	   Constructor that sets the weight of the penalty term 
	   \param weight 
	 */
	C3DSplineTransformPenalty(double weight); 

	C3DSplineTransformPenalty(const C3DSplineTransformPenalty& org) = delete; 
	C3DSplineTransformPenalty& operator = (const C3DSplineTransformPenalty& org) = delete; 

	/**
	   Provides a deep copy of the penalty 
	   \returns newly created copy of the spline penalty term. 
	 */
	C3DSplineTransformPenalty *clone() const __attribute__((warn_unused_result));

	virtual ~C3DSplineTransformPenalty(); 

	/**
	   The initializer of the spline transform penalty to set up required data structurs 
	   \param size size of the coefficient field 
	   \param range transformation range 
	   \param kernel kernel used to define the spline transformation 
	 */
	void initialize(const C3DBounds& size, const C3DFVector& range, PSplineKernel kernel); 

	/**
	   Evaluate the penalty of a spline transformation defined by its coefficients 
	   \param coefficients the spline transformation coefficients 
	   \returns the weighted penalty
	 */
	double value(const C3DFVectorfield&  coefficients) const; 

	/**
	   Evaluate the penalty and the gradient of a spline transformation defined by its coefficients 
	   \param coefficients the spline transformation coefficients 
	   \param[out] gradient the weighted gradient of the spline transformation penalty given so that 
	   coefficients + \f$\delta\f$ gradient moves toward the minimum of the penalty term.
	   \returns the weighted penalty
	 */

	double value_and_gradient(const C3DFVectorfield&  coefficients, CDoubleVector& gradient) const;

protected: 


	
	const C3DBounds& get_size() const;
	
	const C3DFVector& get_range() const; 
	
	PSplineKernel get_kernel() const;        

	double get_weight() const; 

private:

 	virtual void do_initialize() = 0; 

	virtual double do_value(const C3DFVectorfield&  coefficients) const = 0; 

	virtual double do_value_and_gradient(const C3DFVectorfield&  coefficients, CDoubleVector& gradient) const = 0;
	
	virtual C3DSplineTransformPenalty *do_clone() const  = 0;

	double m_weight; 

	C3DBounds m_size;
	C3DFVector m_range; 
	PSplineKernel m_kernel;
}; 

typedef std::shared_ptr<C3DSplineTransformPenalty> P3DSplineTransformPenalty; 


class EXPORT_3D C3DSplineTransformPenaltyPlugin: public TFactory<C3DSplineTransformPenalty> {
public: 
	C3DSplineTransformPenaltyPlugin(char const * const  name); 
private: 
	virtual Product *do_create() const __attribute__((warn_unused_result));
	virtual Product *do_create(float weight) const __attribute__((warn_unused_result)) = 0 ;

	float m_weight; 
}; 


/// Plugin handler for image combiner plugins 
typedef THandlerSingleton<TFactoryPluginHandler<C3DSplineTransformPenaltyPlugin> > 
         C3DSplineTransformPenaltyPluginHandler;


FACTORY_TRAIT(C3DSplineTransformPenaltyPluginHandler); 


EXPORT_3D  C3DSplineTransformPenaltyPluginHandler::ProductPtr produce_3d_spline_transform_penalty(const std::string& descr); 


class EXPORT_3D C3DSplineTransformPenaltyPluginHandlerTest {
public: 
	C3DSplineTransformPenaltyPluginHandlerTest(); 
}; 

NS_MIA_END

#endif 
