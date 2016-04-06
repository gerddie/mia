/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/property_flags.hh>
#include <mia/core/parallel.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)

///  @cond DOC_PLUGINS 

/**

   @ingroup registation 
*/
template <typename TCost> 
class TSSDAutomaskCost: public TCost {
public: 	
	typedef typename TCost::Data Data; 
	typedef typename TCost::Force Force; 

	TSSDAutomaskCost(); 
	TSSDAutomaskCost(double src_mask_thresh, double ref_mask_thresh); 
private: 
	virtual double do_value(const Data& a, const Data& b) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, Force& force) const; 
	double m_src_mask_thresh;
	double m_ref_mask_thresh; 
};


/**
   This is the plug-in declaration - the actual plugin needs to define the 
   cost plugin type and the data type (this could be unified) 
   do_test and do_get_descr need to be implemented 
*/
template <typename CP, typename C> 
class TSSDAutomaskCostPlugin: public CP {
public: 
	TSSDAutomaskCostPlugin();
	C *do_create()const;
private: 
	double m_src_mask_thresh;
	double m_ref_mask_thresh; 

};


/// @endcond 
NS_END
