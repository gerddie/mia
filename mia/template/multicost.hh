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

#ifndef mia_internal_multicost_hh
#define mia_internal_multicost_hh

#include <mia/template/fullcost.hh>

NS_MIA_BEGIN

/**
   \ingroup registration 
   \tparam Transform the transformation type used to achieve registration by optimizing the cost function 
   \brief A accumulation of cost functions that are weigted against each other 

   This class is used to accumulate various cost measures to be optimized for image registration. 
 */
template <typename Transform>
class EXPORT_HANDLER TFullCostList : public TFullCost<Transform> {
public: 
	TFullCostList(); 

	/// The pointer type of this cost function 
	typedef typename TFullCost<Transform>::Pointer  Pointer; 

	/// The size type of this cost function type 
	typedef typename TFullCost<Transform>::Size     Size; 

	/**
	   Append a new cost function to the list 
	   \param cost a shared pointer to the new cost measure. 
	 */
	void push(typename TFullCost<Transform>::Pointer cost); 

	
private: 
	bool do_has(const char *property) const;

	virtual double do_evaluate(const Transform& t, CDoubleVector& gradient) const;
	double do_value(const Transform& t) const; 
	double do_value() const; 
	void do_set_size(); 
	void do_reinit(); 
	bool do_get_full_size(Size& size) const; 
	std::vector<typename TFullCost<Transform>::Pointer> m_costs; 
}; 

NS_MIA_END

#endif
