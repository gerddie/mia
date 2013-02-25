/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/3d/splinepenalty/divcurl.hh>

NS_BEGIN(divcurl_splinepenalty)

NS_MIA_USE

C3DDivcurlSplinePenalty::C3DDivcurlSplinePenalty(double weight, double div_weight, double curl_weight):
	C3DSplineTransformPenalty(weight), 
	m_div_weight(div_weight), 
	m_curl_weight(curl_weight)
{
}

void C3DDivcurlSplinePenalty::do_initialize()
{
	if (m_ppmatrix)
		m_ppmatrix->reset(get_size(), get_range(), *get_kernel(), 
				  m_div_weight, m_curl_weight); 
	else 
		m_ppmatrix.reset(new C3DPPDivcurlMatrix(get_size(), get_range(), *get_kernel(), 
							m_div_weight, m_curl_weight)); 
}

double C3DDivcurlSplinePenalty::do_value(const C3DFVectorfield&  coefficients) const
{
	assert(m_ppmatrix); 
	return *m_ppmatrix * coefficients; 
}


double C3DDivcurlSplinePenalty::do_value_and_gradient(const C3DFVectorfield&  coefficients, CDoubleVector& gradient) const
{
	return m_ppmatrix->evaluate(coefficients, gradient); 
}

C3DSplineTransformPenalty *C3DDivcurlSplinePenalty::do_clone() const
{
	C3DSplineTransformPenalty *result =  
		new C3DDivcurlSplinePenalty(get_weight(), m_div_weight, m_curl_weight);
	if (get_kernel()) 
		result->initialize(get_size(), get_range(), get_kernel()); 
	return result; 
}
  
C3DDivcurlSplinePenaltyPlugin::C3DDivcurlSplinePenaltyPlugin():
	C3DSplineTransformPenaltyPlugin("divcurl"), 
	m_div_weight(1.0), 
	m_curl_weight(1.0)
{
	this->add_parameter("div", new CFloatParameter(m_div_weight, 0.0f, std::numeric_limits<float>::max(), 
						       false, "penalty weight on divergence"));
	this->add_parameter("curl", new CFloatParameter(m_curl_weight, 0.0f, std::numeric_limits<float>::max(), 
							false, "penalty weight on curl"));
	
}

const std::string C3DDivcurlSplinePenaltyPlugin::do_get_descr() const
{
	return "divcurl penalty on the transformation"; 
}

C3DDivcurlSplinePenaltyPlugin::Product *C3DDivcurlSplinePenaltyPlugin::do_create(float weight) const
{
	return new C3DDivcurlSplinePenalty(weight, m_div_weight, m_curl_weight); 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDivcurlSplinePenaltyPlugin();
}

NS_END
