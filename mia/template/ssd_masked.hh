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

#ifndef mia_template_ssd_masked_hh
#define mia_template_ssd_masked_hh


NS_BEGIN(NS)

///  @cond DOC_PLUGINS 

/**

   @ingroup registation 
*/
template <typename TCost> 
class TSSDMaskedImageCost: public TCost {
public: 	
	typedef typename TCost::Data Data; 
	typedef typename TCost::Mask Mask; 
	typedef typename TCost::Force Force; 

	TSSDMaskedImageCost(); 
private: 
	virtual double do_value(const Data& a, const Data& b, const Mask& m) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const; 
};


/**
   This is the plug-in declaration - the actual plugin needs to define the 
   cost plugin type and the data type (this could be unified) 
   do_test and do_get_descr need to be implemented 
*/
template <typename CP, typename C> 
class TSSDMaskedImageCostPlugin: public CP {
public: 
	TSSDMaskedImageCostPlugin();
private:
	C *do_create()const;
	const std::string do_get_descr() const; 
};

/// @endcond 
NS_END

#endif 

