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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>

#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>


NS_BEGIN(NS)

template <typename Mask>
struct FEvalMI : public mia::TFilter<double> {
	FEvalMI( mia::CSplineParzenMI& parzen_mi, const Mask& m):
		m_parzen_mi(parzen_mi), 
		m_mask(m)
		{}
	

	template <typename  T, typename  R>
	FEvalMI::result_type operator () (const T& a, const R& b) const {
		m_parzen_mi.fill(a.begin(), a.end(), b.begin(), b.end(), m_mask.begin(), m_mask.end()); 
		return  m_parzen_mi.value(); 
	}
	mia::CSplineParzenMI& m_parzen_mi; 
	const Mask& m_mask; 
}; 


template <typename T> 
TMIMaskedImageCost<T>::TMIMaskedImageCost(size_t rbins, mia::PSplineKernel rkernel, size_t mbins, 
			      mia::PSplineKernel mkernel, double cut):
	m_parzen_mi(rbins, rkernel, mbins,  mkernel, cut)
	
{
	this->add(::mia::property_gradient);
}

template <typename T> 
double TMIMaskedImageCost<T>::do_value(const Data& a, const Data& b, const Mask& m) const
{
	FEvalMI<Mask> essd(m_parzen_mi, m); 
	return filter(essd, a, b); 
}

template <typename Force, typename Mask>
struct FEvalForce: public mia::TFilter<float> {
	FEvalForce(Force& force, mia::CSplineParzenMI& parzen_mi, const Mask& m):
		m_force(force), 
		m_parzen_mi(parzen_mi), 
		m_mask(m)
		{
		}
	template <typename T, typename R> 
	float operator ()( const T& a, const R& b) const {
		Force gradient = get_gradient(a); 
		m_parzen_mi.fill(a.begin(), a.end(), 
				 b.begin(), b.end(), 
				 m_mask.begin(), m_mask.end()); 
		auto ai = a.begin();
		auto bi = b.begin();
		auto mi = m_mask.begin();
	
		for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi, ++mi) {
			if (*mi) {
				float delta = -m_parzen_mi.get_gradient_slow(*ai, *bi); 
				m_force[i] = gradient[i] * delta;
			}
		}
		return m_parzen_mi.value(); 
	}
private: 
	Force& m_force; 
	mia::CSplineParzenMI& m_parzen_mi;
	const Mask& m_mask;
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename T> 
double TMIMaskedImageCost<T>::do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == m.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce<Force, Mask> ef(force, m_parzen_mi, m); 
	return filter(ef, a, b); 
}

template <typename T> 
void TMIMaskedImageCost<T>::post_set_reference(const Data& MIA_PARAM_UNUSED(ref))
{
	m_parzen_mi.reset(); 
}

template <typename CP, typename C> 
TMIMaskedImageCostPlugin<CP,C>::TMIMaskedImageCostPlugin():
	CP("mi"), 
	m_rbins(64), 
	m_mbins(64), 
	m_histogram_cut(0.0)
{
	TRACE("TMIMaskedImageCostPlugin<CP,C>::TMIMaskedImageCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
	this->add_parameter("rbins", mia::make_ci_param(m_rbins, 1, 256, false, 
							"Number of histogram bins used for the reference image")); 
	
	this->add_parameter("mbins", mia::make_ci_param(m_mbins, 1, 256, false, 
							"Number of histogram bins used for the moving image")); 
	
	this->add_parameter("rkernel", mia::make_param(m_rkernel, "bspline:d=0", false, 
						       "Spline kernel for reference image parzen hinstogram")); 
	
	this->add_parameter("mkernel", mia::make_param(m_mkernel, "bspline:d=3", false, 
						       "Spline kernel for moving image parzen hinstogram"));  

	this->add_parameter("cut", mia::make_ci_param(m_histogram_cut, 0.0f, 40.0f, false, 
						      "Percentage of pixels to cut at high and low "
						      "intensities to remove outliers")); 
}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
C *TMIMaskedImageCostPlugin<CP,C>::do_create() const
{
	return new TMIMaskedImageCost<C>(m_rbins, m_rkernel, m_mbins,  m_mkernel, m_histogram_cut); 
}

template <typename CP, typename C> 
const std::string TMIMaskedImageCostPlugin<CP,C>::do_get_descr() const
{
	return "Spline parzen based mutual information with masking.";  
	
}

NS_END
