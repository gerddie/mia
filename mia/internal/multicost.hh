/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#ifndef mia_internal_multicost_hh
#define mia_internal_multicost_hh

#include <mia/internal/fullcost.hh>

NS_MIA_BEGIN

/**
   A accumulation of cost functions that are weigted against each other 
 */
template <typename Transform>
class EXPORT_HANDLER TFullCostList : public TFullCost<Transform> {
public: 
	TFullCostList(); 
	typedef typename TFullCost<Transform>::Pointer  Pointer; 

	void push(typename TFullCost<Transform>::Pointer cost); 
private: 
	bool do_has(const char *property) const;

	virtual double do_evaluate(const Transform& t, CDoubleVector& gradient) const;
	double do_value(const Transform& t) const; 
	double do_value() const; 
	virtual void do_set_size(); 
	virtual void do_reinit(); 
	std::vector<typename TFullCost<Transform>::Pointer> m_costs; 
}; 

NS_MIA_END

#endif
