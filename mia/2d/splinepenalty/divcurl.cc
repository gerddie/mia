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

#include <mia/2d/splinepenalty/divcurl.hh>

NS_BEGIN(divcurl_splinepenalty)

NS_MIA_USE

C2DDivcurlSplinePenalty::C2DDivcurlSplinePenalty(double weight, bool normalize, double div_weight, double curl_weight):
	C2DSplineTransformPenalty(weight, normalize), 
	m_div_weight(div_weight), 
	m_curl_weight(curl_weight)
{
}

void C2DDivcurlSplinePenalty::do_initialize()
{
	if (m_ppmatrix)
		m_ppmatrix->reset(get_size(), get_range(), *get_kernel(), 
				  m_div_weight, m_curl_weight); 
	else 
		m_ppmatrix.reset(new C2DPPDivcurlMatrix(get_size(), get_range(), *get_kernel(), 
							m_div_weight, m_curl_weight)); 
}

double C2DDivcurlSplinePenalty::do_value(const C2DFVectorfield&  coefficients) const
{
	assert(m_ppmatrix); 
	return *m_ppmatrix * coefficients; 
}


double C2DDivcurlSplinePenalty::do_value_and_gradient(const C2DFVectorfield&  coefficients, CDoubleVector& gradient) const
{
	return m_ppmatrix->evaluate(coefficients, gradient); 
}

C2DSplineTransformPenalty *C2DDivcurlSplinePenalty::do_clone() const
{
	C2DSplineTransformPenalty *result =  
		new C2DDivcurlSplinePenalty(get_weight(), get_normalize(), m_div_weight, m_curl_weight);
	if (get_kernel()) 
		result->initialize(get_size(), get_range(), get_kernel()); 
	return result; 
}
  
C2DDivcurlSplinePenaltyPlugin::C2DDivcurlSplinePenaltyPlugin():
	C2DSplineTransformPenaltyPlugin("divcurl"), 
	m_div_weight(1.0), 
	m_curl_weight(1.0)
{
	this->add_parameter("div", make_nonnegative_param(m_div_weight, false, "penalty weight on divergence"));
	this->add_parameter("curl", make_nonnegative_param(m_curl_weight, false, "penalty weight on curl"));
	
}

const std::string C2DDivcurlSplinePenaltyPlugin::do_get_descr() const
{
	return "divcurl penalty on the transformation"; 
}

C2DDivcurlSplinePenaltyPlugin::Product *C2DDivcurlSplinePenaltyPlugin::do_create(float weight, bool normalize) const
{
	return new C2DDivcurlSplinePenalty(weight, normalize, m_div_weight, m_curl_weight); 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDivcurlSplinePenaltyPlugin();
}

NS_END
