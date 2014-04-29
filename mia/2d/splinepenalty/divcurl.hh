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

#include <mia/2d/splinetransformpenalty.hh>
#include <mia/2d/ppmatrix.hh>

NS_BEGIN(divcurl_splinepenalty)

class C2DDivcurlSplinePenalty: public mia::C2DSplineTransformPenalty {
public: 
	
	C2DDivcurlSplinePenalty(double weight, bool normalize, double div_weight, double curl_weight);
	
private: 
	void do_initialize(); 
	
	double do_value(const mia::C2DFVectorfield&  coefficients) const; 
	
	double do_value_and_gradient(const mia::C2DFVectorfield&  coefficients, mia::CDoubleVector& gradient) const;
	
	mia::C2DSplineTransformPenalty *do_clone() const;

	double m_div_weight; 
	double m_curl_weight; 

	std::unique_ptr<mia::C2DPPDivcurlMatrix> m_ppmatrix;

}; 


class C2DDivcurlSplinePenaltyPlugin : public mia::C2DSplineTransformPenaltyPlugin {
public: 
	C2DDivcurlSplinePenaltyPlugin(); 
	
private: 
	virtual const std::string do_get_descr() const;
	virtual Product *do_create(float weight, bool normalize) const __attribute__((warn_unused_result));
	float m_div_weight; 
	float m_curl_weight; 

}; 

NS_END
