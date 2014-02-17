/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_internal_divcurl_hh
#define mia_internal_divcurl_hh


#include <mia/template/fullcost.hh>


NS_MIA_BEGIN

///  @cond DOC_PLUGINS 

template <typename Transform> 
class EXPORT TDivCurlFullCost : public TFullCost<Transform> {
public: 
	typedef typename TFullCost<Transform>::Size Size; 
	TDivCurlFullCost(double weight_div, double weight_curl, double weight); 
private: 
	double do_evaluate(const Transform& t, CDoubleVector& gradient) const;
	double do_value(const Transform& t) const;
	double do_value() const;
	void do_set_size(); 
	bool do_get_full_size(Size& size) const; 
	double m_weight_div; 
	double m_weight_curl; 
	double m_size_scale; 
}; 

template <typename T> 
class TDivcurlFullCostPlugin: public TFullCostPlugin<T> {
public: 
	TDivcurlFullCostPlugin(); 
private:
	TFullCost<T> *do_create(float weight) const;
	const std::string do_get_descr() const;
	float m_div;
	float m_curl;
}; 

NS_MIA_END
///  @endcond

#endif
