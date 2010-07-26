/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#ifndef mia_2d_fullcost_hh
#define mia_2d_fullcost_hh

#include <mia/2d/transform.hh>
#include <gsl++/vector.hh>

NS_MIA_BEGIN

struct EXPORT_2D fullcost_data2d_type {
	static const char *type_descr;
};


/**
   Base class for a general cost function. 
 */

class EXPORT_2D C2DFullCostBase {
public: 
	/**
	   Initialize the cost function with a weight 
	   \param weight 
	 */
	C2DFullCostBase(double weight);


	/**
	   Evaluate the weighted cost value and the weighted gradient in optimizer space 
	   given a current  transformation. 
	   \param t tranformation to be applied 
	   \retval gradient gradient in optimizer space 
	   \returns weighted cost value 
	 */
	double evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;
	
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
	virtual void do_set_size() = 0; 
	
	double _M_weight;
	C2DBounds _M_current_size; 

}; 

NS_MIA_END

#endif
