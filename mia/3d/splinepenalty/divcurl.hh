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

#include <mia/3d/splinetransformpenalty.hh>
#include <mia/3d/ppmatrix.hh>

NS_BEGIN(divcurl_splinepenalty)

class C3DDivcurlSplinePenalty: public mia::C3DSplineTransformPenalty {
public: 
	
	C3DDivcurlSplinePenalty(double weight, double div_weight, double curl_weight);
	
private: 
	void do_initialize(); 
	
	double do_value(const mia::C3DFVectorfield&  coefficients) const; 
	
	double do_value_and_gradient(const mia::C3DFVectorfield&  coefficients, mia::CDoubleVector& gradient) const;
	
	mia::C3DSplineTransformPenalty *do_clone() const;

	double m_div_weight; 
	double m_curl_weight; 

	std::unique_ptr<mia::C3DPPDivcurlMatrix> m_ppmatrix;

}; 


class C3DDivcurlSplinePenaltyPlugin : public mia::C3DSplineTransformPenaltyPlugin {
public: 
	C3DDivcurlSplinePenaltyPlugin(); 
	
private: 
	virtual const std::string do_get_descr() const;
	virtual Product *do_create(float weight) const __attribute__((warn_unused_result));
	float m_div_weight; 
	float m_curl_weight; 

}; 

NS_END