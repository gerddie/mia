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

#include <mia/2d/fullcost.hh>

NS_MIA_BEGIN

/**
   A accumulation of cost functions that are weigted against each other 
 */
class EXPORT_2D C2DFullCostList : public C2DFullCost {
public: 
	C2DFullCostList(); 

	void push(P2DFullCost cost); 
private: 
	bool do_has(const char *property) const;

	virtual double do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;
	double do_value(const C2DTransformation& t) const; 
	double do_value() const; 
	virtual void do_set_size(); 
	virtual void do_reinit(); 
	std::vector<P2DFullCost> _M_costs; 
}; 

NS_MIA_END
