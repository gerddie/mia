/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
 *
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

#ifndef mia_2d_fullcost_hh
#define mia_2d_fullcost_hh

#include <mia/2d/transform.hh>
#include <gsl++/vector.hh>

NS_MIA_BEGIN


/**
   Base class for a general cost function. 
 */

class EXPORT_2D C2DFullCost : public CProductBase {
public: 
	static const char *type_descr;
	static const char *value;
	
	/**
	   Initialize the cost function with a weight 
	   \param weight 
	 */
	C2DFullCost(double weight);


	/**
	   Evaluate the weighted cost value and the weighted gradient in optimizer space 
	   given a current  transformation. 
	   \param t tranformation to be applied 
	   \retval gradient gradient in optimizer space 
	   \returns weighted cost value 
	 */
	double evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;

	/**
	   Evaluate the weighted cost value 
	   \param t tranformation to be applied 
	   \returns weighted cost value 
	 */
	double cost_value(const C2DTransformation& t) const;

	/**
	   Evaluate the weighted cost value without transforming the image 
	   \returns weighted cost value 
	 */
	double cost_value() const;

		
	
	/**
	   Set the size of the cost function 
	 */
	void set_size(const C2DBounds& size); 
protected: 
	/** \returns cost function weight  */
	double get_weight() const; 
	const C2DBounds& get_current_size() const; 
private:
	virtual double do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const = 0;
	virtual double do_value(const C2DTransformation& t) const = 0;
	virtual double do_value() const = 0;
	
	virtual void do_set_size() = 0; 
	
	double _M_weight;
	C2DBounds _M_current_size; 

}; 
typedef std::shared_ptr<C2DFullCost > P2DFullCost;


class EXPORT_2D C2DFullCostPlugin: public TFactory<C2DFullCost, C2DFullCost, C2DFullCost> {
public:
	C2DFullCostPlugin(const char *name);
private:
	virtual C2DFullCostPlugin::ProductPtr do_create() const;
	virtual C2DFullCostPlugin::ProductPtr do_create(float weight) const = 0;
	float _M_weight;
}; 

typedef THandlerSingleton<TFactoryPluginHandler<C2DFullCostPlugin> > C2DFullCostPluginHandler;

NS_MIA_END

#endif
